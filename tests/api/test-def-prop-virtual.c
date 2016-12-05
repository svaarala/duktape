/*
 *  Test DUK_DEFPROP_FORCE for virtual properties.
 */

/*===
*** test_array_length_enumerable_noforce (duk_safe_call)
set array .length enumerable
==> rc=1, result='TypeError: not configurable'
*** test_array_length_enumerable_force (duk_safe_call)
set array .length enumerable
==> rc=1, result='TypeError: not configurable'
*** test_array_length_configurable_noforce (duk_safe_call)
set array .length configurable
==> rc=1, result='TypeError: not configurable'
*** test_array_length_configurable_force (duk_safe_call)
set array .length configurable
==> rc=1, result='TypeError: not configurable'
*** test_array_length_overwrite_same_noforce (duk_safe_call)
["foo","bar","quux"]
final top: 0
==> rc=0, result='undefined'
*** test_array_length_overwrite_same_force (duk_safe_call)
["foo","bar","quux"]
final top: 0
==> rc=0, result='undefined'
*** test_array_length_overwrite_bigger_noforce (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_array_length_overwrite_bigger_force (duk_safe_call)
["foo","bar","quux",undefined,undefined]
final top: 0
==> rc=0, result='undefined'
*** test_array_length_overwrite_smaller_noforce (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_array_length_overwrite_smaller_force (duk_safe_call)
["foo"]
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test__array_length_enumerable(duk_context *ctx, int force) {
	duk_push_array(ctx);

	duk_push_string(ctx, "length");
	printf("set array .length enumerable\n");
	fflush(stdout);
	duk_def_prop(ctx, -2, DUK_DEFPROP_SET_ENUMERABLE | (force ? DUK_DEFPROP_FORCE : 0));

	duk_eval_string(ctx,
		"(function (v) { print(Duktape.enc('jx', Object.getOwnPropertyDescriptor(v, 'length' ))); })");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);

	duk_pop_2(ctx);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_array_length_enumerable_noforce(duk_context *ctx, void *udata) {
	(void) udata;
	return test__array_length_enumerable(ctx, 0);
}
static duk_ret_t test_array_length_enumerable_force(duk_context *ctx, void *udata) {
	(void) udata;
	return test__array_length_enumerable(ctx, 1);
}

static duk_ret_t test__array_length_configurable(duk_context *ctx, int force) {
	duk_push_array(ctx);

	duk_push_string(ctx, "length");
	printf("set array .length configurable\n");
	fflush(stdout);
	duk_def_prop(ctx, -2, DUK_DEFPROP_SET_CONFIGURABLE | (force ? DUK_DEFPROP_FORCE : 0));

	duk_eval_string(ctx,
		"(function (v) { print(Duktape.enc('jx', Object.getOwnPropertyDescriptor(v, 'length' ))); })");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);

	duk_pop_2(ctx);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_array_length_configurable_noforce(duk_context *ctx, void *udata) {
	(void) udata;
	return test__array_length_configurable(ctx, 0);
}
static duk_ret_t test_array_length_configurable_force(duk_context *ctx, void *udata) {
	(void) udata;
	return test__array_length_configurable(ctx, 1);
}

static duk_ret_t test__length_overwrite(duk_context *ctx, int force, int new_len) {
	duk_eval_string(ctx,
		"(function () {\n"
		"    var arr = [ 'foo', 'bar', 'quux' ];\n"
		"    Object.defineProperty(arr, 'length', { writable: false });\n"
		"    return arr;\n"
		"})()\n");

	/* Array .length is not writable; with DUK_DEFPROP_FORCE we can still
	 * write it.
	 */
	duk_push_string(ctx, "length");
	duk_push_int(ctx, new_len);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | (force ? DUK_DEFPROP_FORCE : 0));

	duk_eval_string(ctx,
		"(function (v) { print(Duktape.enc('jx', v)); })");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);

	duk_pop_2(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_array_length_overwrite_same_noforce(duk_context *ctx, void *udata) {
	(void) udata;
	return test__length_overwrite(ctx, 0, 3);
}
static duk_ret_t test_array_length_overwrite_same_force(duk_context *ctx, void *udata) {
	(void) udata;
	return test__length_overwrite(ctx, 1, 3);
}
static duk_ret_t test_array_length_overwrite_bigger_noforce(duk_context *ctx, void *udata) {
	(void) udata;
	return test__length_overwrite(ctx, 0, 5);
}
static duk_ret_t test_array_length_overwrite_bigger_force(duk_context *ctx, void *udata) {
	(void) udata;
	return test__length_overwrite(ctx, 1, 5);
}
static duk_ret_t test_array_length_overwrite_smaller_noforce(duk_context *ctx, void *udata) {
	(void) udata;
	return test__length_overwrite(ctx, 0, 1);
}
static duk_ret_t test_array_length_overwrite_smaller_force(duk_context *ctx, void *udata) {
	(void) udata;
	return test__length_overwrite(ctx, 1, 1);
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_array_length_enumerable_noforce);
	TEST_SAFE_CALL(test_array_length_enumerable_force);
	TEST_SAFE_CALL(test_array_length_configurable_noforce);
	TEST_SAFE_CALL(test_array_length_configurable_force);
	TEST_SAFE_CALL(test_array_length_overwrite_same_noforce);
	TEST_SAFE_CALL(test_array_length_overwrite_same_force);
	TEST_SAFE_CALL(test_array_length_overwrite_bigger_noforce);
	TEST_SAFE_CALL(test_array_length_overwrite_bigger_force);
	TEST_SAFE_CALL(test_array_length_overwrite_smaller_noforce);
	TEST_SAFE_CALL(test_array_length_overwrite_smaller_force);
}
