/*
 *  Shared error message strings
 *
 *  To minimize code footprint, try to share error messages inside Duktape
 *  code.
 */

#include "duk_internal.h"

const char *duk_str_invalid_count = "invalid count";
const char *duk_str_invalid_call_args = "invalid call args";
const char *duk_str_not_constructable = "not constructable";

const char *duk_str_invalid_index = "invalid index";
const char *duk_str_valstack_limit = "valstack limit";
const char *duk_str_push_beyond_alloc_stack = "attempt to push beyond currently allocated stack";
const char *duk_str_src_stack_not_enough = "source stack does not contain enough elements";
const char *duk_str_not_undefined = "not undefined";
const char *duk_str_not_null = "not null";
const char *duk_str_not_boolean = "not boolean";
const char *duk_str_not_number = "not number";
const char *duk_str_not_string = "not string";
const char *duk_str_not_pointer = "not pointer";
const char *duk_str_not_buffer = "not buffer";
const char *duk_str_unexpected_type = "unexpected type";
const char *duk_str_expected_thread = "expected thread";
const char *duk_str_expected_compiledfunction = "expected compiledfunction";
const char *duk_str_expected_nativefunction = "expected nativefunction";
const char *duk_str_expected_c_function = "expected c function";

const char *duk_str_fmt_ptr = "%p";
const char *duk_str_invalid_json = "invalid json";
const char *duk_str_invalid_number = "invalid number";
const char *duk_str_jsondec_reclimit = "json decode recursion limit";
const char *duk_str_jsonenc_reclimit = "json encode recursion limit";
const char *duk_str_cyclic_input = "cyclic input";
