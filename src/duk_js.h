/*
 *  Ecmascript execution, support primitives.
 */

#ifndef DUK_JS_H_INCLUDED
#define DUK_JS_H_INCLUDED

/* Flags for call handling. */
#define DUK_CALL_FLAG_PROTECTED              (1 << 0)  /* duk_handle_call: call is protected */
#define DUK_CALL_FLAG_IGNORE_RECLIMIT        (1 << 1)  /* duk_handle_call: call ignores C recursion limit (for errhandler calls) */
#define DUK_CALL_FLAG_CONSTRUCTOR_CALL       (1 << 2)  /* duk_handle_call: constructor call (i.e. called as 'new Foo()') */
#define DUK_CALL_FLAG_IS_RESUME              (1 << 3)  /* duk_handle_ecma_call_setup: setup for a resume() */
#define DUK_CALL_FLAG_IS_TAILCALL            (1 << 4)  /* duk_handle_ecma_call_setup: setup for a tailcall */
#define DUK_CALL_FLAG_DIRECT_EVAL            (1 << 5)  /* call is a direct eval call */

/* Flags for duk__js_equals_helper(). */
#define DUK_EQUALS_FLAG_SAMEVALUE            (1 << 0)  /* use SameValue instead of non-strict equality */
#define DUK_EQUALS_FLAG_STRICT               (1 << 1)  /* use strict equality instead of non-strict equality */

/* conversions, coercions, comparison, etc */
int duk_js_toboolean(duk_tval *tv);
double duk_js_tonumber(duk_hthread *thr, duk_tval *tv);
double duk_js_tointeger_number(double x);
double duk_js_tointeger(duk_hthread *thr, duk_tval *tv);
duk_uint32_t duk_js_touint32_number(double x);
duk_uint32_t duk_js_touint32(duk_hthread *thr, duk_tval *tv);
duk_int32_t duk_js_toint32_number(double x);
duk_int32_t duk_js_toint32(duk_hthread *thr, duk_tval *tv);
duk_uint16_t duk_js_touint16_number(double x);
duk_uint16_t duk_js_touint16(duk_hthread *thr, duk_tval *tv);
duk_small_int_t duk_js_to_arrayindex_raw_string(duk_uint8_t *str, duk_uint32_t blen, duk_uint32_t *out_idx);
duk_uint32_t duk_js_to_arrayindex_string_helper(duk_hstring *h);
int duk_js_equals_helper(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y, duk_small_int_t flags);
int duk_js_string_compare(duk_hstring *h1, duk_hstring *h2);
int duk_js_compare_helper(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y, int eval_left_first, int negate);
int duk_js_lessthan(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
int duk_js_greaterthan(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
int duk_js_lessthanorequal(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
int duk_js_greaterthanorequal(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
int duk_js_instanceof(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
int duk_js_in(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
duk_hstring *duk_js_typeof(duk_hthread *thr, duk_tval *tv_x);

#define duk_js_equals(thr,tv_x,tv_y) \
	duk_js_equals_helper((thr), (tv_x), (tv_y), 0)
#define duk_js_strict_equals(tv_x,tv_y) \
	duk_js_equals_helper(NULL, (tv_x), (tv_y), DUK_EQUALS_FLAG_STRICT)
#define duk_js_samevalue(tv_x,tv_y) \
	duk_js_equals_helper(NULL, (tv_x), (tv_y), DUK_EQUALS_FLAG_SAMEVALUE)

/* E5 Sections 11.8.1, 11.8.5; x < y */
#define duk_js_lessthan(thr,tv_x,tv_y) \
	duk_js_compare_helper((thr), (tv_x), (tv_Y), 1, 0)

/* E5 Sections 11.8.2, 11.8.5; x > y  -->  y < x */
#define duk_js_greaterthan(thr,tv_x,tv_y) \
	duk_js_compare_helper((thr), (tv_y), (tv_x), 0, 0)

/* E5 Sections 11.8.3, 11.8.5; x <= y  -->  not (x > y)  -->  not (y < x) */
#define duk_js_lessthanorequal(thr,tv_x,tv_y) \
	duk_js_compare_helper((thr), (tv_y), (tv_x), 0, 1);

/* E5 Sections 11.8.4, 11.8.5; x >= y  -->  not (x < y) */
#define duk_js_greaterthanorequal(thr,tv_x,tv_y) \
	duk_js_compare_helper((thr), (tv_x), (tv_y), 1, 1);

/* identifiers and environment handling */
int duk_js_getvar_envrec(duk_hthread *thr, duk_hobject *env, duk_hstring *name, int throw_flag);
int duk_js_getvar_activation(duk_hthread *thr, duk_activation *act, duk_hstring *name, int throw_flag);
void duk_js_putvar_envrec(duk_hthread *thr, duk_hobject *env, duk_hstring *name, duk_tval *val, int strict);
void duk_js_putvar_activation(duk_hthread *thr, duk_activation *act, duk_hstring *name, duk_tval *val, int strict);
int duk_js_delvar_envrec(duk_hthread *thr, duk_hobject *env, duk_hstring *name);
int duk_js_delvar_activation(duk_hthread *thr, duk_activation *act, duk_hstring *name);
int duk_js_declvar_activation(duk_hthread *thr, duk_activation *act, duk_hstring *name, duk_tval *val, int prop_flags, int is_func_decl);
void duk_js_init_activation_environment_records_delayed(duk_hthread *thr, duk_activation *act);
void duk_js_close_environment_record(duk_hthread *thr, duk_hobject *env, duk_hobject *func, int regbase);
duk_hobject *duk_create_activation_environment_record(duk_hthread *thr, duk_hobject *func, duk_uint32_t reg_bottom);
void duk_js_push_closure(duk_hthread *thr,
                         duk_hcompiledfunction *fun_temp,
                         duk_hobject *outer_var_env,
                         duk_hobject *outer_lex_env);

/* call handling */
int duk_handle_call(duk_hthread *thr,
                    int num_stack_args,
                    int call_flags);
int duk_handle_safe_call(duk_hthread *thr,
                         duk_safe_call_function func,
                         int num_stack_args,
                         int num_stack_res);
void duk_handle_ecma_call_setup(duk_hthread *thr,
                                int num_stack_args,
                                int call_flags);

/* bytecode execution */
void duk_js_execute_bytecode(duk_hthread *entry_thread);

#endif  /* DUK_JS_H_INCLUDED */

