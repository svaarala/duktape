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
*** test_flags (duk_safe_call)
flags: DUK_DEFPROP_SET_WRITABLE
{value:123,writable:true,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:true,enumerable:false,configurable:false}
flags: DUK_DEFPROP_CLEAR_WRITABLE
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:false,enumerable:true,configurable:true}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_SET_ENUMERABLE
{value:123,writable:false,enumerable:true,configurable:false}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:false,enumerable:true,configurable:false}
flags: DUK_DEFPROP_CLEAR_ENUMERABLE
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:false,configurable:true}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_SET_CONFIGURABLE
{value:123,writable:false,enumerable:false,configurable:true}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:false,enumerable:false,configurable:true}
flags: DUK_DEFPROP_CLEAR_CONFIGURABLE
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:true,configurable:false}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_W
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_E
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_C
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_WE
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_WC
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_WEC
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_HAVE_W
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:false,enumerable:true,configurable:true}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_HAVE_E
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:false,configurable:true}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_HAVE_C
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:true,configurable:false}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_HAVE_WE
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:false,enumerable:false,configurable:true}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_HAVE_WC
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:false,enumerable:true,configurable:false}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_HAVE_WEC
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_SET_W
{value:123,writable:true,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:true,enumerable:false,configurable:false}
flags: DUK_DEFPROP_SET_E
{value:123,writable:false,enumerable:true,configurable:false}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:false,enumerable:true,configurable:false}
flags: DUK_DEFPROP_SET_C
{value:123,writable:false,enumerable:false,configurable:true}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:false,enumerable:false,configurable:true}
flags: DUK_DEFPROP_SET_WE
{value:123,writable:true,enumerable:true,configurable:false}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:true,enumerable:true,configurable:false}
flags: DUK_DEFPROP_SET_WC
{value:123,writable:true,enumerable:false,configurable:true}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:true,enumerable:false,configurable:true}
flags: DUK_DEFPROP_SET_WEC
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:true,enumerable:true,configurable:true}
flags: DUK_DEFPROP_CLEAR_W
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:false,enumerable:true,configurable:true}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_CLEAR_E
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:false,configurable:true}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_CLEAR_C
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:true,configurable:false}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_CLEAR_WE
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:false,enumerable:false,configurable:true}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_CLEAR_WC
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:false,enumerable:true,configurable:false}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_CLEAR_WEC
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:false,enumerable:false,configurable:false}
{value:123,writable:false,enumerable:false,configurable:false}
flags: DUK_DEFPROP_ATTR_W
{value:123,writable:true,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:false,configurable:false}
flags: DUK_DEFPROP_ATTR_E
{value:123,writable:false,enumerable:true,configurable:false}
{value:123,writable:false,enumerable:true,configurable:false}
{value:123,writable:false,enumerable:true,configurable:false}
flags: DUK_DEFPROP_ATTR_C
{value:123,writable:false,enumerable:false,configurable:true}
{value:123,writable:false,enumerable:false,configurable:true}
{value:123,writable:false,enumerable:false,configurable:true}
flags: DUK_DEFPROP_ATTR_WE
{value:123,writable:true,enumerable:true,configurable:false}
{value:123,writable:true,enumerable:true,configurable:false}
{value:123,writable:true,enumerable:true,configurable:false}
flags: DUK_DEFPROP_ATTR_WC
{value:123,writable:true,enumerable:false,configurable:true}
{value:123,writable:true,enumerable:false,configurable:true}
{value:123,writable:true,enumerable:false,configurable:true}
flags: DUK_DEFPROP_ATTR_WEC
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:true,enumerable:true,configurable:true}
flags: DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_W
{value:123,writable:true,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:false,configurable:false}
{value:123,writable:true,enumerable:false,configurable:false}
flags: DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_E
{value:123,writable:false,enumerable:true,configurable:false}
{value:123,writable:false,enumerable:true,configurable:false}
{value:123,writable:false,enumerable:true,configurable:false}
flags: DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_C
{value:123,writable:false,enumerable:false,configurable:true}
{value:123,writable:false,enumerable:false,configurable:true}
{value:123,writable:false,enumerable:false,configurable:true}
flags: DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WE
{value:123,writable:true,enumerable:true,configurable:false}
{value:123,writable:true,enumerable:true,configurable:false}
{value:123,writable:true,enumerable:true,configurable:false}
flags: DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WC
{value:123,writable:true,enumerable:false,configurable:true}
{value:123,writable:true,enumerable:false,configurable:true}
{value:123,writable:true,enumerable:false,configurable:true}
flags: DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WEC
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:true,enumerable:true,configurable:true}
{value:123,writable:true,enumerable:true,configurable:true}
final top: 0
==> rc=0, result='undefined'
===*/

static void dump_prop(duk_context *ctx) {
	duk_eval_string(ctx,
		"(function (obj) {\n"
		"    print(Duktape.enc('jx', Object.getOwnPropertyDescriptor(obj, 'prop')));\n"
		"})");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);
}

static void test_one_flags(duk_context *ctx, duk_uint_t defprop_flags) {
	duk_eval_string(ctx, "({})");
	duk_push_string(ctx, "prop");
	duk_push_uint(ctx, 123);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_FORCE | defprop_flags);
	dump_prop(ctx);
	duk_pop(ctx);

	duk_eval_string(ctx, "(function () { var o = {}; Object.defineProperty(o, 'prop', {value:-123,writable:true,enumerable:true,configurable:true}); return o; })()");
	duk_push_string(ctx, "prop");
	duk_push_uint(ctx, 123);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_FORCE | defprop_flags);
	dump_prop(ctx);
	duk_pop(ctx);

	duk_eval_string(ctx, "(function () { var o = {}; Object.defineProperty(o, 'prop', {value:-123,writable:false,enumerable:false,configurable:false}); return o; })()");
	duk_push_string(ctx, "prop");
	duk_push_uint(ctx, 123);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_FORCE | defprop_flags);
	dump_prop(ctx);
	duk_pop(ctx);
}

#define TEST_ONE_FLAGS(flags) do { \
		printf("flags: %s\n", #flags); \
		test_one_flags(ctx, flags); \
	} while (0)

static duk_ret_t test_flags(duk_context *ctx, void *udata) {
	(void) udata;

	TEST_ONE_FLAGS(DUK_DEFPROP_SET_WRITABLE);
	TEST_ONE_FLAGS(DUK_DEFPROP_CLEAR_WRITABLE);
	TEST_ONE_FLAGS(DUK_DEFPROP_SET_ENUMERABLE);
	TEST_ONE_FLAGS(DUK_DEFPROP_CLEAR_ENUMERABLE);
	TEST_ONE_FLAGS(DUK_DEFPROP_SET_CONFIGURABLE);
	TEST_ONE_FLAGS(DUK_DEFPROP_CLEAR_CONFIGURABLE);
	TEST_ONE_FLAGS(DUK_DEFPROP_W);
	TEST_ONE_FLAGS(DUK_DEFPROP_E);
	TEST_ONE_FLAGS(DUK_DEFPROP_C);
	TEST_ONE_FLAGS(DUK_DEFPROP_WE);
	TEST_ONE_FLAGS(DUK_DEFPROP_WC);
	TEST_ONE_FLAGS(DUK_DEFPROP_WEC);
	TEST_ONE_FLAGS(DUK_DEFPROP_HAVE_W);
	TEST_ONE_FLAGS(DUK_DEFPROP_HAVE_E);
	TEST_ONE_FLAGS(DUK_DEFPROP_HAVE_C);
	TEST_ONE_FLAGS(DUK_DEFPROP_HAVE_WE);
	TEST_ONE_FLAGS(DUK_DEFPROP_HAVE_WC);
	TEST_ONE_FLAGS(DUK_DEFPROP_HAVE_WEC);
	TEST_ONE_FLAGS(DUK_DEFPROP_SET_W);
	TEST_ONE_FLAGS(DUK_DEFPROP_SET_E);
	TEST_ONE_FLAGS(DUK_DEFPROP_SET_C);
	TEST_ONE_FLAGS(DUK_DEFPROP_SET_WE);
	TEST_ONE_FLAGS(DUK_DEFPROP_SET_WC);
	TEST_ONE_FLAGS(DUK_DEFPROP_SET_WEC);
	TEST_ONE_FLAGS(DUK_DEFPROP_CLEAR_W);
	TEST_ONE_FLAGS(DUK_DEFPROP_CLEAR_E);
	TEST_ONE_FLAGS(DUK_DEFPROP_CLEAR_C);
	TEST_ONE_FLAGS(DUK_DEFPROP_CLEAR_WE);
	TEST_ONE_FLAGS(DUK_DEFPROP_CLEAR_WC);
	TEST_ONE_FLAGS(DUK_DEFPROP_CLEAR_WEC);
	TEST_ONE_FLAGS(DUK_DEFPROP_ATTR_W);
	TEST_ONE_FLAGS(DUK_DEFPROP_ATTR_E);
	TEST_ONE_FLAGS(DUK_DEFPROP_ATTR_C);
	TEST_ONE_FLAGS(DUK_DEFPROP_ATTR_WE);
	TEST_ONE_FLAGS(DUK_DEFPROP_ATTR_WC);
	TEST_ONE_FLAGS(DUK_DEFPROP_ATTR_WEC);

	/* Need HAVE_WEC to test plain DUK_DEFPROP_W etc */
	TEST_ONE_FLAGS(DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_W);
	TEST_ONE_FLAGS(DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_E);
	TEST_ONE_FLAGS(DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_C);
	TEST_ONE_FLAGS(DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WE);
	TEST_ONE_FLAGS(DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WC);
	TEST_ONE_FLAGS(DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_WEC);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

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
	TEST_SAFE_CALL(test_flags);
}
