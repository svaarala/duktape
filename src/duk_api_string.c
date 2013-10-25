/*
 *  String manipulation
 */

#include "duk_internal.h"

static void concat_and_join_helper(duk_context *ctx, int count, int is_join) {
	unsigned int i;
	unsigned int len;
	unsigned int idx;
	duk_hstring *h;
	duk_u8 *buf;

	DUK_ASSERT(ctx != NULL);

	if (count <= 0) {
		duk_push_hstring_stridx(ctx, DUK_STRIDX_EMPTY_STRING);
		return;
	}

	if (is_join) {
		h = duk_to_hstring(ctx, -count-1);
		DUK_ASSERT(h != NULL);
		len = DUK_HSTRING_GET_BYTELEN(h) * (count - 1);
	} else {
		len = 0;
	}

	for (i = count; i >= 1; i--) {
		duk_to_string(ctx, -i);
		h = duk_require_hstring(ctx, -i);
		len += DUK_HSTRING_GET_BYTELEN(h);
	}

	DUK_DDDPRINT("join/concat %d strings, total length %d bytes", count, len);

	/* use stack allocated buffer to ensure reachability in errors (e.g. intern error) */
	buf = (duk_u8 *) duk_push_fixed_buffer(ctx, len);
	DUK_ASSERT(buf != NULL);

	/* [... (sep) str1 str2 ... strN buf] */

	idx = 0;
	for (i = count; i >= 1; i--) {
		if (is_join && i != count) {
			h = duk_require_hstring(ctx, -count-2);  /* extra -1 for buffer */
			DUK_MEMCPY(buf + idx, DUK_HSTRING_GET_DATA(h), DUK_HSTRING_GET_BYTELEN(h));
			idx += DUK_HSTRING_GET_BYTELEN(h);
		}
		h = duk_require_hstring(ctx, -i-1);  /* extra -1 for buffer */
		DUK_MEMCPY(buf + idx, DUK_HSTRING_GET_DATA(h), DUK_HSTRING_GET_BYTELEN(h));
		idx += DUK_HSTRING_GET_BYTELEN(h);
	}

	DUK_ASSERT(idx == len);

	/* [... (sep) str1 str2 ... strN buf] */

	/* get rid of the strings early to minimize memory use before intern */

	if (is_join) {
		duk_replace(ctx, -count-2);  /* overwrite sep */
		duk_pop_n(ctx, count);
	} else {
		duk_replace(ctx, -count-1);  /* overwrite str1 */
		duk_pop_n(ctx, count-1);
	}

	/* [... buf] */

	/*
	 *  FIXME: just allow C code to call duk_to_string() on buffers.
	 *  This allows C code to manufacture internal keys, but since we
	 *  trust C code anyway, this is not a big issue.
	 */

	duk_push_lstring(ctx, (const char *) buf, len);

	/* [... buf res] */

	duk_remove(ctx, -2);

	/* [... res] */
}

void duk_concat(duk_context *ctx, unsigned int count) {
	concat_and_join_helper(ctx, count, 0 /*is_join*/);
}

void duk_join(duk_context *ctx, unsigned int count) {
	concat_and_join_helper(ctx, count, 1 /*is_join*/);
}

void duk_decode_string(duk_context *ctx, int index, duk_decode_char_function callback, void *udata) {
	DUK_ERROR((duk_hthread *) ctx, DUK_ERR_UNIMPLEMENTED_ERROR, "FIXME");
}

void duk_map_string(duk_context *ctx, int index, duk_map_char_function callback, void *udata) {
	DUK_ERROR((duk_hthread *) ctx, DUK_ERR_UNIMPLEMENTED_ERROR, "FIXME");
}

void duk_substring(duk_context *ctx, int index, size_t start_offset, size_t end_offset) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h;
	duk_hstring *res;
	size_t start_byte_offset;
	size_t end_byte_offset;

	DUK_ASSERT(ctx != NULL);

	index = duk_require_normalize_index(ctx, index);
	h = duk_require_hstring(ctx, index);
	DUK_ASSERT(h != NULL);

	if (end_offset >= DUK_HSTRING_GET_CHARLEN(h)) {
		end_offset = DUK_HSTRING_GET_CHARLEN(h);
	}
	if (start_offset > end_offset) {
		start_offset = end_offset;
	}

	DUK_ASSERT(start_offset >= 0 && start_offset <= end_offset && start_offset <= DUK_HSTRING_GET_CHARLEN(h));
	DUK_ASSERT(end_offset >= 0 && end_offset >= start_offset && end_offset <= DUK_HSTRING_GET_CHARLEN(h));

	start_byte_offset = (size_t) duk_heap_strcache_offset_char2byte(thr, h, start_offset);
	end_byte_offset = (size_t) duk_heap_strcache_offset_char2byte(thr, h, end_offset);

	DUK_ASSERT(end_byte_offset >= start_byte_offset);

	res = duk_heap_string_intern_checked(thr,
	                                     DUK_HSTRING_GET_DATA(h) + start_byte_offset,
	                                     end_byte_offset - start_byte_offset);

	duk_push_hstring(ctx, res);
	duk_replace(ctx, index);
}

/* FIXME: this is quite clunky.  Add Unicode helpers to scan backwards and
 * forwards with a callback to process codepoints?
 */
void duk_trim(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h;
	duk_u8 *p, *p_start, *p_end, *p_tmp1, *p_tmp2;  /* pointers for scanning */
	duk_u8 *q_start, *q_end;  /* start (incl) and end (excl) of trimmed part */
	duk_u32 cp;

	index = duk_require_normalize_index(ctx, index);
	h = duk_require_hstring(ctx, index);
	DUK_ASSERT(h != NULL);

	p_start = DUK_HSTRING_GET_DATA(h);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h);

	p = p_start;
	while (p < p_end) {
		p_tmp1 = p;
		cp = duk_unicode_xutf8_get_u32_checked(thr, &p_tmp1, p_start, p_end);
		if (!(duk_unicode_is_whitespace(cp) || duk_unicode_is_line_terminator(cp))) {
			break;
		}
		p = p_tmp1;
	}
	q_start = p;
	if (p == p_end) {
		/* entire string is whitespace */
		q_end = p;
		goto scan_done;
	}

	p = p_end;
	while (p > p_start) {
		p_tmp1 = p;
		while (p > p_start) {
			p--;
			if (((*p) & 0xc0) != 0x80) {
				break;
			}
		}
		p_tmp2 = p;

		cp = duk_unicode_xutf8_get_u32_checked(thr, &p_tmp2, p_start, p_end);
		if (!(duk_unicode_is_whitespace(cp) || duk_unicode_is_line_terminator(cp))) {
			p = p_tmp1;
			break;
		}
	}
	q_end = p;

 scan_done:
	/* This may happen when forward and backward scanning disagree.
	 * This may happen for non-extended-UTF-8 strings.
	 */
	if (q_end < q_start) {
		q_end = q_start;
	}

	DUK_ASSERT(q_start >= p_start && q_start <= p_end);
	DUK_ASSERT(q_end >= p_start && q_end <= p_end);
	DUK_ASSERT(q_end >= q_start);

	DUK_DDDPRINT("trim: p_start=%p, p_end=%p, q_start=%p, q_end=%p",
	             (void *) p_start, (void *) p_end, (void *) q_start, (void *) q_end);

	if (q_start == p_start && q_end == p_end) {
		DUK_DDDPRINT("nothing was trimmed: avoid interning (hashing etc)");
		return;
	}

	duk_push_lstring(ctx, (const char *) q_start, (size_t) (q_end - q_start));
	duk_replace(ctx, index);
}

