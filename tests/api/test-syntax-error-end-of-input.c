/*===
*** test_1 (duk_safe_call)
end of input: 'SyntaxError: invalid object literal (line 1, end of input)'
before end of input: 'SyntaxError: parse error (line 2)'
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_peval_string(ctx, "var obj = {");
	printf("end of input: '%s'\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	duk_peval_string(ctx, "var obj = {\n2+2;");
	printf("before end of input: '%s'\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
