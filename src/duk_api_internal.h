/*
 *  Internal API calls which have (stack and other) semantics similar
 *  to the public API.
 */

#ifndef DUK_API_INTERNAL_H_INCLUDED
#define DUK_API_INTERNAL_H_INCLUDED

/* duk_push_sprintf constants */
#define DUK_PUSH_SPRINTF_INITIAL_SIZE  256L
#define DUK_PUSH_SPRINTF_SANITY_LIMIT  (1L * 1024L * 1024L * 1024L)

/* Flag ORed to err_code to indicate __FILE__ / __LINE__ is not
 * blamed as source of error for error fileName / lineNumber.
 */
#define DUK_ERRCODE_FLAG_NOBLAME_FILELINE  (1L << 24)

/* Current convention is to use duk_size_t for value stack sizes and global indices,
 * and duk_idx_t for local frame indices.
 */
duk_bool_t duk_check_valstack_resize(duk_context *ctx, duk_size_t min_new_size, duk_bool_t allow_shrink);
void duk_require_valstack_resize(duk_context *ctx, duk_size_t min_new_size, duk_bool_t allow_shrink);

duk_tval *duk_get_tval(duk_context *ctx, duk_idx_t index);
duk_tval *duk_require_tval(duk_context *ctx, duk_idx_t index);
void duk_push_tval(duk_context *ctx, duk_tval *tv);

void duk_push_this_check_object_coercible(duk_context *ctx);   /* push the current 'this' binding; throw TypeError
                                                                * if binding is not object coercible (CheckObjectCoercible).
                                                                */
duk_hobject *duk_push_this_coercible_to_object(duk_context *ctx);       /* duk_push_this() + CheckObjectCoercible() + duk_to_object() */
duk_hstring *duk_push_this_coercible_to_string(duk_context *ctx);       /* duk_push_this() + CheckObjectCoercible() + duk_to_string() */

/* duk_push_uint() is guaranteed to support at least unsigned 32-bit range */
#define duk_push_u32(ctx,val) \
	duk_push_uint((ctx), (duk_uint_t) (val))

/* sometimes stack and array indices need to go on the stack */
#define duk_push_idx(ctx,val) \
	duk_push_int((ctx), (duk_int_t) (val))
#define duk_push_uarridx(ctx,val) \
	duk_push_uint((ctx), (duk_uint_t) (val))
#define duk_push_size_t(ctx,val) \
	duk_push_uint((ctx), (duk_uint_t) (val))  /* XXX: assumed to fit for now */

/* internal helper for looking up a tagged type */
#define  DUK_GETTAGGED_FLAG_ALLOW_NULL  (1L << 24)
#define  DUK_GETTAGGED_FLAG_CHECK_CLASS (1L << 25)
#define  DUK_GETTAGGED_CLASS_SHIFT      16

duk_heaphdr *duk_get_tagged_heaphdr_raw(duk_context *ctx, duk_idx_t index, duk_uint_t flags_and_tag);

duk_hstring *duk_get_hstring(duk_context *ctx, duk_idx_t index);
duk_hobject *duk_get_hobject(duk_context *ctx, duk_idx_t index);
duk_hbuffer *duk_get_hbuffer(duk_context *ctx, duk_idx_t index);
duk_hthread *duk_get_hthread(duk_context *ctx, duk_idx_t index);
duk_hcompiledfunction *duk_get_hcompiledfunction(duk_context *ctx, duk_idx_t index);
duk_hnativefunction *duk_get_hnativefunction(duk_context *ctx, duk_idx_t index);

#define duk_get_hobject_with_class(ctx,index,classnum) \
	((duk_hobject *) duk_get_tagged_heaphdr_raw((ctx), (index), \
		DUK_TAG_OBJECT | DUK_GETTAGGED_FLAG_ALLOW_NULL | \
		DUK_GETTAGGED_FLAG_CHECK_CLASS | ((classnum) << DUK_GETTAGGED_CLASS_SHIFT)))

void *duk_get_voidptr(duk_context *ctx, duk_idx_t index);

duk_hstring *duk_to_hstring(duk_context *ctx, duk_idx_t index);
duk_int_t duk_to_int_clamped_raw(duk_context *ctx, duk_idx_t index, duk_int_t minval, duk_int_t maxval, duk_bool_t *out_clamped);  /* out_clamped=NULL, RangeError if outside range */
duk_int_t duk_to_int_clamped(duk_context *ctx, duk_idx_t index, duk_int_t minval, duk_int_t maxval);
duk_int_t duk_to_int_check_range(duk_context *ctx, duk_idx_t index, duk_int_t minval, duk_int_t maxval);

duk_hstring *duk_require_hstring(duk_context *ctx, duk_idx_t index);
duk_hobject *duk_require_hobject(duk_context *ctx, duk_idx_t index);
duk_hbuffer *duk_require_hbuffer(duk_context *ctx, duk_idx_t index);
duk_hthread *duk_require_hthread(duk_context *ctx, duk_idx_t index);
duk_hcompiledfunction *duk_require_hcompiledfunction(duk_context *ctx, duk_idx_t index);
duk_hnativefunction *duk_require_hnativefunction(duk_context *ctx, duk_idx_t index);

#define duk_require_hobject_with_class(ctx,index,classnum) \
	((duk_hobject *) duk_get_tagged_heaphdr_raw((ctx), (index), \
		DUK_TAG_OBJECT | \
		DUK_GETTAGGED_FLAG_CHECK_CLASS | ((classnum) << DUK_GETTAGGED_CLASS_SHIFT)))

void duk_push_unused(duk_context *ctx);
void duk_push_hstring(duk_context *ctx, duk_hstring *h);
void duk_push_hstring_stridx(duk_context *ctx, duk_small_int_t stridx);
void duk_push_hobject(duk_context *ctx, duk_hobject *h);
void duk_push_hbuffer(duk_context *ctx, duk_hbuffer *h);
#define duk_push_hthread(ctx,h) \
	duk_push_hobject((ctx), (duk_hobject *) (h))
#define duk_push_hcompiledfunction(ctx,h) \
	duk_push_hobject((ctx), (duk_hobject *) (h))
#define duk_push_hnativefunction(ctx,h) \
	duk_push_hobject((ctx), (duk_hobject *) (h))
void duk_push_hobject_bidx(duk_context *ctx, duk_small_int_t builtin_idx);
duk_idx_t duk_push_object_helper(duk_context *ctx, duk_uint_t hobject_flags_and_class, duk_small_int_t prototype_bidx);
duk_idx_t duk_push_object_helper_proto(duk_context *ctx, duk_uint_t hobject_flags_and_class, duk_hobject *proto);
duk_idx_t duk_push_object_internal(duk_context *ctx);
duk_idx_t duk_push_compiledfunction(duk_context *ctx);
void duk_push_c_function_noexotic(duk_context *ctx, duk_c_function func, duk_int_t nargs);
void duk_push_c_function_noconstruct_noexotic(duk_context *ctx, duk_c_function func, duk_int_t nargs);

duk_bool_t duk_get_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx);     /* [] -> [val] */
duk_bool_t duk_put_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx);     /* [val] -> [] */
duk_bool_t duk_del_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx);     /* [] -> [] */
duk_bool_t duk_has_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx);     /* [] -> [] */

duk_bool_t duk_get_prop_stridx_boolean(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx, duk_bool_t *out_has_prop);  /* [] -> [] */

void duk_def_prop(duk_context *ctx, duk_idx_t obj_index, duk_small_uint_t desc_flags);  /* [key val] -> [] */
void duk_def_prop_index(duk_context *ctx, duk_idx_t obj_index, duk_uarridx_t arr_index, duk_small_uint_t desc_flags);  /* [val] -> [] */
void duk_def_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx, duk_small_uint_t desc_flags);  /* [val] -> [] */
void duk_def_prop_stridx_builtin(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx, duk_small_int_t builtin_idx, duk_small_uint_t desc_flags);  /* [] -> [] */
void duk_def_prop_stridx_thrower(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx, duk_small_uint_t desc_flags);  /* [] -> [] */

/* These are macros for now, but could be separate functions to reduce code
 * footprint (check call site count before refactoring).
 */
#define duk_def_prop_wec(ctx,obj_index) \
	duk_def_prop((ctx), (obj_index), DUK_PROPDESC_FLAGS_WEC)
#define duk_def_prop_index_wec(ctx,obj_index,arr_index) \
	duk_def_prop_index((ctx), (obj_index), (arr_index), DUK_PROPDESC_FLAGS_WEC)
#define duk_def_prop_stridx_wec(ctx,obj_index,stridx) \
	duk_def_prop_stridx((ctx), (obj_index), (stridx), DUK_PROPDESC_FLAGS_WEC)

/* Set object 'length'. */
void duk_set_length(duk_context *ctx, duk_idx_t index, duk_size_t length);

#endif  /* DUK_API_INTERNAL_H_INCLUDED */
