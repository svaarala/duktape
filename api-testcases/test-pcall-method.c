/*===
object 123
rc=0, result='21'
number 123
rc=0, result='21'
rc=1, result='Error: my error'
final top: 0
===*/

void test(duk_context *ctx) {
	duk_ret_t rc;

	/* basic success case, non-strict target function (this gets coerced) */
	duk_eval_string(ctx, "(function (x,y) { print(typeof this, this); return x+y; })");
	duk_push_int(ctx, 123);  /* this */
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_method(ctx, 2);
	printf("rc=%d, result='%s'\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* basic success case, strict target function (this not coerced)) */
	duk_eval_string(ctx, "(function (x,y) { 'use strict'; print(typeof this, this); return x+y; })");
	duk_push_int(ctx, 123);  /* this */
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_method(ctx, 2);
	printf("rc=%d, result='%s'\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* basic error case */
	duk_eval_string(ctx, "(function (x,y) { throw new Error('my error'); })");
	duk_push_int(ctx, 123);  /* this */
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_method(ctx, 2);
	printf("rc=%d, result='%s'\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
}
