/*
 *  String manipulation
 */

#include "duk_internal.h"

DUK_LOCAL void duk__concat_and_join_helper(duk_context *ctx, duk_idx_t count_in, duk_bool_t is_join) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_uint_t count;
	duk_uint_t i;
	duk_size_t idx;
	duk_size_t len;
	duk_hstring *h;
	duk_uint8_t *buf;

	DUK_ASSERT_CTX_VALID(ctx);

	if (DUK_UNLIKELY(count_in <= 0)) {
		if (count_in < 0) {
			DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_COUNT);
			return;
		}
		DUK_ASSERT(count_in == 0);
		duk_push_hstring_stridx(ctx, DUK_STRIDX_EMPTY_STRING);
		return;
	}
	count = (duk_uint_t) count_in;

	if (is_join) {
		duk_size_t t1, t2, limit;
		h = duk_to_hstring(ctx, -((duk_idx_t) count) - 1);
		DUK_ASSERT(h != NULL);

		/* A bit tricky overflow test, see doc/code-issues.rst. */
		t1 = (duk_size_t) DUK_HSTRING_GET_BYTELEN(h);
		t2 = (duk_size_t) (count - 1);
		limit = (duk_size_t) DUK_HSTRING_MAX_BYTELEN;
		if (DUK_UNLIKELY(t2 != 0 && t1 > limit / t2)) {
			/* Combined size of separators already overflows */
			goto error_overflow;
		}
		len = (duk_size_t) (t1 * t2);
	} else {
		len = (duk_size_t) 0;
	}

	for (i = count; i >= 1; i--) {
		duk_size_t new_len;
		duk_to_string(ctx, -((duk_idx_t) i));
		h = duk_require_hstring(ctx, -((duk_idx_t) i));
		new_len = len + (duk_size_t) DUK_HSTRING_GET_BYTELEN(h);

		/* Impose a string maximum length, need to handle overflow
		 * correctly.
		 */
		if (new_len < len ||  /* wrapped */
		    new_len > (duk_size_t) DUK_HSTRING_MAX_BYTELEN) {
			goto error_overflow;
		}
		len = new_len;
	}

	DUK_DDD(DUK_DDDPRINT("join/concat %lu strings, total length %lu bytes",
	                     (unsigned long) count, (unsigned long) len));

	/* use stack allocated buffer to ensure reachability in errors (e.g. intern error) */
	buf = (duk_uint8_t *) duk_push_fixed_buffer(ctx, len);
	DUK_ASSERT(buf != NULL);

	/* [... (sep) str1 str2 ... strN buf] */

	idx = 0;
	for (i = count; i >= 1; i--) {
		if (is_join && i != count) {
			h = duk_require_hstring(ctx, -((duk_idx_t) count) - 2);  /* extra -1 for buffer */
			DUK_MEMCPY(buf + idx, DUK_HSTRING_GET_DATA(h), DUK_HSTRING_GET_BYTELEN(h));
			idx += DUK_HSTRING_GET_BYTELEN(h);
		}
		h = duk_require_hstring(ctx, -((duk_idx_t) i) - 1);  /* extra -1 for buffer */
		DUK_MEMCPY(buf + idx, DUK_HSTRING_GET_DATA(h), DUK_HSTRING_GET_BYTELEN(h));
		idx += DUK_HSTRING_GET_BYTELEN(h);
	}

	DUK_ASSERT(idx == len);

	/* [... (sep) str1 str2 ... strN buf] */

	/* get rid of the strings early to minimize memory use before intern */

	if (is_join) {
		duk_replace(ctx, -((duk_idx_t) count) - 2);  /* overwrite sep */
		duk_pop_n(ctx, count);
	} else {
		duk_replace(ctx, -((duk_idx_t) count) - 1);  /* overwrite str1 */
		duk_pop_n(ctx, count-1);
	}

	/* [... buf] */

	(void) duk_to_string(ctx, -1);

	/* [... res] */
	return;

 error_overflow:
	DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, DUK_STR_CONCAT_RESULT_TOO_LONG);
}

DUK_EXTERNAL void duk_concat(duk_context *ctx, duk_idx_t count) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk__concat_and_join_helper(ctx, count, 0 /*is_join*/);
}

DUK_EXTERNAL void duk_join(duk_context *ctx, duk_idx_t count) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk__concat_and_join_helper(ctx, count, 1 /*is_join*/);
}

/* XXX: could map/decode be unified with duk_unicode_support.c code?
 * Case conversion needs also the character surroundings though.
 */

DUK_EXTERNAL void duk_decode_string(duk_context *ctx, duk_idx_t index, duk_decode_char_function callback, void *udata) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_input;
	const duk_uint8_t *p, *p_start, *p_end;
	duk_codepoint_t cp;

	DUK_ASSERT_CTX_VALID(ctx);

	h_input = duk_require_hstring(ctx, index);
	DUK_ASSERT(h_input != NULL);

	p_start = (duk_uint8_t *) DUK_HSTRING_GET_DATA(h_input);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_input);
	p = p_start;

	for (;;) {
		if (p >= p_end) {
			break;
		}
		cp = (int) duk_unicode_decode_xutf8_checked(thr, &p, p_start, p_end);
		callback(udata, cp);
	}
}

DUK_EXTERNAL void duk_map_string(duk_context *ctx, duk_idx_t index, duk_map_char_function callback, void *udata) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_input;
	duk_bufwriter_ctx bw_alloc;
	duk_bufwriter_ctx *bw;
	const duk_uint8_t *p, *p_start, *p_end;
	duk_codepoint_t cp;

	DUK_ASSERT_CTX_VALID(ctx);

	index = duk_normalize_index(ctx, index);

	h_input = duk_require_hstring(ctx, index);
	DUK_ASSERT(h_input != NULL);

	bw = &bw_alloc;
	DUK_BW_INIT_PUSHBUF(thr, bw, DUK_HSTRING_GET_BYTELEN(h_input));  /* reasonable output estimate */

	p_start = (duk_uint8_t *) DUK_HSTRING_GET_DATA(h_input);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_input);
	p = p_start;

	for (;;) {
		/* XXX: could write output in chunks with fewer ensure calls,
		 * but relative benefit would be small here.
		 */

		if (p >= p_end) {
			break;
		}
		cp = (int) duk_unicode_decode_xutf8_checked(thr, &p, p_start, p_end);
		cp = callback(udata, cp);

		DUK_BW_WRITE_ENSURE_XUTF8(thr, bw, cp);
	}

	DUK_BW_COMPACT(thr, bw);
	duk_to_string(ctx, -1);
	duk_replace(ctx, index);
}

DUK_EXTERNAL void duk_substring(duk_context *ctx, duk_idx_t index, duk_size_t start_offset, duk_size_t end_offset) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h;
	duk_hstring *res;
	duk_size_t start_byte_offset;
	duk_size_t end_byte_offset;

	DUK_ASSERT_CTX_VALID(ctx);

	index = duk_require_normalize_index(ctx, index);
	h = duk_require_hstring(ctx, index);
	DUK_ASSERT(h != NULL);

	if (end_offset >= DUK_HSTRING_GET_CHARLEN(h)) {
		end_offset = DUK_HSTRING_GET_CHARLEN(h);
	}
	if (start_offset > end_offset) {
		start_offset = end_offset;
	}

	DUK_ASSERT_DISABLE(start_offset >= 0);
	DUK_ASSERT(start_offset <= end_offset && start_offset <= DUK_HSTRING_GET_CHARLEN(h));
	DUK_ASSERT_DISABLE(end_offset >= 0);
	DUK_ASSERT(end_offset >= start_offset && end_offset <= DUK_HSTRING_GET_CHARLEN(h));

	/* guaranteed by string limits */
	DUK_ASSERT(start_offset <= DUK_UINT32_MAX);
	DUK_ASSERT(end_offset <= DUK_UINT32_MAX);

	start_byte_offset = (duk_size_t) duk_heap_strcache_offset_char2byte(thr, h, (duk_uint_fast32_t) start_offset);
	end_byte_offset = (duk_size_t) duk_heap_strcache_offset_char2byte(thr, h, (duk_uint_fast32_t) end_offset);

	DUK_ASSERT(end_byte_offset >= start_byte_offset);
	DUK_ASSERT(end_byte_offset - start_byte_offset <= DUK_UINT32_MAX);  /* guaranteed by string limits */

	/* no size check is necessary */
	res = duk_heap_string_intern_checked(thr,
	                                     DUK_HSTRING_GET_DATA(h) + start_byte_offset,
	                                     (duk_uint32_t) (end_byte_offset - start_byte_offset));

	duk_push_hstring(ctx, res);
	duk_replace(ctx, index);
}

/* XXX: this is quite clunky.  Add Unicode helpers to scan backwards and
 * forwards with a callback to process codepoints?
 */
DUK_EXTERNAL void duk_trim(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h;
	const duk_uint8_t *p, *p_start, *p_end, *p_tmp1, *p_tmp2;  /* pointers for scanning */
	const duk_uint8_t *q_start, *q_end;  /* start (incl) and end (excl) of trimmed part */
	duk_codepoint_t cp;

	DUK_ASSERT_CTX_VALID(ctx);

	index = duk_require_normalize_index(ctx, index);
	h = duk_require_hstring(ctx, index);
	DUK_ASSERT(h != NULL);

	p_start = DUK_HSTRING_GET_DATA(h);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h);

	p = p_start;
	while (p < p_end) {
		p_tmp1 = p;
		cp = (duk_codepoint_t) duk_unicode_decode_xutf8_checked(thr, &p_tmp1, p_start, p_end);
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

		cp = (duk_codepoint_t) duk_unicode_decode_xutf8_checked(thr, &p_tmp2, p_start, p_end);
		if (!(duk_unicode_is_whitespace(cp) || duk_unicode_is_line_terminator(cp))) {
			p = p_tmp1;
			break;
		}
	}
	q_end = p;

 scan_done:
	/* This may happen when forward and backward scanning disagree
	 * (possible for non-extended-UTF-8 strings).
	 */
	if (q_end < q_start) {
		q_end = q_start;
	}

	DUK_ASSERT(q_start >= p_start && q_start <= p_end);
	DUK_ASSERT(q_end >= p_start && q_end <= p_end);
	DUK_ASSERT(q_end >= q_start);

	DUK_DDD(DUK_DDDPRINT("trim: p_start=%p, p_end=%p, q_start=%p, q_end=%p",
	                     (void *) p_start, (void *) p_end, (void *) q_start, (void *) q_end));

	if (q_start == p_start && q_end == p_end) {
		DUK_DDD(DUK_DDDPRINT("nothing was trimmed: avoid interning (hashing etc)"));
		return;
	}

	duk_push_lstring(ctx, (const char *) q_start, (duk_size_t) (q_end - q_start));
	duk_replace(ctx, index);
}

DUK_EXTERNAL duk_codepoint_t duk_char_code_at(duk_context *ctx, duk_idx_t index, duk_size_t char_offset) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h;
	duk_ucodepoint_t cp;

	DUK_ASSERT_CTX_VALID(ctx);

	h = duk_require_hstring(ctx, index);
	DUK_ASSERT(h != NULL);

	DUK_ASSERT_DISABLE(char_offset >= 0);  /* always true, arg is unsigned */
	if (char_offset >= DUK_HSTRING_GET_CHARLEN(h)) {
		return 0;
	}

	DUK_ASSERT(char_offset <= DUK_UINT_MAX);  /* guaranteed by string limits */
	cp = duk_hstring_char_code_at_raw(thr, h, (duk_uint_t) char_offset);
	return (duk_codepoint_t) cp;
}
