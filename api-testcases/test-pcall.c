/*===
rc=0, result='21'
top after pop: 0
rc=1, result='Error: my error'
top after pop: 0
final top: 0
===*/

void test(duk_context *ctx) {
	duk_ret_t rc;

	/* basic success case */
	duk_eval_string(ctx, "(function (x,y) { return x+y; })");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall(ctx, 2);
	printf("rc=%d, result='%s'\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	printf("top after pop: %ld\n", (long) duk_get_top(ctx));

	/* basic error case */
	duk_eval_string(ctx, "(function (x,y) { throw new Error('my error'); })");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall(ctx, 2);
	printf("rc=%d, result='%s'\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	printf("top after pop: %ld\n", (long) duk_get_top(ctx));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
}
