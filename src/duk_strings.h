/*
 *  Shared error messages: externs and macros
 *
 *  Error messages are accessed through macros with fine-grained, explicit
 *  error message distinctions.  Concrete error messages are selected by the
 *  macros and multiple macros can map to the same concrete string to save
 *  on code footprint.  This allows flexible footprint/verbosity tuning with
 *  minimal code impact.  There are a few limitations to this approach:
 *  (1) switching between plain messages and format strings doesn't work
 *  conveniently, and (2) conditional strings are a bit awkward to handle.
 *
 *  Because format strings behave differently in the call site (they need to
 *  be followed by format arguments), they have a special prefix (DUK_STR_FMT_
 *  and duk_str_fmt_).
 *
 *  On some compilers using explicit shared strings is preferable; on others
 *  it may be better to use straight literals because the compiler will combine
 *  them anyway, and such strings won't end up unnecessarily in a symbol table.
 */

#ifndef DUK_ERRMSG_H_INCLUDED
#define DUK_ERRMSG_H_INCLUDED

#define DUK_STR_INTERNAL_ERROR duk_str_internal_error
#define DUK_STR_INVALID_COUNT duk_str_invalid_count
#define DUK_STR_INVALID_CALL_ARGS duk_str_invalid_call_args
#define DUK_STR_NOT_CONSTRUCTABLE duk_str_not_constructable
#define DUK_STR_NOT_CALLABLE duk_str_not_callable
#define DUK_STR_NOT_EXTENSIBLE duk_str_not_extensible
#define DUK_STR_NOT_WRITABLE duk_str_not_writable
#define DUK_STR_NOT_CONFIGURABLE duk_str_not_configurable

extern const char *duk_str_internal_error;
extern const char *duk_str_invalid_count;
extern const char *duk_str_invalid_call_args;
extern const char *duk_str_not_constructable;
extern const char *duk_str_not_callable;
extern const char *duk_str_not_extensible;
extern const char *duk_str_not_writable;
extern const char *duk_str_not_configurable;

#define DUK_STR_INVALID_INDEX duk_str_invalid_index
#define DUK_STR_PUSH_BEYOND_ALLOC_STACK duk_str_push_beyond_alloc_stack
#define DUK_STR_SRC_STACK_NOT_ENOUGH duk_str_src_stack_not_enough
#define DUK_STR_NOT_UNDEFINED duk_str_not_undefined
#define DUK_STR_NOT_NULL duk_str_not_null
#define DUK_STR_NOT_BOOLEAN duk_str_not_boolean
#define DUK_STR_NOT_NUMBER duk_str_not_number
#define DUK_STR_NOT_STRING duk_str_not_string
#define DUK_STR_NOT_POINTER duk_str_not_pointer
#define DUK_STR_NOT_BUFFER duk_str_not_buffer
#define DUK_STR_NOT_OBJECT duk_str_not_object
#define DUK_STR_UNEXPECTED_TYPE duk_str_unexpected_type
#define DUK_STR_NOT_THREAD duk_str_not_thread
#define DUK_STR_NOT_COMPILEDFUNCTION duk_str_not_compiledfunction
#define DUK_STR_NOT_NATIVEFUNCTION duk_str_not_nativefunction
#define DUK_STR_NOT_C_FUNCTION duk_str_not_c_function
#define DUK_STR_DEFAULTVALUE_COERCE_FAILED duk_str_defaultvalue_coerce_failed
#define DUK_STR_NUMBER_OUTSIDE_RANGE duk_str_number_outside_range
#define DUK_STR_NOT_OBJECT_COERCIBLE duk_str_not_object_coercible
#define DUK_STR_STRING_TOO_LONG duk_str_string_too_long
#define DUK_STR_BUFFER_TOO_LONG duk_str_buffer_too_long
#define DUK_STR_SPRINTF_TOO_LONG duk_str_sprintf_too_long
#define DUK_STR_OBJECT_ALLOC_FAILED duk_str_object_alloc_failed
#define DUK_STR_THREAD_ALLOC_FAILED duk_str_thread_alloc_failed
#define DUK_STR_FUNC_ALLOC_FAILED duk_str_func_alloc_failed
#define DUK_STR_BUFFER_ALLOC_FAILED duk_str_buffer_alloc_failed
#define DUK_STR_POP_TOO_MANY duk_str_pop_too_many
#define DUK_STR_BUFFER_NOT_DYNAMIC duk_str_buffer_not_dynamic
#define DUK_STR_FAILED_TO_EXTEND_VALSTACK duk_str_failed_to_extend_valstack
#define DUK_STR_BASE64_ENCODE_FAILED duk_str_base64_encode_failed
#define DUK_STR_BASE64_DECODE_FAILED duk_str_base64_decode_failed
#define DUK_STR_HEX_DECODE_FAILED duk_str_hex_decode_failed
#define DUK_STR_NO_SOURCECODE duk_str_no_sourcecode
#define DUK_STR_CONCAT_RESULT_TOO_LONG duk_str_concat_result_too_long
#define DUK_STR_UNIMPLEMENTED duk_str_unimplemented
#define DUK_STR_ARRAY_LENGTH_OVER_2G duk_str_array_length_over_2g

extern const char *duk_str_invalid_index;
extern const char *duk_str_push_beyond_alloc_stack;
extern const char *duk_str_src_stack_not_enough;
extern const char *duk_str_not_undefined;
extern const char *duk_str_not_null;
extern const char *duk_str_not_boolean;
extern const char *duk_str_not_number;
extern const char *duk_str_not_string;
extern const char *duk_str_not_pointer;
extern const char *duk_str_not_buffer;
extern const char *duk_str_not_object;
extern const char *duk_str_unexpected_type;
extern const char *duk_str_not_thread;
extern const char *duk_str_not_compiledfunction;
extern const char *duk_str_not_nativefunction;
extern const char *duk_str_not_c_function;
extern const char *duk_str_defaultvalue_coerce_failed;
extern const char *duk_str_number_outside_range;
extern const char *duk_str_not_object_coercible;
extern const char *duk_str_string_too_long;
extern const char *duk_str_buffer_too_long;
extern const char *duk_str_sprintf_too_long;
extern const char *duk_str_object_alloc_failed;
extern const char *duk_str_thread_alloc_failed;
extern const char *duk_str_func_alloc_failed;
extern const char *duk_str_buffer_alloc_failed;
extern const char *duk_str_pop_too_many;
extern const char *duk_str_buffer_not_dynamic;
extern const char *duk_str_failed_to_extend_valstack;
extern const char *duk_str_base64_encode_failed;
extern const char *duk_str_base64_decode_failed;
extern const char *duk_str_hex_decode_failed;
extern const char *duk_str_no_sourcecode;
extern const char *duk_str_concat_result_too_long;
extern const char *duk_str_unimplemented;
extern const char *duk_str_array_length_over_2g;

#define DUK_STR_FMT_PTR duk_str_fmt_ptr
#define DUK_STR_INVALID_JSON duk_str_invalid_json
#define DUK_STR_INVALID_NUMBER duk_str_invalid_number
#define DUK_STR_JSONDEC_RECLIMIT duk_str_jsondec_reclimit
#define DUK_STR_JSONENC_RECLIMIT duk_str_jsonenc_reclimit
#define DUK_STR_CYCLIC_INPUT duk_str_cyclic_input

extern const char *duk_str_fmt_ptr;
extern const char *duk_str_invalid_json;
extern const char *duk_str_invalid_number;
extern const char *duk_str_jsondec_reclimit;
extern const char *duk_str_jsonenc_reclimit;
extern const char *duk_str_cyclic_input;

#define DUK_STR_PROXY_REVOKED duk_str_proxy_revoked
#define DUK_STR_OBJECT_RESIZE_FAILED duk_str_object_resize_failed
#define DUK_STR_INVALID_BASE duk_str_invalid_base
#define DUK_STR_STRICT_CALLER_READ duk_str_strict_caller_read
#define DUK_STR_PROXY_REJECTED duk_str_proxy_rejected
#define DUK_STR_INVALID_ARRAY_LENGTH duk_str_invalid_array_length
#define DUK_STR_ARRAY_LENGTH_WRITE_FAILED duk_str_array_length_write_failed
#define DUK_STR_ARRAY_LENGTH_NOT_WRITABLE duk_str_array_length_not_writable
#define DUK_STR_SETTER_UNDEFINED duk_str_setter_undefined
#define DUK_STR_REDEFINE_VIRT_PROP duk_str_redefine_virt_prop
#define DUK_STR_INVALID_DESCRIPTOR duk_str_invalid_descriptor
#define DUK_STR_PROPERTY_IS_VIRTUAL duk_str_property_is_virtual

extern const char *duk_str_proxy_revoked;
extern const char *duk_str_object_resize_failed;
extern const char *duk_str_invalid_base;
extern const char *duk_str_strict_caller_read;
extern const char *duk_str_proxy_rejected;
extern const char *duk_str_invalid_array_length;
extern const char *duk_str_array_length_write_failed;
extern const char *duk_str_array_length_not_writable;
extern const char *duk_str_setter_undefined;
extern const char *duk_str_redefine_virt_prop;
extern const char *duk_str_invalid_descriptor;
extern const char *duk_str_property_is_virtual;

#define DUK_STR_PARSE_ERROR duk_str_parse_error
#define DUK_STR_DUPLICATE_LABEL duk_str_duplicate_label
#define DUK_STR_INVALID_LABEL duk_str_invalid_label
#define DUK_STR_INVALID_ARRAY_LITERAL duk_str_invalid_array_literal
#define DUK_STR_INVALID_OBJECT_LITERAL duk_str_invalid_object_literal
#define DUK_STR_INVALID_VAR_DECLARATION duk_str_invalid_var_declaration
#define DUK_STR_CANNOT_DELETE_IDENTIFIER duk_str_cannot_delete_identifier
#define DUK_STR_INVALID_EXPRESSION duk_str_invalid_expression
#define DUK_STR_INVALID_LVALUE duk_str_invalid_lvalue
#define DUK_STR_EXPECTED_IDENTIFIER duk_str_expected_identifier
#define DUK_STR_EMPTY_EXPR_NOT_ALLOWED duk_str_empty_expr_not_allowed
#define DUK_STR_INVALID_FOR duk_str_invalid_for
#define DUK_STR_INVALID_SWITCH duk_str_invalid_switch
#define DUK_STR_INVALID_BREAK_CONT_LABEL duk_str_invalid_break_cont_label
#define DUK_STR_INVALID_RETURN duk_str_invalid_return
#define DUK_STR_INVALID_TRY duk_str_invalid_try
#define DUK_STR_INVALID_THROW duk_str_invalid_throw
#define DUK_STR_WITH_IN_STRICT_MODE duk_str_with_in_strict_mode
#define DUK_STR_FUNC_STMT_NOT_ALLOWED duk_str_func_stmt_not_allowed
#define DUK_STR_UNTERMINATED_STMT duk_str_unterminated_stmt
#define DUK_STR_INVALID_ARG_NAME duk_str_invalid_arg_name
#define DUK_STR_INVALID_FUNC_NAME duk_str_invalid_func_name
#define DUK_STR_INVALID_GETSET_NAME duk_str_invalid_getset_name
#define DUK_STR_FUNC_NAME_REQUIRED duk_str_func_name_required

extern const char *duk_str_parse_error;
extern const char *duk_str_duplicate_label;
extern const char *duk_str_invalid_label;
extern const char *duk_str_invalid_array_literal;
extern const char *duk_str_invalid_object_literal;
extern const char *duk_str_invalid_var_declaration;
extern const char *duk_str_cannot_delete_identifier;
extern const char *duk_str_invalid_expression;
extern const char *duk_str_invalid_lvalue;
extern const char *duk_str_expected_identifier;
extern const char *duk_str_empty_expr_not_allowed;
extern const char *duk_str_invalid_for;
extern const char *duk_str_invalid_switch;
extern const char *duk_str_invalid_break_cont_label;
extern const char *duk_str_invalid_return;
extern const char *duk_str_invalid_try;
extern const char *duk_str_invalid_throw;
extern const char *duk_str_with_in_strict_mode;
extern const char *duk_str_func_stmt_not_allowed;
extern const char *duk_str_unterminated_stmt;
extern const char *duk_str_invalid_arg_name;
extern const char *duk_str_invalid_func_name;
extern const char *duk_str_invalid_getset_name;
extern const char *duk_str_func_name_required;

#define DUK_STR_INTERNAL_ERROR_EXEC_LONGJMP duk_str_internal_error_exec_longjmp

extern const char *duk_str_internal_error_exec_longjmp;

#define DUK_STR_INVALID_QUANTIFIER_NO_ATOM duk_str_invalid_quantifier_no_atom
#define DUK_STR_INVALID_QUANTIFIER_VALUES duk_str_invalid_quantifier_values
#define DUK_STR_QUANTIFIER_TOO_MANY_COPIES duk_str_quantifier_too_many_copies
#define DUK_STR_UNEXPECTED_CLOSING_PAREN duk_str_unexpected_closing_paren
#define DUK_STR_UNEXPECTED_END_OF_PATTERN duk_str_unexpected_end_of_pattern
#define DUK_STR_UNEXPECTED_REGEXP_TOKEN duk_str_unexpected_regexp_token
#define DUK_STR_INVALID_REGEXP_FLAGS duk_str_invalid_regexp_flags
#define DUK_STR_INVALID_BACKREFS duk_str_invalid_backrefs
#define DUK_STR_REGEXP_BACKTRACK_FAILED duk_str_regexp_backtrack_failed
#define DUK_STR_REGEXP_ADVANCE_FAILED duk_str_regexp_advance_failed
#define DUK_STR_REGEXP_INTERNAL_ERROR duk_str_regexp_internal_error

extern const char *duk_str_invalid_quantifier_no_atom;
extern const char *duk_str_invalid_quantifier_values;
extern const char *duk_str_quantifier_too_many_copies;
extern const char *duk_str_unexpected_closing_paren;
extern const char *duk_str_unexpected_end_of_pattern;
extern const char *duk_str_unexpected_regexp_token;
extern const char *duk_str_invalid_regexp_flags;
extern const char *duk_str_invalid_backrefs;
extern const char *duk_str_regexp_backtrack_failed;
extern const char *duk_str_regexp_advance_failed;
extern const char *duk_str_regexp_internal_error;

#define DUK_STR_VALSTACK_LIMIT duk_str_valstack_limit
#define DUK_STR_OBJECT_PROPERTY_LIMIT duk_str_object_property_limit
#define DUK_STR_PROTOTYPE_CHAIN_LIMIT duk_str_prototype_chain_limit
#define DUK_STR_BOUND_CHAIN_LIMIT duk_str_bound_chain_limit
#define DUK_STR_C_CALLSTACK_LIMIT duk_str_c_callstack_limit
#define DUK_STR_COMPILER_RECURSION_LIMIT duk_str_compiler_recursion_limit
#define DUK_STR_BYTECODE_LIMIT duk_str_bytecode_limit
#define DUK_STR_REG_LIMIT duk_str_reg_limit
#define DUK_STR_TEMP_LIMIT duk_str_temp_limit
#define DUK_STR_CONST_LIMIT duk_str_const_limit
#define DUK_STR_FUNC_LIMIT duk_str_func_limit
#define DUK_STR_REGEXP_COMPILER_RECURSION_LIMIT duk_str_regexp_compiler_recursion_limit
#define DUK_STR_REGEXP_EXECUTOR_RECURSION_LIMIT duk_str_regexp_executor_recursion_limit
#define DUK_STR_REGEXP_EXECUTOR_STEP_LIMIT duk_str_regexp_executor_step_limit

extern const char *duk_str_valstack_limit;
extern const char *duk_str_object_property_limit;
extern const char *duk_str_prototype_chain_limit;
extern const char *duk_str_bound_chain_limit;
extern const char *duk_str_c_callstack_limit;
extern const char *duk_str_compiler_recursion_limit;
extern const char *duk_str_bytecode_limit;
extern const char *duk_str_reg_limit;
extern const char *duk_str_temp_limit;
extern const char *duk_str_const_limit;
extern const char *duk_str_func_limit;
extern const char *duk_str_regexp_compiler_recursion_limit;
extern const char *duk_str_regexp_executor_recursion_limit;
extern const char *duk_str_regexp_executor_step_limit;

#endif  /* DUK_ERRMSG_H_INCLUDED */
