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

/* Valstack resize flags */
#define DUK_VSRESIZE_FLAG_SHRINK           (1 << 0)
#define DUK_VSRESIZE_FLAG_COMPACT          (1 << 1)
#define DUK_VSRESIZE_FLAG_THROW            (1 << 2)

/* Current convention is to use duk_size_t for value stack sizes and global indices,
 * and duk_idx_t for local frame indices.
 */
DUK_INTERNAL_DECL
duk_bool_t duk_valstack_resize_raw(duk_context *ctx,
                                   duk_size_t min_new_size,
                                   duk_small_uint_t flags);

DUK_INTERNAL_DECL duk_tval *duk_get_tval(duk_context *ctx, duk_idx_t index);
DUK_INTERNAL_DECL duk_tval *duk_require_tval(duk_context *ctx, duk_idx_t index);
DUK_INTERNAL_DECL void duk_push_tval(duk_context *ctx, duk_tval *tv);

/* Push the current 'this' binding; throw TypeError if binding is not object
 * coercible (CheckObjectCoercible).
 */
DUK_INTERNAL_DECL void duk_push_this_check_object_coercible(duk_context *ctx);

/* duk_push_this() + CheckObjectCoercible() + duk_to_object() */
DUK_INTERNAL_DECL duk_hobject *duk_push_this_coercible_to_object(duk_context *ctx);

/* duk_push_this() + CheckObjectCoercible() + duk_to_string() */
DUK_INTERNAL_DECL duk_hstring *duk_push_this_coercible_to_string(duk_context *ctx);

/* duk_push_(u)int() is guaranteed to support at least (un)signed 32-bit range */
#define duk_push_u32(ctx,val) \
	duk_push_uint((ctx), (duk_uint_t) (val))
#define duk_push_i32(ctx,val) \
	duk_push_int((ctx), (duk_int_t) (val))

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

DUK_INTERNAL_DECL duk_heaphdr *duk_get_tagged_heaphdr_raw(duk_context *ctx, duk_idx_t index, duk_uint_t flags_and_tag);

DUK_INTERNAL_DECL duk_hstring *duk_get_hstring(duk_context *ctx, duk_idx_t index);
DUK_INTERNAL_DECL duk_hobject *duk_get_hobject(duk_context *ctx, duk_idx_t index);
DUK_INTERNAL_DECL duk_hbuffer *duk_get_hbuffer(duk_context *ctx, duk_idx_t index);
DUK_INTERNAL_DECL duk_hthread *duk_get_hthread(duk_context *ctx, duk_idx_t index);
DUK_INTERNAL_DECL duk_hcompiledfunction *duk_get_hcompiledfunction(duk_context *ctx, duk_idx_t index);
DUK_INTERNAL_DECL duk_hnativefunction *duk_get_hnativefunction(duk_context *ctx, duk_idx_t index);

#define duk_get_hobject_with_class(ctx,index,classnum) \
	((duk_hobject *) duk_get_tagged_heaphdr_raw((ctx), (index), \
		DUK_TAG_OBJECT | DUK_GETTAGGED_FLAG_ALLOW_NULL | \
		DUK_GETTAGGED_FLAG_CHECK_CLASS | ((classnum) << DUK_GETTAGGED_CLASS_SHIFT)))

#if 0  /* This would be pointless: unexpected type and lightfunc would both return NULL */
DUK_INTERNAL_DECL duk_hobject *duk_get_hobject_or_lfunc(duk_context *ctx, duk_idx_t index);
#endif
DUK_INTERNAL_DECL duk_hobject *duk_get_hobject_or_lfunc_coerce(duk_context *ctx, duk_idx_t index);

#if 0  /*unused*/
DUK_INTERNAL_DECL void *duk_get_voidptr(duk_context *ctx, duk_idx_t index);
#endif

DUK_INTERNAL_DECL duk_hstring *duk_to_hstring(duk_context *ctx, duk_idx_t index);
DUK_INTERNAL_DECL duk_int_t duk_to_int_clamped_raw(duk_context *ctx, duk_idx_t index, duk_int_t minval, duk_int_t maxval, duk_bool_t *out_clamped);  /* out_clamped=NULL, RangeError if outside range */
DUK_INTERNAL_DECL duk_int_t duk_to_int_clamped(duk_context *ctx, duk_idx_t index, duk_int_t minval, duk_int_t maxval);
DUK_INTERNAL_DECL duk_int_t duk_to_int_check_range(duk_context *ctx, duk_idx_t index, duk_int_t minval, duk_int_t maxval);

DUK_INTERNAL_DECL duk_hstring *duk_require_hstring(duk_context *ctx, duk_idx_t index);
DUK_INTERNAL_DECL duk_hobject *duk_require_hobject(duk_context *ctx, duk_idx_t index);
DUK_INTERNAL_DECL duk_hbuffer *duk_require_hbuffer(duk_context *ctx, duk_idx_t index);
DUK_INTERNAL_DECL duk_hthread *duk_require_hthread(duk_context *ctx, duk_idx_t index);
#if 0  /*unused */
DUK_INTERNAL_DECL duk_hcompiledfunction *duk_require_hcompiledfunction(duk_context *ctx, duk_idx_t index);
#endif
DUK_INTERNAL_DECL duk_hnativefunction *duk_require_hnativefunction(duk_context *ctx, duk_idx_t index);

#define duk_require_hobject_with_class(ctx,index,classnum) \
	((duk_hobject *) duk_get_tagged_heaphdr_raw((ctx), (index), \
		DUK_TAG_OBJECT | \
		DUK_GETTAGGED_FLAG_CHECK_CLASS | ((classnum) << DUK_GETTAGGED_CLASS_SHIFT)))

DUK_INTERNAL_DECL duk_hobject *duk_require_hobject_or_lfunc(duk_context *ctx, duk_idx_t index);
DUK_INTERNAL_DECL duk_hobject *duk_require_hobject_or_lfunc_coerce(duk_context *ctx, duk_idx_t index);

#if defined(DUK_USE_DEBUGGER_SUPPORT)
DUK_INTERNAL_DECL void duk_push_unused(duk_context *ctx);
#endif
DUK_INTERNAL_DECL void duk_push_hstring(duk_context *ctx, duk_hstring *h);
DUK_INTERNAL_DECL void duk_push_hstring_stridx(duk_context *ctx, duk_small_int_t stridx);
DUK_INTERNAL_DECL void duk_push_hobject(duk_context *ctx, duk_hobject *h);
DUK_INTERNAL_DECL void duk_push_hbuffer(duk_context *ctx, duk_hbuffer *h);
#define duk_push_hthread(ctx,h) \
	duk_push_hobject((ctx), (duk_hobject *) (h))
#define duk_push_hcompiledfunction(ctx,h) \
	duk_push_hobject((ctx), (duk_hobject *) (h))
#define duk_push_hnativefunction(ctx,h) \
	duk_push_hobject((ctx), (duk_hobject *) (h))
DUK_INTERNAL_DECL void duk_push_hobject_bidx(duk_context *ctx, duk_small_int_t builtin_idx);
DUK_INTERNAL_DECL duk_idx_t duk_push_object_helper(duk_context *ctx, duk_uint_t hobject_flags_and_class, duk_small_int_t prototype_bidx);
DUK_INTERNAL_DECL duk_idx_t duk_push_object_helper_proto(duk_context *ctx, duk_uint_t hobject_flags_and_class, duk_hobject *proto);
DUK_INTERNAL_DECL duk_idx_t duk_push_object_internal(duk_context *ctx);
DUK_INTERNAL_DECL duk_idx_t duk_push_compiledfunction(duk_context *ctx);
DUK_INTERNAL_DECL void duk_push_c_function_noexotic(duk_context *ctx, duk_c_function func, duk_int_t nargs);
DUK_INTERNAL_DECL void duk_push_c_function_noconstruct_noexotic(duk_context *ctx, duk_c_function func, duk_int_t nargs);

DUK_INTERNAL_DECL void duk_push_string_funcptr(duk_context *ctx, duk_uint8_t *ptr, duk_size_t sz);
DUK_INTERNAL_DECL void duk_push_lightfunc_name(duk_context *ctx, duk_tval *tv);
DUK_INTERNAL_DECL void duk_push_lightfunc_tostring(duk_context *ctx, duk_tval *tv);

DUK_INTERNAL_DECL duk_bool_t duk_get_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx);     /* [] -> [val] */
DUK_INTERNAL_DECL duk_bool_t duk_put_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx);     /* [val] -> [] */
DUK_INTERNAL_DECL duk_bool_t duk_del_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx);     /* [] -> [] */
DUK_INTERNAL_DECL duk_bool_t duk_has_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx);     /* [] -> [] */

DUK_INTERNAL_DECL duk_bool_t duk_get_prop_stridx_boolean(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx, duk_bool_t *out_has_prop);  /* [] -> [] */

DUK_INTERNAL_DECL void duk_xdef_prop(duk_context *ctx, duk_idx_t obj_index, duk_small_uint_t desc_flags);  /* [key val] -> [] */
DUK_INTERNAL_DECL void duk_xdef_prop_index(duk_context *ctx, duk_idx_t obj_index, duk_uarridx_t arr_index, duk_small_uint_t desc_flags);  /* [val] -> [] */
DUK_INTERNAL_DECL void duk_xdef_prop_stridx(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx, duk_small_uint_t desc_flags);  /* [val] -> [] */
DUK_INTERNAL_DECL void duk_xdef_prop_stridx_builtin(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx, duk_small_int_t builtin_idx, duk_small_uint_t desc_flags);  /* [] -> [] */
DUK_INTERNAL_DECL void duk_xdef_prop_stridx_thrower(duk_context *ctx, duk_idx_t obj_index, duk_small_int_t stridx, duk_small_uint_t desc_flags);  /* [] -> [] */

/* These are macros for now, but could be separate functions to reduce code
 * footprint (check call site count before refactoring).
 */
#define duk_xdef_prop_wec(ctx,obj_index) \
	duk_xdef_prop((ctx), (obj_index), DUK_PROPDESC_FLAGS_WEC)
#define duk_xdef_prop_index_wec(ctx,obj_index,arr_index) \
	duk_xdef_prop_index((ctx), (obj_index), (arr_index), DUK_PROPDESC_FLAGS_WEC)
#define duk_xdef_prop_stridx_wec(ctx,obj_index,stridx) \
	duk_xdef_prop_stridx((ctx), (obj_index), (stridx), DUK_PROPDESC_FLAGS_WEC)

/* Set object 'length'. */
DUK_INTERNAL_DECL void duk_set_length(duk_context *ctx, duk_idx_t index, duk_size_t length);

#endif  /* DUK_API_INTERNAL_H_INCLUDED */
