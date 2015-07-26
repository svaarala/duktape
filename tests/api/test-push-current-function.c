/*===
no running function
type=1
duk_is_object: 0
duk_is_function: 0
basic case
my_func, top=1
type=6
duk_is_object: 1
duk_is_function: 1
duk_get_c_function matches my_func: 1
final top: 2
rc=0, result='undefined'
===*/

static duk_ret_t my_func(duk_context *ctx) {
	duk_c_function funcptr;

	printf("my_func, top=%ld\n", (long) duk_get_top(ctx));

	duk_push_current_function(ctx);
	printf("type=%d\n", (int) duk_get_type(ctx, -1));
	printf("duk_is_object: %d\n", (int) duk_is_object(ctx, -1));
	printf("duk_is_function: %d\n", (int) duk_is_function(ctx, -1));

	funcptr = duk_get_c_function(ctx, -1);
	printf("duk_get_c_function matches my_func: %d\n", (my_func == funcptr ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	duk_ret_t rc;

	/* first test what happens when there is no running function */

	printf("no running function\n");
	duk_push_current_function(ctx);
	printf("type=%d\n", (int) duk_get_type(ctx, -1));
	printf("duk_is_object: %d\n", (int) duk_is_object(ctx, -1));
	printf("duk_is_function: %d\n", (int) duk_is_function(ctx, -1));
	duk_pop(ctx);

	/* then test the basic case */

	printf("basic case\n");
	duk_push_c_function(ctx, my_func, 1 /*nargs*/);
	duk_push_int(ctx, 123);
	rc = duk_pcall(ctx, 1);
	printf("rc=%d, result='%s'\n", (int) rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}
