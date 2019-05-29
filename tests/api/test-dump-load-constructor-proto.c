/*
 *  Dump/load a constructor, check e.g. .prototype.
 */

/*===
*** test_1 (duk_safe_call)
dump
load
call
constructor called
object
true
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string(ctx,
		"(function MyConstructor() {\n"
		"    print('constructor called');\n"
		"    print(typeof MyConstructor.prototype);\n"
		"    print(Object.getPrototypeOf(MyConstructor.prototype) === Object.prototype);\n"
		"})\n");

	printf("dump\n");
	duk_dump_function(ctx);

	printf("load\n");
	duk_load_function(ctx);

	printf("call\n");
	duk_new(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
