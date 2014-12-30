/*
 *  Buffer built-ins
 */

#include "duk_internal.h"

/*
 *  Constructor
 */

DUK_INTERNAL duk_ret_t duk_bi_buffer_constructor(duk_context *ctx) {
	duk_size_t buf_size;
	duk_small_int_t buf_dynamic;
	duk_uint8_t *buf_data;
	const duk_uint8_t *src_data;
	duk_hobject *h_obj;

	/*
	 *  Constructor arguments are currently somewhat compatible with
	 *  (keep it that way if possible):
	 *
	 *    http://nodejs.org/api/buffer.html
	 *
	 *  Note that the ToBuffer() coercion (duk_to_buffer()) does NOT match
	 *  the constructor behavior.
	 */

	buf_dynamic = duk_get_boolean(ctx, 1);  /* default to false */

	switch (duk_get_type(ctx, 0)) {
	case DUK_TYPE_NUMBER:
		/* new buffer of specified size */
		buf_size = (duk_size_t) duk_to_int(ctx, 0);
		(void) duk_push_buffer(ctx, buf_size, buf_dynamic);
		break;
	case DUK_TYPE_BUFFER:
		/* return input buffer, converted to a Buffer object if called as a
		 * constructor (no change if called as a function).
		 */
		duk_set_top(ctx, 1);
		break;
	case DUK_TYPE_STRING:
		/* new buffer with string contents */
		src_data = (const duk_uint8_t *) duk_get_lstring(ctx, 0, &buf_size);
		DUK_ASSERT(src_data != NULL);  /* even for zero-length string */
		buf_data = (duk_uint8_t *) duk_push_buffer(ctx, buf_size, buf_dynamic);
		DUK_MEMCPY((void *) buf_data, (const void *) src_data, (size_t) buf_size);
		break;
	case DUK_TYPE_OBJECT:
		/* Buffer object: get the plain buffer inside.  If called as as
		 * constructor, a new Buffer object pointing to the same plain
		 * buffer is created below.
		 */
		h_obj = duk_get_hobject(ctx, 0);
		DUK_ASSERT(h_obj != NULL);
		if (DUK_HOBJECT_GET_CLASS_NUMBER(h_obj) != DUK_HOBJECT_CLASS_BUFFER) {
			return DUK_RET_TYPE_ERROR;
		}
		duk_get_prop_stridx(ctx, 0, DUK_STRIDX_INT_VALUE);
		DUK_ASSERT(duk_is_buffer(ctx, -1));
		break;
	case DUK_TYPE_NONE:
	default:
		return DUK_RET_TYPE_ERROR;
	}

	/* stack is unbalanced, but: [ <something> buf ] */

	if (duk_is_constructor_call(ctx)) {
		duk_push_object_helper(ctx,
		                       DUK_HOBJECT_FLAG_EXTENSIBLE |
		                       DUK_HOBJECT_FLAG_EXOTIC_BUFFEROBJ |
		                       DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_BUFFER),
		                       DUK_BIDX_BUFFER_PROTOTYPE);

		/* Buffer object internal value is immutable */
		duk_dup(ctx, -2);
		duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_NONE);
	}
	/* Note: unbalanced stack on purpose */

	return 1;
}

/*
 *  toString(), valueOf()
 */

DUK_INTERNAL duk_ret_t duk_bi_buffer_prototype_tostring_shared(duk_context *ctx) {
	duk_tval *tv;
	duk_small_int_t to_string = duk_get_current_magic(ctx);

	duk_push_this(ctx);
	tv = duk_require_tval(ctx, -1);
	DUK_ASSERT(tv != NULL);

	if (DUK_TVAL_IS_BUFFER(tv)) {
		/* nop */
	} else if (DUK_TVAL_IS_OBJECT(tv)) {
		duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);

		/* Must be a "buffer object", i.e. class "Buffer" */
		if (DUK_HOBJECT_GET_CLASS_NUMBER(h) != DUK_HOBJECT_CLASS_BUFFER) {
			goto type_error;
		}

		duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_VALUE);
	} else {
		goto type_error;
	}

	if (to_string) {
		duk_to_string(ctx, -1);
	}
	return 1;

 type_error:
	return DUK_RET_TYPE_ERROR;
}
