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

const char *duk_str_fmt_ptr = "%p";
const char *duk_str_invalid_json = "invalid json";
const char *duk_str_invalid_number = "invalid number";
const char *duk_str_jsondec_reclimit = "json decode recursion limit";
const char *duk_str_jsonenc_reclimit = "json encode recursion limit";
const char *duk_str_cyclic_input = "cyclic input";
