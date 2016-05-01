/*
 *  Ecmascript execution, support primitives.
 */

#ifndef DUK_JS_H_INCLUDED
#define DUK_JS_H_INCLUDED

/* Flags for call handling. */
#define DUK_CALL_FLAG_IGNORE_RECLIMIT        (1 << 0)  /* duk_handle_call_xxx: call ignores C recursion limit (for errhandler calls) */
#define DUK_CALL_FLAG_CONSTRUCTOR_CALL       (1 << 1)  /* duk_handle_call_xxx: constructor call (i.e. called as 'new Foo()') */
#define DUK_CALL_FLAG_IS_RESUME              (1 << 2)  /* duk_handle_ecma_call_setup: setup for a resume() */
#define DUK_CALL_FLAG_IS_TAILCALL            (1 << 3)  /* duk_handle_ecma_call_setup: setup for a tail call */
#define DUK_CALL_FLAG_DIRECT_EVAL            (1 << 4)  /* call is a direct eval call */

/* Flags for duk_js_equals_helper(). */
#define DUK_EQUALS_FLAG_SAMEVALUE            (1 << 0)  /* use SameValue instead of non-strict equality */
#define DUK_EQUALS_FLAG_STRICT               (1 << 1)  /* use strict equality instead of non-strict equality */

/* Flags for duk_js_compare_helper(). */
#define DUK_COMPARE_FLAG_EVAL_LEFT_FIRST     (1 << 0)  /* eval left argument first */
#define DUK_COMPARE_FLAG_NEGATE              (1 << 1)  /* negate result */

/* conversions, coercions, comparison, etc */
DUK_INTERNAL_DECL duk_bool_t duk_js_toboolean(duk_tval *tv);
DUK_INTERNAL_DECL duk_double_t duk_js_tonumber(duk_hthread *thr, duk_tval *tv);
DUK_INTERNAL_DECL duk_double_t duk_js_tointeger_number(duk_double_t x);
DUK_INTERNAL_DECL duk_double_t duk_js_tointeger(duk_hthread *thr, duk_tval *tv);
DUK_INTERNAL_DECL duk_uint32_t duk_js_touint32(duk_hthread *thr, duk_tval *tv);
DUK_INTERNAL_DECL duk_int32_t duk_js_toint32(duk_hthread *thr, duk_tval *tv);
DUK_INTERNAL_DECL duk_uint16_t duk_js_touint16(duk_hthread *thr, duk_tval *tv);
DUK_INTERNAL_DECL duk_small_int_t duk_js_to_arrayindex_raw_string(const duk_uint8_t *str, duk_uint32_t blen, duk_uarridx_t *out_idx);
DUK_INTERNAL_DECL duk_uarridx_t duk_js_to_arrayindex_string_helper(duk_hstring *h);
DUK_INTERNAL_DECL duk_bool_t duk_js_equals_helper(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y, duk_small_int_t flags);
DUK_INTERNAL_DECL duk_small_int_t duk_js_data_compare(const duk_uint8_t *buf1, const duk_uint8_t *buf2, duk_size_t len1, duk_size_t len2);
DUK_INTERNAL_DECL duk_small_int_t duk_js_string_compare(duk_hstring *h1, duk_hstring *h2);
#if 0  /* unused */
DUK_INTERNAL_DECL duk_small_int_t duk_js_buffer_compare(duk_heap *heap, duk_hbuffer *h1, duk_hbuffer *h2);
#endif
DUK_INTERNAL_DECL duk_bool_t duk_js_compare_helper(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y, duk_small_int_t flags);
DUK_INTERNAL_DECL duk_bool_t duk_js_instanceof(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
DUK_INTERNAL_DECL duk_bool_t duk_js_in(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
DUK_INTERNAL_DECL duk_hstring *duk_js_typeof(duk_hthread *thr, duk_tval *tv_x);

#define duk_js_equals(thr,tv_x,tv_y) \
	duk_js_equals_helper((thr), (tv_x), (tv_y), 0)
#define duk_js_strict_equals(tv_x,tv_y) \
	duk_js_equals_helper(NULL, (tv_x), (tv_y), DUK_EQUALS_FLAG_STRICT)
#define duk_js_samevalue(tv_x,tv_y) \
	duk_js_equals_helper(NULL, (tv_x), (tv_y), DUK_EQUALS_FLAG_SAMEVALUE)

/* E5 Sections 11.8.1, 11.8.5; x < y */
#define duk_js_lessthan(thr,tv_x,tv_y) \
	duk_js_compare_helper((thr), (tv_x), (tv_Y), DUK_COMPARE_FLAG_EVAL_LEFT_FIRST)

/* E5 Sections 11.8.2, 11.8.5; x > y  -->  y < x */
#define duk_js_greaterthan(thr,tv_x,tv_y) \
	duk_js_compare_helper((thr), (tv_y), (tv_x), 0)

/* E5 Sections 11.8.3, 11.8.5; x <= y  -->  not (x > y)  -->  not (y < x) */
#define duk_js_lessthanorequal(thr,tv_x,tv_y) \
	duk_js_compare_helper((thr), (tv_y), (tv_x), DUK_COMPARE_FLAG_NEGATE)

/* E5 Sections 11.8.4, 11.8.5; x >= y  -->  not (x < y) */
#define duk_js_greaterthanorequal(thr,tv_x,tv_y) \
	duk_js_compare_helper((thr), (tv_x), (tv_y), DUK_COMPARE_FLAG_EVAL_LEFT_FIRST | DUK_COMPARE_FLAG_NEGATE)

/* identifiers and environment handling */
#if 0  /*unused*/
DUK_INTERNAL duk_bool_t duk_js_hasvar_envrec(duk_hthread *thr, duk_hobject *env, duk_hstring *name);
#endif
DUK_INTERNAL_DECL duk_bool_t duk_js_getvar_envrec(duk_hthread *thr, duk_hobject *env, duk_hstring *name, duk_bool_t throw_flag);
DUK_INTERNAL_DECL duk_bool_t duk_js_getvar_activation(duk_hthread *thr, duk_activation *act, duk_hstring *name, duk_bool_t throw_flag);
DUK_INTERNAL_DECL void duk_js_putvar_envrec(duk_hthread *thr, duk_hobject *env, duk_hstring *name, duk_tval *val, duk_bool_t strict);
DUK_INTERNAL_DECL void duk_js_putvar_activation(duk_hthread *thr, duk_activation *act, duk_hstring *name, duk_tval *val, duk_bool_t strict);
#if 0  /*unused*/
DUK_INTERNAL_DECL duk_bool_t duk_js_delvar_envrec(duk_hthread *thr, duk_hobject *env, duk_hstring *name);
#endif
DUK_INTERNAL_DECL duk_bool_t duk_js_delvar_activation(duk_hthread *thr, duk_activation *act, duk_hstring *name);
DUK_INTERNAL_DECL duk_bool_t duk_js_declvar_activation(duk_hthread *thr, duk_activation *act, duk_hstring *name, duk_tval *val, duk_small_int_t prop_flags, duk_bool_t is_func_decl);
DUK_INTERNAL_DECL void duk_js_init_activation_environment_records_delayed(duk_hthread *thr, duk_activation *act);
DUK_INTERNAL_DECL void duk_js_close_environment_record(duk_hthread *thr, duk_hobject *env, duk_hobject *func, duk_size_t regbase);
DUK_INTERNAL_DECL duk_hobject *duk_create_activation_environment_record(duk_hthread *thr, duk_hobject *func, duk_size_t idx_bottom);
DUK_INTERNAL_DECL
void duk_js_push_closure(duk_hthread *thr,
                         duk_hcompiledfunction *fun_temp,
                         duk_hobject *outer_var_env,
                         duk_hobject *outer_lex_env,
                         duk_bool_t add_auto_proto);

/* call handling */
DUK_INTERNAL_DECL duk_int_t duk_handle_call_protected(duk_hthread *thr, duk_idx_t num_stack_args, duk_small_uint_t call_flags);
DUK_INTERNAL_DECL void duk_handle_call_unprotected(duk_hthread *thr, duk_idx_t num_stack_args, duk_small_uint_t call_flags);
DUK_INTERNAL_DECL duk_int_t duk_handle_safe_call(duk_hthread *thr, duk_safe_call_function func, duk_idx_t num_stack_args, duk_idx_t num_stack_res);
DUK_INTERNAL_DECL duk_bool_t duk_handle_ecma_call_setup(duk_hthread *thr, duk_idx_t num_stack_args, duk_small_uint_t call_flags);

/* bytecode execution */
DUK_INTERNAL_DECL void duk_js_execute_bytecode(duk_hthread *exec_thr);

#endif  /* DUK_JS_H_INCLUDED */
