/*===
no running function
type=1
duk_is_object: 0
duk_is_thread: 0
basic case
type: 6
duk_is_thread: 1
duk_get_context matches ctx: 1
final top: 1
rc=0, result='undefined'
===*/

static duk_ret_t my_func(duk_context *ctx) {
	duk_context *ctx2;

	duk_push_current_thread(ctx);
	printf("type: %d\n", (int) duk_get_type(ctx, -1));
	printf("duk_is_thread: %d\n", (int) duk_is_thread(ctx, -1));

	ctx2 = duk_get_context(ctx, -1);
#if 0
	printf("ctx: %p\n", ctx);
	printf("ctx2: %p\n", ctx2);
#endif
	printf("duk_get_context matches ctx: %d\n", (ctx == ctx2 ? 1 : 0));

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	duk_ret_t rc;

	/* first test what happens when there is no running function */

	printf("no running function\n");
	duk_push_current_thread(ctx);
	printf("type=%d\n", (int) duk_get_type(ctx, -1));
	printf("duk_is_object: %d\n", (int) duk_is_object(ctx, -1));
	printf("duk_is_thread: %d\n", (int) duk_is_thread(ctx, -1));
	duk_pop(ctx);

	/* then test the basic case */

	printf("basic case\n");
	duk_push_c_function(ctx, my_func, 1 /*nargs*/);
	duk_push_int(ctx, 123);
	rc = duk_pcall(ctx, 1);
	printf("rc=%d, result='%s'\n", (int) rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}
