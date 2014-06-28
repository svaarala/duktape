/*===
top: 11
this binding: type=1, value='undefined'
this binding: type=2, value='null'
this binding: type=3, value='true'
this binding: type=3, value='false'
this binding: type=4, value='123.456'
this binding: type=5, value='foo'
this binding: type=6, value='[object Object]'
this binding: type=6, value=''
this binding: type=7, value=''
this binding: type=8, value='0xdeadbeef'
===*/

/* Test how 'this' binding works.  Also tests that Duktape/C functions are
 * strict, i.e. that the 'this' binding is not coerced when a method call
 * happens.
 */

static duk_ret_t func(duk_context *ctx) {
	duk_int_t t;
	duk_push_this(ctx);
	t = duk_get_type(ctx, -1);
	printf("this binding: type=%ld, value='%s'\n", (long) t, duk_to_string(ctx, -1));
	return 0;
}

void test(duk_context *ctx) {
	duk_idx_t i, n;

	duk_push_c_function(ctx, func, 0);

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_number(ctx, 123.456);
	duk_push_string(ctx, "foo");
	duk_push_object(ctx);
	duk_push_array(ctx);
	duk_push_fixed_buffer(ctx, 16);
	duk_push_pointer(ctx, (void *) 0xdeadbeef);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 1; i < n; i++) {
		duk_dup(ctx, 0);
		duk_dup(ctx, i);
		duk_call_method(ctx, 0);  /* [ ... func this ] -> [ ret ] */
		duk_pop(ctx);
	}
}
