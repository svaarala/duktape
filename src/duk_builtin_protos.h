/*
 *  Prototypes for all built-in functions.
 */

#ifndef DUK_BUILTIN_PROTOS_H_INCLUDED
#define DUK_BUILTIN_PROTOS_H_INCLUDED

int duk_builtin_array_constructor(duk_context *ctx);
int duk_builtin_array_constructor_is_array(duk_context *ctx);
int duk_builtin_array_prototype_to_string(duk_context *ctx);
int duk_builtin_array_prototype_to_locale_string(duk_context *ctx);
int duk_builtin_array_prototype_concat(duk_context *ctx);
int duk_builtin_array_prototype_join(duk_context *ctx);
int duk_builtin_array_prototype_pop(duk_context *ctx);
int duk_builtin_array_prototype_push(duk_context *ctx);
int duk_builtin_array_prototype_reverse(duk_context *ctx);
int duk_builtin_array_prototype_shift(duk_context *ctx);
int duk_builtin_array_prototype_slice(duk_context *ctx);
int duk_builtin_array_prototype_sort(duk_context *ctx);
int duk_builtin_array_prototype_splice(duk_context *ctx);
int duk_builtin_array_prototype_unshift(duk_context *ctx);
int duk_builtin_array_prototype_index_of(duk_context *ctx);
int duk_builtin_array_prototype_last_index_of(duk_context *ctx);
int duk_builtin_array_prototype_every(duk_context *ctx);
int duk_builtin_array_prototype_some(duk_context *ctx);
int duk_builtin_array_prototype_for_each(duk_context *ctx);
int duk_builtin_array_prototype_map(duk_context *ctx);
int duk_builtin_array_prototype_filter(duk_context *ctx);
int duk_builtin_array_prototype_reduce(duk_context *ctx);
int duk_builtin_array_prototype_reduce_right(duk_context *ctx);

int duk_builtin_boolean_constructor(duk_context *ctx);
int duk_builtin_boolean_prototype_to_string(duk_context *ctx);
int duk_builtin_boolean_prototype_value_of(duk_context *ctx);

int duk_builtin_buffer_constructor(duk_context *ctx);
int duk_builtin_buffer_prototype_to_string(duk_context *ctx);
int duk_builtin_buffer_prototype_value_of(duk_context *ctx);

int duk_builtin_date_constructor(duk_context *ctx);
int duk_builtin_date_constructor_parse(duk_context *ctx);
int duk_builtin_date_constructor_utc(duk_context *ctx);
int duk_builtin_date_constructor_now(duk_context *ctx);
int duk_builtin_date_prototype_to_string(duk_context *ctx);
int duk_builtin_date_prototype_to_date_string(duk_context *ctx);
int duk_builtin_date_prototype_to_time_string(duk_context *ctx);
int duk_builtin_date_prototype_to_locale_string(duk_context *ctx);
int duk_builtin_date_prototype_to_locale_date_string(duk_context *ctx);
int duk_builtin_date_prototype_to_locale_time_string(duk_context *ctx);
int duk_builtin_date_prototype_value_of(duk_context *ctx);
int duk_builtin_date_prototype_get_time(duk_context *ctx);
int duk_builtin_date_prototype_get_timezone_offset(duk_context *ctx);
int duk_builtin_date_prototype_get_shared(duk_context *ctx);
int duk_builtin_date_prototype_set_time(duk_context *ctx);
int duk_builtin_date_prototype_set_shared(duk_context *ctx);
int duk_builtin_date_prototype_to_utc_string(duk_context *ctx);
int duk_builtin_date_prototype_to_iso_string(duk_context *ctx);
int duk_builtin_date_prototype_to_json(duk_context *ctx);

int duk_builtin_duk_object_addr(duk_context *ctx);
int duk_builtin_duk_object_refc(duk_context *ctx);
int duk_builtin_duk_object_gc(duk_context *ctx);
int duk_builtin_duk_object_get_finalizer(duk_context *ctx);
int duk_builtin_duk_object_set_finalizer(duk_context *ctx);
int duk_builtin_duk_object_time(duk_context *ctx);
int duk_builtin_duk_object_enc(duk_context *ctx);
int duk_builtin_duk_object_dec(duk_context *ctx);
int duk_builtin_duk_object_sleep(duk_context *ctx);

int duk_builtin_error_constructor_shared(duk_context *ctx);
int duk_builtin_error_prototype_to_string(duk_context *ctx);
int duk_builtin_error_prototype_stack_getter(duk_context *ctx);
int duk_builtin_error_prototype_filename_getter(duk_context *ctx);
int duk_builtin_error_prototype_linenumber_getter(duk_context *ctx);
int duk_builtin_error_prototype_stack_getter(duk_context *ctx);
int duk_builtin_error_prototype_nop_setter(duk_context *ctx);

int duk_builtin_function_constructor(duk_context *ctx);
int duk_builtin_function_prototype(duk_context *ctx);
int duk_builtin_function_prototype_to_string(duk_context *ctx);
int duk_builtin_function_prototype_apply(duk_context *ctx);
int duk_builtin_function_prototype_call(duk_context *ctx);
int duk_builtin_function_prototype_bind(duk_context *ctx);

int duk_builtin_global_object_eval(duk_context *ctx);
int duk_builtin_global_object_parse_int(duk_context *ctx);
int duk_builtin_global_object_parse_float(duk_context *ctx);
int duk_builtin_global_object_is_nan(duk_context *ctx);
int duk_builtin_global_object_is_finite(duk_context *ctx);
int duk_builtin_global_object_decode_uri(duk_context *ctx);
int duk_builtin_global_object_decode_uri_component(duk_context *ctx);
int duk_builtin_global_object_encode_uri(duk_context *ctx);
int duk_builtin_global_object_encode_uri_component(duk_context *ctx);
#if 1  /* FIXME: Section B */
int duk_builtin_global_object_escape(duk_context *ctx);
int duk_builtin_global_object_unescape(duk_context *ctx);
#endif
#if 1  /* FIXME: browser-like */
int duk_builtin_global_object_print(duk_context *ctx);
int duk_builtin_global_object_alert(duk_context *ctx);
#endif

void duk_builtin_json_parse_helper(duk_context *ctx,
                                   int idx_value,
                                   int idx_reviver,
                                   int flags);
void duk_builtin_json_stringify_helper(duk_context *ctx,
                                       int idx_value,
                                       int idx_replacer,
                                       int idx_space,
                                       int flags);
int duk_builtin_json_object_parse(duk_context *ctx);
int duk_builtin_json_object_stringify(duk_context *ctx);

int duk_builtin_math_object_abs(duk_context *ctx);
int duk_builtin_math_object_acos(duk_context *ctx);
int duk_builtin_math_object_asin(duk_context *ctx);
int duk_builtin_math_object_atan(duk_context *ctx);
int duk_builtin_math_object_atan2(duk_context *ctx);
int duk_builtin_math_object_ceil(duk_context *ctx);
int duk_builtin_math_object_cos(duk_context *ctx);
int duk_builtin_math_object_exp(duk_context *ctx);
int duk_builtin_math_object_floor(duk_context *ctx);
int duk_builtin_math_object_log(duk_context *ctx);
int duk_builtin_math_object_max(duk_context *ctx);
int duk_builtin_math_object_min(duk_context *ctx);
int duk_builtin_math_object_pow(duk_context *ctx);
int duk_builtin_math_object_random(duk_context *ctx);
int duk_builtin_math_object_round(duk_context *ctx);
int duk_builtin_math_object_sin(duk_context *ctx);
int duk_builtin_math_object_sqrt(duk_context *ctx);
int duk_builtin_math_object_tan(duk_context *ctx);

int duk_builtin_number_constructor(duk_context *ctx);
int duk_builtin_number_prototype_to_string(duk_context *ctx);
int duk_builtin_number_prototype_to_locale_string(duk_context *ctx);
int duk_builtin_number_prototype_value_of(duk_context *ctx);
int duk_builtin_number_prototype_to_fixed(duk_context *ctx);
int duk_builtin_number_prototype_to_exponential(duk_context *ctx);
int duk_builtin_number_prototype_to_precision(duk_context *ctx);

int duk_builtin_object_constructor(duk_context *ctx);
int duk_builtin_object_constructor_get_prototype_of(duk_context *ctx);
int duk_builtin_object_constructor_get_own_property_descriptor(duk_context *ctx);
int duk_builtin_object_constructor_get_own_property_names(duk_context *ctx);
int duk_builtin_object_constructor_create(duk_context *ctx);
int duk_builtin_object_constructor_define_property(duk_context *ctx);
int duk_builtin_object_constructor_define_properties(duk_context *ctx);
int duk_builtin_object_constructor_seal(duk_context *ctx);
int duk_builtin_object_constructor_freeze(duk_context *ctx);
int duk_builtin_object_constructor_prevent_extensions(duk_context *ctx);
int duk_builtin_object_constructor_is_sealed(duk_context *ctx);
int duk_builtin_object_constructor_is_frozen(duk_context *ctx);
int duk_builtin_object_constructor_is_extensible(duk_context *ctx);
int duk_builtin_object_constructor_keys(duk_context *ctx);
int duk_builtin_object_prototype_to_string(duk_context *ctx);
int duk_builtin_object_prototype_to_locale_string(duk_context *ctx);
int duk_builtin_object_prototype_value_of(duk_context *ctx);
int duk_builtin_object_prototype_has_own_property(duk_context *ctx);
int duk_builtin_object_prototype_is_prototype_of(duk_context *ctx);
int duk_builtin_object_prototype_property_is_enumerable(duk_context *ctx);

int duk_builtin_pointer_constructor(duk_context *ctx);
int duk_builtin_pointer_prototype_to_string(duk_context *ctx);
int duk_builtin_pointer_prototype_value_of(duk_context *ctx);

int duk_builtin_regexp_prototype_exec(duk_context *ctx);
int duk_builtin_regexp_prototype_test(duk_context *ctx);
int duk_builtin_regexp_prototype_to_string(duk_context *ctx);

int duk_builtin_string_constructor_from_char_code(duk_context *ctx);
int duk_builtin_string_prototype_to_string(duk_context *ctx);
int duk_builtin_string_prototype_value_of(duk_context *ctx);
int duk_builtin_string_prototype_char_at(duk_context *ctx);
int duk_builtin_string_prototype_char_code_at(duk_context *ctx);
int duk_builtin_string_prototype_concat(duk_context *ctx);
int duk_builtin_string_prototype_index_of(duk_context *ctx);
int duk_builtin_string_prototype_last_index_of(duk_context *ctx);
int duk_builtin_string_prototype_locale_compare(duk_context *ctx);
int duk_builtin_string_prototype_match(duk_context *ctx);
int duk_builtin_string_prototype_replace(duk_context *ctx);
int duk_builtin_string_prototype_search(duk_context *ctx);
int duk_builtin_string_prototype_slice(duk_context *ctx);
int duk_builtin_string_prototype_split(duk_context *ctx);
int duk_builtin_string_prototype_substring(duk_context *ctx);
int duk_builtin_string_prototype_to_lower_case(duk_context *ctx);
int duk_builtin_string_prototype_to_locale_lower_case(duk_context *ctx);
int duk_builtin_string_prototype_to_upper_case(duk_context *ctx);
int duk_builtin_string_prototype_to_locale_upper_case(duk_context *ctx);
int duk_builtin_string_prototype_trim(duk_context *ctx);
#if 1  /* FIXME: section B */
int duk_builtin_string_prototype_substr(duk_context *ctx);
#endif
int duk_builtin_thread_constructor(duk_context *ctx);
int duk_builtin_thread_resume(duk_context *ctx);
int duk_builtin_thread_yield(duk_context *ctx);
int duk_builtin_thread_current(duk_context *ctx);
int duk_builtin_thread_prototype_to_string(duk_context *ctx);
int duk_builtin_thread_prototype_value_of(duk_context *ctx);

int duk_builtin_type_error_thrower(duk_context *ctx);

#endif  /* DUK_BUILTIN_PROTOS_H_INCLUDED */

