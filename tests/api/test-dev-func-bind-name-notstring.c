/*
 *  Function.prototype.bind() must set the bound function .name as
 *  'bound ' + target .name.  However, if target .name does not exist
 *  or isn't a string, empty string is used instead.
 */

/*===
*** test_basic (duk_safe_call)
123
"bound "
final top: 2
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

	/* Create a function; the .name property is usually non-writable
	 * and non-configurable.  Then force the name to 123.
	 */
	duk_eval_string(ctx, "(function foo() {})");
	duk_push_string(ctx, "name");
	duk_push_int(ctx, 123);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_FORCE);

	/* ES2015 19.2.3.2 requires that a non-string .name is ignored and
	 * treated like an empty string.  The bound function .name is
	 * then required to be 'bound ' + .name.
	 */
	duk_eval_string(ctx,
		"(function (v) {\n"
		"    print(JSON.stringify(v.name));\n"
		"    var f = v.bind();\n"
		"    print(JSON.stringify(f.name));\n"
		"})\n");
	duk_dup(ctx, 0);
	duk_call(ctx, 1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
