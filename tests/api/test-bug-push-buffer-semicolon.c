/*
 *  https://github.com/svaarala/duktape/issues/500
 */

/*===
still here
===*/

void test(duk_context *ctx) {
	(void) (duk_push_buffer(ctx, 123, 0), "dummy");
	printf("still here\n");
}
