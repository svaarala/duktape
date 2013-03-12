/*
 *  Internal API calls which have (stack and other) semantics
 *  similar to the public API.
 */

#ifndef DUK_API_INTERNAL_H_INCLUDED
#define DUK_API_INTERNAL_H_INCLUDED

/* duk_push_sprintf constants */
#define  DUK_PUSH_SPRINTF_INITIAL_SIZE  256
#define  DUK_PUSH_SPRINTF_SANITY_LIMIT  (1*1024*1024*1024)

int duk_check_valstack_resize(duk_context *ctx, unsigned int min_new_size, int allow_shrink);
void duk_require_valstack_resize(duk_context *ctx, unsigned int min_new_size, int allow_shrink);

int duk_check_stack_raw(duk_context *ctx, unsigned int extra);
void duk_require_stack_raw(duk_context *ctx, unsigned int extra);

duk_tval *duk_get_tval(duk_context *ctx, int index);
duk_tval duk_get_tval_value(duk_context *ctx, int index);      /* FIXME: not implemented now */
duk_tval *duk_require_tval(duk_context *ctx, int index);
duk_tval duk_require_tval_value(duk_context *ctx, int index);  /* FIXME: not implemented now */
void duk_push_tval(duk_context *ctx, duk_tval *tv);
void duk_push_tval_value(duk_context *ctx, duk_tval tv);       /* FIXME: not implemented now */

void duk_push_this_check_object_coercible(duk_context *ctx);   /* push the current 'this' binding; throw TypeError
                                                                * if binding is not object coercible (CheckObjectCoercible).
                                                                */
void duk_push_this_coercible_to_object(duk_context *ctx);       /* duk_push_this() + CheckObjectCoercible() + duk_to_object() */
void duk_push_this_coercible_to_string(duk_context *ctx);       /* duk_push_this() + CheckObjectCoercible() + duk_to_string() */

duk_hstring *duk_get_hstring(duk_context *ctx, int index);
duk_hobject *duk_get_hobject(duk_context *ctx, int index);
duk_hobject *duk_get_hobject_with_class(duk_context *ctx, int index, int class);
duk_hbuffer *duk_get_hbuffer(duk_context *ctx, int index);
duk_hthread *duk_get_hthread(duk_context *ctx, int index);
duk_hcompiledfunction *duk_get_hcompiledfunction(duk_context *ctx, int index);
duk_hnativefunction *duk_get_hnativefunction(duk_context *ctx, int index);

/* FIXME: specific getters for e.g. thread; duk_get_hobject_with_flags()
 * could be the underlying primitive?
 */

duk_hstring *duk_to_hstring(duk_context *ctx, int index);
int duk_to_int_clamped(duk_context *ctx, int index, int minval, int maxval);

duk_hstring *duk_require_hstring(duk_context *ctx, int index);
duk_hobject *duk_require_hobject(duk_context *ctx, int index);
duk_hobject *duk_require_hobject_with_class(duk_context *ctx, int index, int class);
duk_hbuffer *duk_require_hbuffer(duk_context *ctx, int index);
duk_hthread *duk_require_hthread(duk_context *ctx, int index);
duk_hcompiledfunction *duk_require_hcompiledfunction(duk_context *ctx, int index);
duk_hnativefunction *duk_require_hnativefunction(duk_context *ctx, int index);

void duk_push_unused(duk_context *ctx);
void duk_push_hstring(duk_context *ctx, duk_hstring *h);
void duk_push_hstring_stridx(duk_context *ctx, int stridx);
void duk_push_hobject(duk_context *ctx, duk_hobject *h);
void duk_push_hbuffer(duk_context *ctx, duk_hbuffer *h);
void duk_push_builtin(duk_context *ctx, int builtin_idx);
int duk_push_new_object_helper(duk_context *ctx, int hobject_flags_and_class, int prototype_bidx);
int duk_push_new_object_internal(duk_context *ctx);
int duk_push_new_compiledfunction(duk_context *ctx);

int duk_get_prop_stridx(duk_context *ctx, int obj_index, unsigned int stridx);     /* [] -> [val] */
int duk_put_prop_stridx(duk_context *ctx, int obj_index, unsigned int stridx);     /* [val] -> [] */
int duk_del_prop_stridx(duk_context *ctx, int obj_index, unsigned int stridx);     /* [] -> [] */
int duk_has_prop_stridx(duk_context *ctx, int obj_index, unsigned int stridx);     /* [] -> [] */

void duk_def_prop(duk_context *ctx, int obj_index, int desc_flags);  /* [key val] -> [] */
void duk_def_prop_stridx(duk_context *ctx, int obj_index, unsigned int stridx, int desc_flags);  /* [val] -> [] */
void duk_def_prop_stridx_builtin(duk_context *ctx, int obj_index, unsigned int stridx, unsigned int builtin_idx, int desc_flags);  /* [] -> [] */

void duk_def_prop_stridx_thrower(duk_context *ctx, int obj_index, unsigned int stridx, int desc_flags);  /* [] -> [] */

#endif  /* DUK_API_INTERNAL_H_INCLUDED */

