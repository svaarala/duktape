/*
 *  Shared error message strings
 *
 *  To minimize code footprint, try to share error messages inside Duktape
 *  code.  Modern compilers will do this automatically anyway, this is mostly
 *  for older compilers.
 */

#include "duk_internal.h"

/* Mostly API and built-in method related */
DUK_INTERNAL const char *duk_str_internal_error = "internal error";
DUK_INTERNAL const char *duk_str_invalid_count = "invalid count";
DUK_INTERNAL const char *duk_str_invalid_call_args = "invalid call args";
DUK_INTERNAL const char *duk_str_not_constructable = "not constructable";
DUK_INTERNAL const char *duk_str_not_callable = "not callable";
DUK_INTERNAL const char *duk_str_not_extensible = "not extensible";
DUK_INTERNAL const char *duk_str_not_writable = "not writable";
DUK_INTERNAL const char *duk_str_not_configurable = "not configurable";

DUK_INTERNAL const char *duk_str_invalid_context = "invalid context";
DUK_INTERNAL const char *duk_str_invalid_index = "invalid index";
DUK_INTERNAL const char *duk_str_push_beyond_alloc_stack = "attempt to push beyond currently allocated stack";
DUK_INTERNAL const char *duk_str_not_undefined = "not undefined";
DUK_INTERNAL const char *duk_str_not_null = "not null";
DUK_INTERNAL const char *duk_str_not_boolean = "not boolean";
DUK_INTERNAL const char *duk_str_not_number = "not number";
DUK_INTERNAL const char *duk_str_not_string = "not string";
DUK_INTERNAL const char *duk_str_not_pointer = "not pointer";
DUK_INTERNAL const char *duk_str_not_buffer = "not buffer";
DUK_INTERNAL const char *duk_str_unexpected_type = "unexpected type";
DUK_INTERNAL const char *duk_str_not_thread = "not thread";
DUK_INTERNAL const char *duk_str_not_compiledfunction = "not compiledfunction";
DUK_INTERNAL const char *duk_str_not_nativefunction = "not nativefunction";
DUK_INTERNAL const char *duk_str_not_c_function = "not c function";
DUK_INTERNAL const char *duk_str_defaultvalue_coerce_failed = "[[DefaultValue]] coerce failed";
DUK_INTERNAL const char *duk_str_number_outside_range = "number outside range";
DUK_INTERNAL const char *duk_str_not_object_coercible = "not object coercible";
DUK_INTERNAL const char *duk_str_string_too_long = "string too long";
DUK_INTERNAL const char *duk_str_buffer_too_long = "buffer too long";
DUK_INTERNAL const char *duk_str_sprintf_too_long = "sprintf message too long";
DUK_INTERNAL const char *duk_str_alloc_failed = "alloc failed";
DUK_INTERNAL const char *duk_str_pop_too_many = "attempt to pop too many entries";
DUK_INTERNAL const char *duk_str_wrong_buffer_type = "wrong buffer type";
DUK_INTERNAL const char *duk_str_failed_to_extend_valstack = "failed to extend valstack";
DUK_INTERNAL const char *duk_str_encode_failed = "encode failed";
DUK_INTERNAL const char *duk_str_decode_failed = "decode failed";
DUK_INTERNAL const char *duk_str_no_sourcecode = "no sourcecode";
DUK_INTERNAL const char *duk_str_concat_result_too_long = "concat result too long";
DUK_INTERNAL const char *duk_str_unimplemented = "unimplemented";
DUK_INTERNAL const char *duk_str_unsupported = "unsupported";
DUK_INTERNAL const char *duk_str_array_length_over_2g = "array length over 2G";

/* JSON */
DUK_INTERNAL const char *duk_str_fmt_ptr = "%p";
DUK_INTERNAL const char *duk_str_fmt_invalid_json = "invalid json (at offset %ld)";
DUK_INTERNAL const char *duk_str_jsondec_reclimit = "json decode recursion limit";
DUK_INTERNAL const char *duk_str_jsonenc_reclimit = "json encode recursion limit";
DUK_INTERNAL const char *duk_str_cyclic_input = "cyclic input";

/* Object property access */
DUK_INTERNAL const char *duk_str_proxy_revoked = "proxy revoked";
DUK_INTERNAL const char *duk_str_object_resize_failed = "object resize failed";
DUK_INTERNAL const char *duk_str_invalid_base = "invalid base value";
DUK_INTERNAL const char *duk_str_strict_caller_read = "attempt to read strict 'caller'";
DUK_INTERNAL const char *duk_str_proxy_rejected = "proxy rejected";
DUK_INTERNAL const char *duk_str_invalid_array_length = "invalid array length";
DUK_INTERNAL const char *duk_str_array_length_write_failed = "array length write failed";
DUK_INTERNAL const char *duk_str_array_length_not_writable = "array length non-writable";
DUK_INTERNAL const char *duk_str_setter_undefined = "setter undefined";
DUK_INTERNAL const char *duk_str_redefine_virt_prop = "attempt to redefine virtual property";
DUK_INTERNAL const char *duk_str_invalid_descriptor = "invalid descriptor";
DUK_INTERNAL const char *duk_str_property_is_virtual = "property is virtual";

/* Compiler */
DUK_INTERNAL const char *duk_str_parse_error = "parse error";
DUK_INTERNAL const char *duk_str_duplicate_label = "duplicate label";
DUK_INTERNAL const char *duk_str_invalid_label = "invalid label";
DUK_INTERNAL const char *duk_str_invalid_array_literal = "invalid array literal";
DUK_INTERNAL const char *duk_str_invalid_object_literal = "invalid object literal";
DUK_INTERNAL const char *duk_str_invalid_var_declaration = "invalid variable declaration";
DUK_INTERNAL const char *duk_str_cannot_delete_identifier = "cannot delete identifier";
DUK_INTERNAL const char *duk_str_invalid_expression = "invalid expression";
DUK_INTERNAL const char *duk_str_invalid_lvalue = "invalid lvalue";
DUK_INTERNAL const char *duk_str_expected_identifier = "expected identifier";
DUK_INTERNAL const char *duk_str_empty_expr_not_allowed = "empty expression not allowed";
DUK_INTERNAL const char *duk_str_invalid_for = "invalid for statement";
DUK_INTERNAL const char *duk_str_invalid_switch = "invalid switch statement";
DUK_INTERNAL const char *duk_str_invalid_break_cont_label = "invalid break/continue label";
DUK_INTERNAL const char *duk_str_invalid_return = "invalid return";
DUK_INTERNAL const char *duk_str_invalid_try = "invalid try";
DUK_INTERNAL const char *duk_str_invalid_throw = "invalid throw";
DUK_INTERNAL const char *duk_str_with_in_strict_mode = "with in strict mode";
DUK_INTERNAL const char *duk_str_func_stmt_not_allowed = "function statement not allowed";
DUK_INTERNAL const char *duk_str_unterminated_stmt = "unterminated statement";
DUK_INTERNAL const char *duk_str_invalid_arg_name = "invalid argument name";
DUK_INTERNAL const char *duk_str_invalid_func_name = "invalid function name";
DUK_INTERNAL const char *duk_str_invalid_getset_name = "invalid getter/setter name";
DUK_INTERNAL const char *duk_str_func_name_required = "function name required";

/* Executor */
DUK_INTERNAL const char *duk_str_internal_error_exec_longjmp = "internal error in bytecode executor longjmp handler";

/* Regexp */
DUK_INTERNAL const char *duk_str_invalid_quantifier_no_atom = "quantifier without preceding atom";
DUK_INTERNAL const char *duk_str_invalid_quantifier_values = "quantifier values invalid (qmin > qmax)";
DUK_INTERNAL const char *duk_str_quantifier_too_many_copies = "quantifier expansion requires too many atom copies";
DUK_INTERNAL const char *duk_str_unexpected_closing_paren = "unexpected closing parenthesis";
DUK_INTERNAL const char *duk_str_unexpected_end_of_pattern = "unexpected end of pattern";
DUK_INTERNAL const char *duk_str_unexpected_regexp_token = "unexpected token in regexp";
DUK_INTERNAL const char *duk_str_invalid_regexp_flags = "invalid regexp flags";
DUK_INTERNAL const char *duk_str_invalid_backrefs = "invalid backreference(s)";
DUK_INTERNAL const char *duk_str_regexp_backtrack_failed = "regexp backtrack failed";
DUK_INTERNAL const char *duk_str_regexp_advance_failed = "regexp advance failed";
DUK_INTERNAL const char *duk_str_regexp_internal_error = "regexp internal error";

/* Limits */
DUK_INTERNAL const char *duk_str_valstack_limit = "valstack limit";
DUK_INTERNAL const char *duk_str_callstack_limit = "callstack limit";
DUK_INTERNAL const char *duk_str_catchstack_limit = "catchstack limit";
DUK_INTERNAL const char *duk_str_object_property_limit = "object property limit";
DUK_INTERNAL const char *duk_str_prototype_chain_limit = "prototype chain limit";
DUK_INTERNAL const char *duk_str_bound_chain_limit = "function call bound chain limit";
DUK_INTERNAL const char *duk_str_c_callstack_limit = "C call stack depth limit";
DUK_INTERNAL const char *duk_str_compiler_recursion_limit = "compiler recursion limit";
DUK_INTERNAL const char *duk_str_bytecode_limit = "bytecode limit";
DUK_INTERNAL const char *duk_str_reg_limit = "register limit";
DUK_INTERNAL const char *duk_str_temp_limit = "temp limit";
DUK_INTERNAL const char *duk_str_const_limit = "const limit";
DUK_INTERNAL const char *duk_str_func_limit = "function limit";
DUK_INTERNAL const char *duk_str_regexp_compiler_recursion_limit = "regexp compiler recursion limit";
DUK_INTERNAL const char *duk_str_regexp_executor_recursion_limit = "regexp executor recursion limit";
DUK_INTERNAL const char *duk_str_regexp_executor_step_limit = "regexp step limit";

/* Misc */
DUK_INTERNAL const char *duk_str_anon = "anon";
DUK_INTERNAL const char *duk_str_realloc_failed = "realloc failed";
