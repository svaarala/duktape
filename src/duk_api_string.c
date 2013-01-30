/*
 *  String manipulation
 */

#include "duk_internal.h"

void duk_concat(duk_context *ctx, unsigned int count) {
	unsigned int i;
	unsigned int len;
	unsigned int idx;
	duk_hstring *h;
	duk_u8 *buf;

	DUK_ASSERT(ctx != NULL);

	len = 0;
	for (i = count; i >= 1; i--) {
		duk_to_string(ctx, -i);
		h = duk_require_hstring(ctx, -i);
		len += DUK_HSTRING_GET_BYTELEN(h);
	}

	DUK_DDDPRINT("concat %d strings, total length %d bytes", count, len);

	/* use stack allocated buffer to ensure reachability in errors (e.g. intern error) */
	buf = duk_push_new_buffer(ctx, len, 0);
	DUK_ASSERT(buf != NULL);

	/* [... str1 str2 ... strN buf] */

	idx = 0;
	for (i = count; i >= 1; i--) {
		h = duk_require_hstring(ctx, -i-1);  /* extra -1 for buffer */
		memcpy(buf + idx, DUK_HSTRING_GET_DATA(h), DUK_HSTRING_GET_BYTELEN(h));
		idx += DUK_HSTRING_GET_BYTELEN(h);
	}

	DUK_ASSERT(idx == len);

	/* [... str1 str2 ... strN buf] */

	/* get rid of the strings early to minimize memory use before intern */

	duk_replace(ctx, -count-1);  /* overwrite str1 */
	duk_pop_n(ctx, count-1);

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

void duk_join(duk_context *ctx, unsigned int count) {
	DUK_ERROR((duk_hthread *) ctx, DUK_ERR_UNIMPLEMENTED_ERROR, "FIXME");
}

void duk_decode_string(duk_context *ctx, int index, duk_decode_char_function callback, void *udata) {
	DUK_ERROR((duk_hthread *) ctx, DUK_ERR_UNIMPLEMENTED_ERROR, "FIXME");
}

void duk_map_string(duk_context *ctx, int index, duk_map_char_function callback, void *udata) {
	DUK_ERROR((duk_hthread *) ctx, DUK_ERR_UNIMPLEMENTED_ERROR, "FIXME");
}

void duk_substring(duk_context *ctx, unsigned int start_offset, unsigned int end_offset) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h;
	duk_hstring *res;

	DUK_ASSERT(ctx != NULL);

	h = duk_require_hstring(ctx, -1);
	DUK_ASSERT(h != NULL);

	if (end_offset >= DUK_HSTRING_GET_CHARLEN(h)) {
		end_offset = DUK_HSTRING_GET_CHARLEN(h);
	}
	if (start_offset > end_offset) {
		start_offset = end_offset;
	}

	DUK_ASSERT(start_offset >= 0 && start_offset <= end_offset && start_offset <= DUK_HSTRING_GET_CHARLEN(h));
	DUK_ASSERT(end_offset >= 0 && end_offset >= start_offset && end_offset <= DUK_HSTRING_GET_CHARLEN(h));

	if (DUK_HSTRING_IS_ASCII(h)) {
		res = duk_heap_string_intern_checked(thr, DUK_HSTRING_GET_DATA(h) + start_offset, end_offset - start_offset);
	} else {
		res = NULL;  /* FIXME */
		DUK_ERROR(thr, DUK_ERR_UNIMPLEMENTED_ERROR, "char substring not implemented for unicode strings");
	}

	duk_push_hstring(ctx, res);
	duk_remove(ctx, -2);
}


