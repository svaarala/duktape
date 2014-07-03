#ifndef DUK_ERRMSG_H_INCLUDED
#define DUK_ERRMSG_H_INCLUDED

extern const char *duk_str_internal_error;
extern const char *duk_str_invalid_count;
extern const char *duk_str_invalid_call_args;
extern const char *duk_str_not_constructable;
extern const char *duk_str_not_callable;
extern const char *duk_str_not_extensible;
extern const char *duk_str_not_writable;
extern const char *duk_str_not_configurable;

extern const char *duk_str_invalid_index;
extern const char *duk_str_valstack_limit;
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
extern const char *duk_str_expected_thread;
extern const char *duk_str_expected_compiledfunction;
extern const char *duk_str_expected_nativefunction;
extern const char *duk_str_expected_c_function;
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

extern const char *duk_str_fmt_ptr;
extern const char *duk_str_invalid_json;
extern const char *duk_str_invalid_number;
extern const char *duk_str_jsondec_reclimit;
extern const char *duk_str_jsonenc_reclimit;
extern const char *duk_str_cyclic_input;

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

extern const char *duk_str_object_property_limit;
extern const char *duk_str_prototype_chain_limit;
extern const char *duk_str_bound_chain_limit;
extern const char *duk_str_c_callstack_limit;

#endif  /* DUK_ERRMSG_H_INCLUDED */
