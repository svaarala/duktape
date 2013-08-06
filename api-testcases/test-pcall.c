/*===
rc=0, result='21'
rc=1, result='Error: my error'
final top: 0
===*/

void test(duk_context *ctx) {
	int rc;

	/* basic success case */
	duk_eval_string(ctx, "(function (x,y) { return x+y; })");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall(ctx, 2, DUK_INVALID_INDEX);
	printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* basic error case */
	duk_eval_string(ctx, "(function (x,y) { throw new Error('my error'); })");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall(ctx, 2, DUK_INVALID_INDEX);
	printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* FIXME: error handler tests */

	printf("final top: %d\n", duk_get_top(ctx));
}
