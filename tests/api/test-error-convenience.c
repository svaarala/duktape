/*
 *  Convenience error throwers.
 */

/*===
*** test_generic_error (duk_safe_call)
==> rc=1, result='Error: my error: 123'
*** test_eval_error (duk_safe_call)
==> rc=1, result='EvalError: my error: 123'
*** test_range_error (duk_safe_call)
==> rc=1, result='RangeError: my error: 123'
*** test_reference_error (duk_safe_call)
==> rc=1, result='ReferenceError: my error: 123'
*** test_syntax_error (duk_safe_call)
==> rc=1, result='SyntaxError: my error: 123'
*** test_type_error (duk_safe_call)
==> rc=1, result='TypeError: my error: 123'
*** test_uri_error (duk_safe_call)
==> rc=1, result='URIError: my error: 123'
*** test_generic_error_va (duk_safe_call)
==> rc=1, result='Error: my error: 123'
*** test_eval_error_va (duk_safe_call)
==> rc=1, result='EvalError: my error: 123'
*** test_range_error_va (duk_safe_call)
==> rc=1, result='RangeError: my error: 123'
*** test_reference_error_va (duk_safe_call)
==> rc=1, result='ReferenceError: my error: 123'
*** test_syntax_error_va (duk_safe_call)
==> rc=1, result='SyntaxError: my error: 123'
*** test_type_error_va (duk_safe_call)
==> rc=1, result='TypeError: my error: 123'
*** test_uri_error_va (duk_safe_call)
==> rc=1, result='URIError: my error: 123'
===*/

/* Non-vararg calls. */

static duk_ret_t test_generic_error(duk_context *ctx, void *udata) {
	(void) udata;
	return duk_generic_error(ctx, "my error: %d", 123);
}

static duk_ret_t test_eval_error(duk_context *ctx, void *udata) {
	(void) udata;
	return duk_eval_error(ctx, "my error: %d", 123);
}

static duk_ret_t test_range_error(duk_context *ctx, void *udata) {
	(void) udata;
	return duk_range_error(ctx, "my error: %d", 123);
}

static duk_ret_t test_reference_error(duk_context *ctx, void *udata) {
	(void) udata;
	return duk_reference_error(ctx, "my error: %d", 123);
}

static duk_ret_t test_syntax_error(duk_context *ctx, void *udata) {
	(void) udata;
	return duk_syntax_error(ctx, "my error: %d", 123);
}

static duk_ret_t test_type_error(duk_context *ctx, void *udata) {
	(void) udata;
	return duk_type_error(ctx, "my error: %d", 123);
}

static duk_ret_t test_uri_error(duk_context *ctx, void *udata) {
	(void) udata;
	return duk_uri_error(ctx, "my error: %d", 123);
}

/* Vararg calls. */

static duk_ret_t throw_generic_va(duk_context *ctx, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	return duk_generic_error_va(ctx, fmt, ap);
	va_end(ap);
}
static duk_ret_t test_generic_error_va(duk_context *ctx, void *udata) {
	(void) udata;
	return throw_generic_va(ctx, "my error: %d", 123);
}

static duk_ret_t throw_eval_va(duk_context *ctx, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	return duk_eval_error_va(ctx, fmt, ap);
	va_end(ap);
}
static duk_ret_t test_eval_error_va(duk_context *ctx, void *udata) {
	(void) udata;
	return throw_eval_va(ctx, "my error: %d", 123);
}

static duk_ret_t throw_range_va(duk_context *ctx, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	return duk_range_error_va(ctx, fmt, ap);
	va_end(ap);
}
static duk_ret_t test_range_error_va(duk_context *ctx, void *udata) {
	(void) udata;
	return throw_range_va(ctx, "my error: %d", 123);
}

static duk_ret_t throw_reference_va(duk_context *ctx, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	return duk_reference_error_va(ctx, fmt, ap);
	va_end(ap);
}
static duk_ret_t test_reference_error_va(duk_context *ctx, void *udata) {
	(void) udata;
	return throw_reference_va(ctx, "my error: %d", 123);
}

static duk_ret_t throw_syntax_va(duk_context *ctx, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	return duk_syntax_error_va(ctx, fmt, ap);
	va_end(ap);
}
static duk_ret_t test_syntax_error_va(duk_context *ctx, void *udata) {
	(void) udata;
	return throw_syntax_va(ctx, "my error: %d", 123);
}

static duk_ret_t throw_type_va(duk_context *ctx, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	return duk_type_error_va(ctx, fmt, ap);
	va_end(ap);
}
static duk_ret_t test_type_error_va(duk_context *ctx, void *udata) {
	(void) udata;
	return throw_type_va(ctx, "my error: %d", 123);
}

static duk_ret_t throw_uri_va(duk_context *ctx, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	return duk_uri_error_va(ctx, fmt, ap);
	va_end(ap);
}
static duk_ret_t test_uri_error_va(duk_context *ctx, void *udata) {
	(void) udata;
	return throw_uri_va(ctx, "my error: %d", 123);
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_generic_error);
	TEST_SAFE_CALL(test_eval_error);
	TEST_SAFE_CALL(test_range_error);
	TEST_SAFE_CALL(test_reference_error);
	TEST_SAFE_CALL(test_syntax_error);
	TEST_SAFE_CALL(test_type_error);
	TEST_SAFE_CALL(test_uri_error);

	TEST_SAFE_CALL(test_generic_error_va);
	TEST_SAFE_CALL(test_eval_error_va);
	TEST_SAFE_CALL(test_range_error_va);
	TEST_SAFE_CALL(test_reference_error_va);
	TEST_SAFE_CALL(test_syntax_error_va);
	TEST_SAFE_CALL(test_type_error_va);
	TEST_SAFE_CALL(test_uri_error_va);
}
