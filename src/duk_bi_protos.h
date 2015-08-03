/*
 *  Prototypes for all built-in functions.
 */

#ifndef DUK_BUILTIN_PROTOS_H_INCLUDED
#define DUK_BUILTIN_PROTOS_H_INCLUDED

/* Buffer size needed for duk_bi_date_format_timeval().
 * Accurate value is 32 + 1 for NUL termination:
 *   >>> len('+123456-01-23T12:34:56.123+12:34')
 *   32
 * Include additional space to be safe.
 */
#define  DUK_BI_DATE_ISO8601_BUFSIZE  48

/* Maximum length of CommonJS module identifier to resolve.  Length includes
 * both current module ID, requested (possibly relative) module ID, and a
 * slash in between.
 */
#define  DUK_BI_COMMONJS_MODULE_ID_LIMIT  256

DUK_INTERNAL_DECL duk_ret_t duk_bi_array_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_constructor_is_array(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_prototype_to_string(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_prototype_concat(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_prototype_join_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_prototype_pop(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_prototype_push(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_prototype_reverse(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_prototype_shift(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_prototype_slice(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_prototype_sort(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_prototype_splice(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_prototype_unshift(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_prototype_indexof_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_prototype_iter_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_array_prototype_reduce_shared(duk_context *ctx);

DUK_INTERNAL_DECL duk_ret_t duk_bi_boolean_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_boolean_prototype_tostring_shared(duk_context *ctx);

/* XXX: naming is inconsistent with other builtins, "prototype" not used as
 * part of function name.
 */
DUK_INTERNAL_DECL duk_ret_t duk_bi_buffer_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_buffer_prototype_tostring_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_buffer_readfield(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_buffer_writefield(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_buffer_compare_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_buffer_slice_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_arraybuffer_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_arraybuffer_isview(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_dataview_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_typedarray_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_typedarray_set(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_nodejs_buffer_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_nodejs_buffer_concat(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_nodejs_buffer_is_encoding(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_nodejs_buffer_is_buffer(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_nodejs_buffer_byte_length(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_nodejs_buffer_tostring(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_nodejs_buffer_tojson(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_nodejs_buffer_fill(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_nodejs_buffer_copy(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_nodejs_buffer_write(duk_context *ctx);

DUK_INTERNAL_DECL duk_ret_t duk_bi_date_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_date_constructor_parse(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_date_constructor_utc(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_date_constructor_now(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_date_prototype_tostring_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_date_prototype_value_of(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_date_prototype_to_json(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_date_prototype_get_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_date_prototype_get_timezone_offset(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_date_prototype_set_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_date_prototype_set_time(duk_context *ctx);
/* Helpers exposed for internal use */
DUK_INTERNAL_DECL void duk_bi_date_timeval_to_parts(duk_double_t d, duk_int_t *parts, duk_double_t *dparts, duk_small_uint_t flags);
DUK_INTERNAL_DECL duk_double_t duk_bi_date_get_timeval_from_dparts(duk_double_t *dparts, duk_small_uint_t flags);
DUK_INTERNAL_DECL void duk_bi_date_format_timeval(duk_double_t timeval, duk_uint8_t *out_buf);
DUK_INTERNAL_DECL duk_bool_t duk_bi_date_is_leap_year(duk_int_t year);
DUK_INTERNAL_DECL duk_bool_t duk_bi_date_timeval_in_valid_range(duk_double_t x);
DUK_INTERNAL_DECL duk_bool_t duk_bi_date_year_in_valid_range(duk_double_t year);
DUK_INTERNAL_DECL duk_bool_t duk_bi_date_timeval_in_leeway_range(duk_double_t x);

DUK_INTERNAL_DECL duk_ret_t duk_bi_duktape_object_info(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_duktape_object_act(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_duktape_object_gc(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_duktape_object_fin(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_duktape_object_enc(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_duktape_object_dec(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_duktape_object_compact(duk_context *ctx);

DUK_INTERNAL_DECL duk_ret_t duk_bi_error_constructor_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_error_prototype_to_string(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_error_prototype_stack_getter(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_error_prototype_filename_getter(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_error_prototype_linenumber_getter(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_error_prototype_nop_setter(duk_context *ctx);

DUK_INTERNAL_DECL duk_ret_t duk_bi_function_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_function_prototype(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_function_prototype_to_string(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_function_prototype_apply(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_function_prototype_call(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_function_prototype_bind(duk_context *ctx);

DUK_INTERNAL_DECL duk_ret_t duk_bi_global_object_eval(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_global_object_parse_int(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_global_object_parse_float(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_global_object_is_nan(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_global_object_is_finite(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_global_object_decode_uri(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_global_object_decode_uri_component(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_global_object_encode_uri(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_global_object_encode_uri_component(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_global_object_escape(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_global_object_unescape(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_global_object_print_helper(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_global_object_require(duk_context *ctx);

DUK_INTERNAL_DECL
void duk_bi_json_parse_helper(duk_context *ctx,
                              duk_idx_t idx_value,
                              duk_idx_t idx_reviver,
                              duk_small_uint_t flags);
DUK_INTERNAL_DECL
void duk_bi_json_stringify_helper(duk_context *ctx,
                                  duk_idx_t idx_value,
                                  duk_idx_t idx_replacer,
                                  duk_idx_t idx_space,
                                  duk_small_uint_t flags);
DUK_INTERNAL_DECL duk_ret_t duk_bi_json_object_parse(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_json_object_stringify(duk_context *ctx);

DUK_INTERNAL_DECL duk_ret_t duk_bi_math_object_onearg_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_math_object_twoarg_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_math_object_max(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_math_object_min(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_math_object_random(duk_context *ctx);

DUK_INTERNAL_DECL duk_ret_t duk_bi_number_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_number_prototype_to_string(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_number_prototype_to_locale_string(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_number_prototype_value_of(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_number_prototype_to_fixed(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_number_prototype_to_exponential(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_number_prototype_to_precision(duk_context *ctx);

DUK_INTERNAL_DECL duk_ret_t duk_bi_object_getprototype_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_setprototype_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_constructor_get_own_property_descriptor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_constructor_create(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_constructor_define_property(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_constructor_define_properties(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_constructor_seal_freeze_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_constructor_prevent_extensions(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_constructor_is_sealed_frozen_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_constructor_is_extensible(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_constructor_keys_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_prototype_to_string(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_prototype_to_locale_string(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_prototype_value_of(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_prototype_has_own_property(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_prototype_is_prototype_of(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_prototype_property_is_enumerable(duk_context *ctx);

DUK_INTERNAL_DECL duk_ret_t duk_bi_pointer_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_pointer_prototype_tostring_shared(duk_context *ctx);

DUK_INTERNAL_DECL duk_ret_t duk_bi_regexp_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_regexp_prototype_exec(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_regexp_prototype_test(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_regexp_prototype_to_string(duk_context *ctx);

DUK_INTERNAL_DECL duk_ret_t duk_bi_string_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_constructor_from_char_code(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_to_string(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_char_at(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_char_code_at(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_concat(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_indexof_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_locale_compare(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_match(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_replace(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_search(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_slice(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_split(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_substring(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_caseconv_shared(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_trim(duk_context *ctx);
/* Note: present even if DUK_USE_SECTION_B undefined given because genbuiltins.py
 * will point to it.
 */
DUK_INTERNAL_DECL duk_ret_t duk_bi_string_prototype_substr(duk_context *ctx);

DUK_INTERNAL_DECL duk_ret_t duk_bi_proxy_constructor(duk_context *ctx);
#if 0  /* unimplemented now */
DUK_INTERNAL_DECL duk_ret_t duk_bi_proxy_constructor_revocable(duk_context *ctx);
#endif

DUK_INTERNAL_DECL duk_ret_t duk_bi_thread_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_thread_resume(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_thread_yield(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_thread_current(duk_context *ctx);

DUK_INTERNAL_DECL duk_ret_t duk_bi_logger_constructor(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_logger_prototype_fmt(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_logger_prototype_raw(duk_context *ctx);
DUK_INTERNAL_DECL duk_ret_t duk_bi_logger_prototype_log_shared(duk_context *ctx);

DUK_INTERNAL_DECL duk_ret_t duk_bi_type_error_thrower(duk_context *ctx);

#endif  /* DUK_BUILTIN_PROTOS_H_INCLUDED */
