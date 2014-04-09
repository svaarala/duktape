/*===
me
rc=0, result='21'
object 123
rc=0, result='21'
number 123
rc=0, result='21'
rc=1, result='Error: my error'
final top: 0
===*/

void test(duk_context *ctx) {
	int rc;

	duk_push_string(ctx, "foo");  /* dummy */

	/* basic success case: own property */
	duk_eval_string(ctx, "({ name: 'me', foo: function (x,y) { print(this.name); return x+y; } })");  /* idx 1 */
	duk_push_string(ctx, "foo");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_prop(ctx, 1, 2);
	printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);  /* res */
	duk_pop(ctx);  /* obj */

	/* use plain number as 'this', add function to Number.prototype; non-strict handler
	 * causes this to be coerced to Number.
	 */
	duk_eval_string(ctx, "Number.prototype.func_nonstrict = function (x,y) { print(typeof this, this); return x+y; };");
	duk_pop(ctx);  /* pop result */
	duk_push_int(ctx, 123);  /* obj */
	duk_push_string(ctx, "func_nonstrict");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_prop(ctx, 1, 2);
	printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);  /* res */
	duk_pop(ctx);  /* obj */

	/* use plain number as 'this', add function to Number.prototype; strict handler
	 * causes this to remain a plain number.
	 */
	duk_eval_string(ctx, "Number.prototype.func_strict = function (x,y) { 'use strict'; print(typeof this, this); return x+y; };");
	duk_pop(ctx);  /* pop result */
	duk_push_int(ctx, 123);  /* obj */
	duk_push_string(ctx, "func_strict");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_prop(ctx, 1, 2);
	printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);  /* res */
	duk_pop(ctx);  /* obj */

	/* basic error case */
	duk_eval_string(ctx, "({ name: 'me', foo: function (x,y) { throw new Error('my error'); } })");  /* idx 1 */
	duk_push_string(ctx, "foo");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_prop(ctx, 1, 2);
	printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);  /* res */
	duk_pop(ctx);  /* obj */

	duk_pop(ctx);  /* dummy */

	printf("final top: %d\n", duk_get_top(ctx));
}
