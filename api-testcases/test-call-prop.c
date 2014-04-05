/*===
[object Object]
result=33
final top: 1
rc=0, result='undefined'
[object Object]
rc=1, result='Error: my error'
object 1 [object Number]
result=undefined
number 1 [object Number]
result=undefined
final top: 1
rc=0, result='undefined'
===*/

int test_1(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_eval_string(ctx, "({ myfunc: function(x,y,z) { print(this); return x+y+z; } })");
	duk_push_string(ctx, "myfunc");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	duk_push_int(ctx, 12);
	duk_push_int(ctx, 13);  /* clipped */
	duk_push_int(ctx, 14);  /* clipped */

	/* [ ... obj "myfunc" 10 11 12 13 14 ] */

	duk_call_prop(ctx, 0, 5);

	/* [ ... obj res ] */

	printf("result=%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_eval_string(ctx, "({ myfunc: function(x,y,z) { print(this); throw new Error('my error'); } })");
	duk_push_string(ctx, "myfunc");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	duk_push_int(ctx, 12);
	duk_push_int(ctx, 13);  /* clipped */
	duk_push_int(ctx, 14);  /* clipped */

	/* [ ... obj "myfunc" 10 11 12 13 14 ] */

	duk_call_prop(ctx, 0, 5);

	/* [ ... obj res ] */

	printf("result=%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);


	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* test this coercion in strict/non-strict functions */
int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);

	/* Use Number.prototype to stash new functions, and call "through" a
	 * plain number value.  The 'this' binding is initially the plain number.
	 * A strict function gets the plain number as is, a non-strict function
	 * gets an object coerced version.
	 */

	duk_eval_string(ctx, "Number.prototype.myfunc1 = function() { print(typeof this, this, Object.prototype.toString.call(this)); };");
	duk_pop(ctx);
	duk_eval_string(ctx, "Number.prototype.myfunc2 = function() { 'use strict'; print(typeof this, this, Object.prototype.toString.call(this)); };");
	duk_pop(ctx);

	duk_push_int(ctx, 1);  /* use '1' as 'obj' */

	duk_push_string(ctx, "myfunc1");  /* -> [ ... obj "myfunc1" ] */
	duk_call_prop(ctx, 0, 0);        /* -> [ ... obj res ] */
	printf("result=%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_push_string(ctx, "myfunc2");  /* -> [ ... obj "myfunc2" ] */
	duk_call_prop(ctx, 0, 0);        /* -> [ ... obj res ] */
	printf("result=%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}


void test(duk_context *ctx) {
	int rc;

	rc = duk_safe_call(ctx, test_1, 0, 1);
	printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_2, 0, 1);
	printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_3, 0, 1);
	printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}
