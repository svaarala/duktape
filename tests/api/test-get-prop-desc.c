/*
 *  duk_get_prop_desc()
 */

/*===
*** test_basic (duk_safe_call)
top before: 4
top after: 4
top before: 5
top after: 5
{value:"foo",writable:true,enumerable:true,configurable:false}
{get:{_func:true},set:{_func:true},enumerable:false,configurable:true}
final top: 6
==> rc=0, result='undefined'
*** test_nonobject (duk_safe_call)
==> rc=1, result='TypeError: object required, found null (stack index 0)'
*** test_invalid_index (duk_safe_call)
==> rc=1, result='TypeError: object required, found none (stack index 3)'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_string(ctx, "dummy");
	duk_eval_string(ctx,
		"(function () {\n"
		"    var obj = {};\n"
		"    Object.defineProperties(obj, {\n"
		"        normal: { value: 'foo', writable: true, enumerable: true, configurable: false },\n"
		"        accessor: { get: function mygetter() {}, set: function mysetter() {}, enumerable: false, configurable: true }\n"
		"    });\n"
		"    return obj;\n"
		"})()\n");

	duk_push_string(ctx, "dummy");

	/* [ dummy obj dummy ] */

	duk_push_string(ctx, "normal");
	printf("top before: %ld\n", (long) duk_get_top(ctx));
	duk_get_prop_desc(ctx, -3, 0 /*flags*/);
	printf("top after: %ld\n", (long) duk_get_top(ctx));

	duk_push_string(ctx, "accessor");
	printf("top before: %ld\n", (long) duk_get_top(ctx));
	duk_get_prop_desc(ctx, 1, 0 /*flags*/);
	printf("top after: %ld\n", (long) duk_get_top(ctx));

	/* [ dummy obj dummy desc1 desc2 ] */

	duk_eval_string(ctx,
		"(function (d1, d2) {\n"
		"    print(Duktape.enc('jx', d1));\n"
		"    print(Duktape.enc('jx', d2));\n"
		"})");
	duk_dup(ctx, 3);
	duk_dup(ctx, 4);
	duk_call(ctx, 2);

	/* [ dummy obj dummy desc1 desc2 result ] */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_nonobject(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_null(ctx);
	duk_push_string(ctx, "prop");
	duk_get_prop_desc(ctx, 0, 0 /*flags*/);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_invalid_index(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_null(ctx);
	duk_push_string(ctx, "prop");
	duk_get_prop_desc(ctx, 3, 0 /*flags*/);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_nonobject);
	TEST_SAFE_CALL(test_invalid_index);
}
