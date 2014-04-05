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

int my_func(duk_context *ctx) {
	duk_c_function funcptr;

	printf("my_func, top=%d\n", duk_get_top(ctx));

	duk_push_current_function(ctx);
	printf("type=%d\n", duk_get_type(ctx, -1));
	printf("duk_is_object: %d\n", duk_is_object(ctx, -1));
	printf("duk_is_function: %d\n", duk_is_function(ctx, -1));

	funcptr = duk_get_c_function(ctx, -1);
	printf("duk_get_c_function matches my_func: %d\n", (my_func == funcptr ? 1 : 0));

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	int rc;

	/* first test what happens when there is no running function */

	printf("no running function\n");
	duk_push_current_function(ctx);
	printf("type=%d\n", duk_get_type(ctx, -1));
	printf("duk_is_object: %d\n", duk_is_object(ctx, -1));
	printf("duk_is_function: %d\n", duk_is_function(ctx, -1));
	duk_pop(ctx);

	/* then test the basic case */

	printf("basic case\n");
	duk_push_c_function(ctx, my_func, 1 /*nargs*/);
	duk_push_int(ctx, 123);
	rc = duk_pcall(ctx, 1);
	printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}

