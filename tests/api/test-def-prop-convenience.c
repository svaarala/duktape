/*
 *  duk_def_prop() convenience flags, added in Duktape 1.4.0.
 */

/*===
*** test_basic (duk_safe_call)
{value:undefined,writable:true,enumerable:true,configurable:true}
{value:undefined,writable:false,enumerable:true,configurable:true}
{value:undefined,writable:false,enumerable:false,configurable:true}
{value:undefined,writable:false,enumerable:false,configurable:false}
{value:undefined,writable:false,enumerable:false,configurable:true}
{value:undefined,writable:false,enumerable:true,configurable:true}
{value:undefined,writable:true,enumerable:true,configurable:true}
final top: 1
==> rc=0, result='undefined'
===*/

static void dump_prop(duk_context *ctx) {
	duk_eval_string(ctx,
		"(function (obj) {\n"
		"    print(Duktape.enc('jx', Object.getOwnPropertyDescriptor(obj, 'prop')));\n"
		"})");
	duk_dup(ctx, 0);
	duk_call(ctx, 1);
	duk_pop(ctx);
}

static duk_ret_t test_basic(duk_context *ctx) {
	duk_push_object(ctx);

	duk_push_string(ctx, "prop");
	duk_def_prop(ctx, -2, DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_ENUMERABLE | DUK_DEFPROP_SET_CONFIGURABLE);
	dump_prop(ctx);

	duk_push_string(ctx, "prop");
	duk_def_prop(ctx, -2, DUK_DEFPROP_CLEAR_WRITABLE);
	dump_prop(ctx);

	duk_push_string(ctx, "prop");
	duk_def_prop(ctx, -2, DUK_DEFPROP_CLEAR_ENUMERABLE);
	dump_prop(ctx);

	duk_push_string(ctx, "prop");
	duk_def_prop(ctx, -2, DUK_DEFPROP_CLEAR_CONFIGURABLE);
	dump_prop(ctx);

	duk_push_string(ctx, "prop");
	duk_def_prop(ctx, -2, DUK_DEFPROP_SET_CONFIGURABLE | DUK_DEFPROP_FORCE);
	dump_prop(ctx);

	duk_push_string(ctx, "prop");
	duk_def_prop(ctx, -2, DUK_DEFPROP_SET_ENUMERABLE);
	dump_prop(ctx);

	duk_push_string(ctx, "prop");
	duk_def_prop(ctx, -2, DUK_DEFPROP_SET_WRITABLE);
	dump_prop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
