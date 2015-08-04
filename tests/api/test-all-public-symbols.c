/*
 *  Dummy testcase containing all Duktape public API functions.  The functions
 *  are not actually called; this testcase can be used to ensure Duktape symbols
 *  are correctly exposed in a library (static or DLL) build.
 *
 *  Technically this file must contain all actually exposed symbols, which is
 *  different from the set of public API calls which can be macros.  Still, it
 *  should be enough to include all API calls, as the macro expansions should
 *  cover the actual link symbols.
 */

/*===
*** test_func (duk_safe_call)
dummy - return here
==> rc=0, result='undefined'
===*/

static duk_ret_t test_func(duk_context *ctx) {
	if (ctx) {
		printf("dummy - return here\n"); fflush(stdout);
		return 0;
	}

	/* Up-to-date for Duktape 0.12.0, alphabetical order:
	 * $ cd website/api; ls *.txt
	 */

	(void) duk_alloc_raw(ctx, 0);
	(void) duk_alloc(ctx, 0);
	(void) duk_base64_decode(ctx, 0);
	(void) duk_base64_encode(ctx, 0);
	(void) duk_call_method(ctx, 0);
	(void) duk_call_prop(ctx, 0, 0);
	(void) duk_call(ctx, 0);
	(void) duk_char_code_at(ctx, 0, 0);
	(void) duk_check_stack_top(ctx, 0);
	(void) duk_check_stack(ctx, 0);
	(void) duk_check_type_mask(ctx, 0, 0);
	(void) duk_check_type(ctx, 0, 0);
	(void) duk_compact(ctx, 0);
	(void) duk_compile_file(ctx, 0, "dummy");
	(void) duk_compile_lstring_filename(ctx, 0, "dummy", 0);
	(void) duk_compile_lstring(ctx, 0, "dummy", 0);
	(void) duk_compile_string_filename(ctx, 0, "dummy");
	(void) duk_compile_string(ctx, 0, "dummy");
	(void) duk_compile(ctx, 0);
	(void) duk_concat(ctx, 0);
	(void) duk_copy(ctx, 0, 0);
	(void) duk_create_heap_default();
	(void) duk_create_heap(NULL, NULL, NULL, NULL, NULL);
	(void) duk_decode_string(ctx, 0, NULL, NULL);
	(void) duk_del_prop_index(ctx, 0, 0);
	(void) duk_del_prop_string(ctx, 0, "dummy");
	(void) duk_del_prop(ctx, 0);
	(void) duk_del_var(ctx);
	(void) duk_destroy_heap(ctx);
	(void) duk_dump_context_stderr(ctx);
	(void) duk_dump_context_stdout(ctx);
	(void) duk_dup_top(ctx);
	(void) duk_dup(ctx, 0);
	(void) duk_enum(ctx, 0, 0);
	(void) duk_equals(ctx, 0, 0);
	duk_error(ctx, 0, "dummy");  /* (void) cast won't work without variadic macros */
	(void) duk_eval_file_noresult(ctx, "dummy");
	(void) duk_eval_file(ctx, "dummy");
	(void) duk_eval_lstring_noresult(ctx, "dummy", 0);
	(void) duk_eval_lstring(ctx, "dummy", 0);
	(void) duk_eval_noresult(ctx);
	(void) duk_eval_string_noresult(ctx, "dummy");
	(void) duk_eval_string(ctx, "dummy");
	(void) duk_eval(ctx);
	(void) duk_fatal(ctx, 0, "dummy");
	(void) duk_free_raw(ctx, NULL);
	(void) duk_free(ctx, NULL);
	(void) duk_gc(ctx, 0);
	(void) duk_get_boolean(ctx, 0);
	(void) duk_get_buffer(ctx, 0, NULL);
	(void) duk_get_c_function(ctx, 0);
	(void) duk_get_context(ctx, 0);
	(void) duk_get_current_magic(ctx);
	(void) duk_get_finalizer(ctx, 0);
	(void) duk_get_global_string(ctx, 0);
	(void) duk_get_int(ctx, 0);
	(void) duk_get_length(ctx, 0);
	(void) duk_get_lstring(ctx, 0, NULL);
	(void) duk_get_magic(ctx, 0);
	(void) duk_get_memory_functions(ctx, NULL);
	(void) duk_get_number(ctx, 0);
	(void) duk_get_pointer(ctx, 0);
	(void) duk_get_prop_index(ctx, 0, 0);
	(void) duk_get_prop_string(ctx, 0, "dummy");
	(void) duk_get_prop(ctx, 0);
	(void) duk_get_prototype(ctx, 0);
	(void) duk_get_string(ctx, 0);
	(void) duk_get_top_index(ctx);
	(void) duk_get_top(ctx);
	(void) duk_get_type_mask(ctx, 0);
	(void) duk_get_type(ctx, 0);
	(void) duk_get_uint(ctx, 0);
	(void) duk_get_var(ctx);
	(void) duk_has_prop_index(ctx, 0, 0);
	(void) duk_has_prop_string(ctx, 0, "dummy");
	(void) duk_has_prop(ctx, 0);
	(void) duk_has_var(ctx);
	(void) duk_hex_decode(ctx, 0);
	(void) duk_hex_encode(ctx, 0);
	(void) duk_insert(ctx, 0);
	(void) duk_is_array(ctx, 0);
	(void) duk_is_boolean(ctx, 0);
	(void) duk_is_bound_function(ctx, 0);
	(void) duk_is_buffer(ctx, 0);
	(void) duk_is_callable(ctx, 0);
	(void) duk_is_c_function(ctx, 0);
	(void) duk_is_constructor_call(ctx);
	(void) duk_is_dynamic_buffer(ctx, 0);
	(void) duk_is_ecmascript_function(ctx, 0);
	(void) duk_is_fixed_buffer(ctx, 0);
	(void) duk_is_function(ctx, 0);
	(void) duk_is_nan(ctx, 0);
	(void) duk_is_null_or_undefined(ctx, 0);
	(void) duk_is_null(ctx, 0);
	(void) duk_is_number(ctx, 0);
	(void) duk_is_object_coercible(ctx, 0);
	(void) duk_is_object(ctx, 0);
	(void) duk_is_pointer(ctx, 0);
	(void) duk_is_primitive(ctx, 0);
	(void) duk_is_strict_call(ctx);
	(void) duk_is_string(ctx, 0);
	(void) duk_is_thread(ctx, 0);
	(void) duk_is_undefined(ctx, 0);
	(void) duk_is_valid_index(ctx, 0);
	(void) duk_join(ctx, 0);
	(void) duk_json_decode(ctx, 0);
	(void) duk_json_encode(ctx, 0);
	(void) duk_map_string(ctx, 0, NULL, NULL);
	(void) duk_new(ctx, 0);
	(void) duk_next(ctx, 0, 0);
	(void) duk_normalize_index(ctx, 0);
	(void) duk_pcall_method(ctx, 0);
	(void) duk_pcall_prop(ctx, 0, 0);
	(void) duk_pcall(ctx, 0);
	(void) duk_pcompile_file(ctx, 0, "dummy");
	(void) duk_pcompile_lstring_filename(ctx, 0, "dummy", 0);
	(void) duk_pcompile_lstring(ctx, 0, "dummy", 0);
	(void) duk_pcompile_string_filename(ctx, 0, "dummy");
	(void) duk_pcompile_string(ctx, 0, "dummy");
	(void) duk_pcompile(ctx, 0);
	(void) duk_peval_file_noresult(ctx, "dummy");
	(void) duk_peval_file(ctx, "dummy");
	(void) duk_peval_lstring_noresult(ctx, "dummy", 0);
	(void) duk_peval_lstring(ctx, "dummy", 0);
	(void) duk_peval_noresult(ctx);
	(void) duk_peval_string_noresult(ctx, "dummy");
	(void) duk_peval_string(ctx, "dummy");
	(void) duk_peval(ctx);
	(void) duk_pop_2(ctx);
	(void) duk_pop_3(ctx);
	(void) duk_pop_n(ctx, 0);
	(void) duk_pop(ctx);
	(void) duk_push_array(ctx);
	(void) duk_push_boolean(ctx, 0);
	(void) duk_push_buffer(ctx, 0, 0);
	(void) duk_push_c_function(ctx, NULL, 0);
	(void) duk_push_context_dump(ctx);
	(void) duk_push_current_function(ctx);
	(void) duk_push_current_thread(ctx);
	(void) duk_push_dynamic_buffer(ctx, 0);
	(void) duk_push_error_object(ctx, 0, "dummy");
	(void) duk_push_false(ctx);
	(void) duk_push_fixed_buffer(ctx, 0);
	(void) duk_push_global_object(ctx);
	(void) duk_push_global_stash(ctx);
	(void) duk_push_heap_stash(ctx);
	(void) duk_push_int(ctx, 0);
	(void) duk_push_lstring(ctx, "dummy", 0);
	(void) duk_push_nan(ctx);
	(void) duk_push_null(ctx);
	(void) duk_push_number(ctx, 0.0);
	(void) duk_push_object(ctx);
	(void) duk_push_pointer(ctx, NULL);
	(void) duk_push_sprintf(ctx, "dummy");
	(void) duk_push_string_file(ctx, "dummy");
	(void) duk_push_string(ctx, "dummy");
	(void) duk_push_this(ctx);
	(void) duk_push_thread_new_globalenv(ctx);
	(void) duk_push_thread_stash(ctx, NULL);
	(void) duk_push_thread(ctx);
	(void) duk_push_true(ctx);
	(void) duk_push_uint(ctx, 0);
	(void) duk_push_undefined(ctx);
	(void) duk_push_vsprintf(ctx, "dummy", NULL);
	(void) duk_put_function_list(ctx, 0, NULL);
	(void) duk_put_number_list(ctx, 0, NULL);
	(void) duk_put_prop_index(ctx, 0, 0);
	(void) duk_put_prop_string(ctx, 0, "dummy");
	(void) duk_put_prop(ctx, 0);
	(void) duk_put_var(ctx);
	(void) duk_realloc_raw(ctx, NULL, 0);
	(void) duk_realloc(ctx, NULL, 0);
	(void) duk_remove(ctx, 0);
	(void) duk_replace(ctx, 0);
	(void) duk_require_boolean(ctx, 0);
	(void) duk_require_buffer(ctx, 0, NULL);
	(void) duk_require_c_function(ctx, 0);
	(void) duk_require_context(ctx, 0);
	(void) duk_require_int(ctx, 0);
	(void) duk_require_lstring(ctx, 0, NULL);
	(void) duk_require_normalize_index(ctx, 0);
	(void) duk_require_null(ctx, 0);
	(void) duk_require_number(ctx, 0);
	(void) duk_require_object_coercible(ctx, 0);
	(void) duk_require_pointer(ctx, 0);
	(void) duk_require_stack_top(ctx, 0);
	(void) duk_require_stack(ctx, 0);
	(void) duk_require_string(ctx, 0);
	(void) duk_require_top_index(ctx);
	(void) duk_require_type_mask(ctx, 0, 0);
	(void) duk_require_uint(ctx, 0);
	(void) duk_require_undefined(ctx, 0);
	(void) duk_require_valid_index(ctx, 0);
	(void) duk_resize_buffer(ctx, 0, 0);
	(void) duk_safe_call(ctx, NULL, 0, 0);
	(void) duk_safe_to_lstring(ctx, 0, NULL);
	(void) duk_safe_to_string(ctx, 0);
	(void) duk_set_finalizer(ctx, 0);
	(void) duk_set_global_object(ctx);
	(void) duk_set_magic(ctx, 0, 0);
	(void) duk_set_prototype(ctx, 0);
	(void) duk_set_top(ctx, 0);
	(void) duk_strict_equals(ctx, 0, 0);
	(void) duk_substring(ctx, 0, 0, 0);
	(void) duk_swap_top(ctx, 0);
	(void) duk_swap(ctx, 0, 0);
	(void) duk_throw(ctx);
	(void) duk_to_boolean(ctx, 0);
	(void) duk_to_buffer(ctx, 0, NULL);
	(void) duk_to_defaultvalue(ctx, 0, 0);
	(void) duk_to_dynamic_buffer(ctx, 0, NULL);
	(void) duk_to_fixed_buffer(ctx, 0, NULL);
	(void) duk_to_int32(ctx, 0);
	(void) duk_to_int(ctx, 0);
	(void) duk_to_lstring(ctx, 0, NULL);
	(void) duk_to_null(ctx, 0);
	(void) duk_to_number(ctx, 0);
	(void) duk_to_object(ctx, 0);
	(void) duk_to_pointer(ctx, 0);
	(void) duk_to_primitive(ctx, 0, 0);
	(void) duk_to_string(ctx, 0);
	(void) duk_to_uint16(ctx, 0);
	(void) duk_to_uint32(ctx, 0);
	(void) duk_to_uint(ctx, 0);
	(void) duk_to_undefined(ctx, 0);
	(void) duk_trim(ctx, 0);
	(void) duk_xcopy_top(ctx, NULL, 0);
	(void) duk_xmove_top(ctx, NULL, 0);

	printf("never here\n"); fflush(stdout);
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_func);
}
