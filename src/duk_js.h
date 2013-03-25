/*
 *  Ecmascript execution, support primitives.
 */

#ifndef DUK_JS_H_INCLUDED
#define DUK_JS_H_INCLUDED

/* call flags */
#define  DUK_CALL_FLAG_PROTECTED              (1 << 0)  /* duk_handle_call: call is protected */
#define  DUK_CALL_FLAG_IGNORE_RECLIMIT        (1 << 1)  /* duk_handle_call: call ignores C recursion limit (for errhandler calls) */
#define  DUK_CALL_FLAG_CONSTRUCTOR_CALL       (1 << 2)  /* duk_handle_call: constructor call (i.e. called as 'new Foo()') */
#define  DUK_CALL_FLAG_IS_RESUME              (1 << 3)  /* duk_handle_ecma_call_setup: setup for a resume() */
#define  DUK_CALL_FLAG_IS_TAILCALL            (1 << 4)  /* duk_handle_ecma_call_setup: setup for a tailcall */
#define  DUK_CALL_FLAG_DIRECT_EVAL            (1 << 5)  /* call is a direct eval call */

/* conversions, coercions, comparison, etc */
int duk_js_toboolean(duk_tval *tv);
double duk_js_tonumber(duk_hthread *thr, duk_tval *tv);
double duk_js_tointeger_number(double x);
double duk_js_tointeger(duk_hthread *thr, duk_tval *tv);
duk_u32 duk_js_touint32_number(double x);
duk_u32 duk_js_touint32(duk_hthread *thr, duk_tval *tv);
duk_i32 duk_js_toint32_number(double x);
duk_i32 duk_js_toint32(duk_hthread *thr, duk_tval *tv);
duk_u16 duk_js_touint16_number(double x);
duk_u16 duk_js_touint16(duk_hthread *thr, duk_tval *tv);
int duk_js_is_arrayindex_raw_string(duk_u8 *str, duk_u32 blen);
int duk_js_to_arrayindex_string(duk_hstring *h, duk_u32 *out_idx);
duk_u32 duk_js_to_arrayindex_string_helper(duk_hstring *h);
int duk_js_equals_number(double x, double y);
int duk_js_equals(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
int duk_js_strict_equals(duk_tval *tv_x, duk_tval *tv_y);
int duk_js_samevalue_number(double x, double y);
int duk_js_samevalue(duk_tval *tv_x, duk_tval *tv_y);
int duk_js_string_compare(duk_hstring *h1, duk_hstring *h2);
int duk_js_compare_helper(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y, int eval_left_first, int negate);
int duk_js_lessthan(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
int duk_js_greaterthan(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
int duk_js_lessthanorequal(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
int duk_js_greaterthanorequal(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
int duk_js_instanceof(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
int duk_js_in(duk_hthread *thr, duk_tval *tv_x, duk_tval *tv_y);
duk_hstring *duk_js_typeof(duk_hthread *thr, duk_tval *tv_x);

/* identifiers and environment handling */
int duk_js_getvar_envrec(duk_hthread *thr, duk_hobject *env, duk_hstring *name, int throw);
int duk_js_getvar_activation(duk_hthread *thr, duk_activation *act, duk_hstring *name, int throw);
void duk_js_putvar_envrec(duk_hthread *thr, duk_hobject *env, duk_hstring *name, duk_tval *val, int strict);
void duk_js_putvar_activation(duk_hthread *thr, duk_activation *act, duk_hstring *name, duk_tval *val, int strict);
int duk_js_delvar_envrec(duk_hthread *thr, duk_hobject *env, duk_hstring *name);
int duk_js_delvar_activation(duk_hthread *thr, duk_activation *act, duk_hstring *name);
int duk_js_declvar_activation(duk_hthread *thr, duk_activation *act, duk_hstring *name, duk_tval *val, int prop_flags, int is_func_decl);
void duk_js_init_activation_environment_records_delayed(duk_hthread *thr, duk_activation *act);
void duk_js_close_environment_record(duk_hthread *thr, duk_hobject *env, duk_hobject *func, int regbase);
duk_hobject *duk_create_activation_environment_record(duk_hthread *thr, duk_hobject *func, duk_u32 reg_bottom);
void duk_js_push_closure(duk_hthread *thr,
                         duk_hcompiledfunction *fun_temp,
                         duk_hobject *outer_var_env,
                         duk_hobject *outer_lex_env);

/* call handling */
int duk_handle_call(duk_hthread *thr,
                    int num_stack_args,
                    int call_flags,
                    duk_hobject *errhandler);
int duk_handle_safe_call(duk_hthread *thr,
                         duk_safe_call_function func,
                         int num_stack_args,
                         int num_stack_res,
                         duk_hobject *errhandler);
void duk_handle_ecma_call_setup(duk_hthread *thr,
                                int num_stack_args,
                                int call_flags);

/* bytecode execution */
void duk_js_execute_bytecode(duk_hthread *entry_thread);

#endif  /* DUK_JS_H_INCLUDED */

