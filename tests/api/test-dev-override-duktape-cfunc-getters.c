/*
 *  Override functions in %NativeFunctionPrototype%.  Here the example is to
 *  provide a name for anonymous native functions by looking up the C function
 *  address from a hypothetical symbol file.
 */

/*===
*** test_symbol_lookup (duk_safe_call)
foo called
bar called
RangeError: fake error from bar
    at [anon] (...) internal
    at myBar (...) native strict preventsyield
    at myFoo (...) native strict preventsyield
    at eval (...) preventsyield
final top: 1
==> rc=0, result='undefined'
===*/

static int my_foo(duk_context *ctx) {
	(void) ctx;
	printf("foo called\n");
	duk_get_global_string(ctx, "bar");
	duk_call(ctx, 0);
	return 0;
}

static int my_bar(duk_context *ctx) {
	(void) ctx;
	printf("bar called\n");
	duk_error(ctx, DUK_ERR_RANGE_ERROR, "fake error from bar");
	return 0;
}

/* Dummy symbol lookup function: map a native function pointer to a name
 * (and push it) or indicate no symbol was found.  An actual implementation
 * might consult a flash-based symbol file, for example.
 */
static int my_symbol_lookup(duk_context *ctx, duk_c_function func) {
	if (func == my_foo) {
		duk_push_string(ctx, "myFoo");
		return 1;
	} else if (func == my_bar) {
		duk_push_string(ctx, "myBar");
		return 1;
	}
	return 0;
}

static duk_ret_t my_name_getter(duk_context *ctx) {
	duk_c_function func;

	/* this: function/lightfunc */

	duk_push_this(ctx);
	func = duk_get_c_function(ctx, -1);
	if (func != NULL) {
		if (my_symbol_lookup(ctx, func)) {
			return 1;
		}
	}

	/* Not found, return empty string. */
	duk_push_string(ctx, "");
	return 1;
}

static duk_ret_t test_symbol_lookup(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_function(ctx, my_foo, 0);
	duk_put_global_string(ctx, "foo");
	duk_push_c_function(ctx, my_bar, 0);
	duk_put_global_string(ctx, "bar");

	/* The %NativeFunctionPrototype% is not directly accessible via e.g.
	 * the global object now, so look it up from a pushed function.
	 */
	duk_push_c_function(ctx, my_foo, 0);
	duk_get_prototype(ctx, -1);
	duk_remove(ctx, -2);

	duk_push_string(ctx, "name");
	duk_push_c_function(ctx, my_name_getter, 0);

	/* [ %NativeFunctionPrototype% "name" my_name_getter ] */

	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER);
	duk_pop(ctx);

	/* [ ] */

	/* Call foo() and get a traceback.  Demonstrate that the dynamically
	 * looked up names appear in the (censored) traceback.
	 */
	duk_eval_string(ctx,
		"try {\n"
		"    foo();\n"
		"} catch (e) {\n"
		"    var trace = e.stack;\n"
		"    trace = trace.replace(/\\(.*?\\)/g, '(...)');\n"
		"    print(trace);\n"
		"}\n");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_symbol_lookup);
}
