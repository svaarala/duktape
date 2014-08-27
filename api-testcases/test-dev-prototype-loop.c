/*
 *  Prototype loop is tricky to handle internally and must not cause e.g.
 *  GC failures.  Exercise all prototype walk sites, and a few other
 *  specific cases.
 *
 *  At the moment prototype loops cause an error to be thrown so that
 *  they are easily noticed (used code should never intentionally create
 *  one).  Internal stuff, like GC handling, must -avoid- throwing errors
 *  in critical situations so that e.g. GC doesn't terminate with an
 *  uncaught error.  This testcase documents the current behavior.
 *
 *  Sites can be found with DUK_HOBJECT_PROTOTYPE_CHAIN_SANITY and then
 *  looking at possible callers:
 *
 *    - duk_hobject_prototype_chain_contains(): see below
 *
 *    - duk__get_property_desc: can be exercised through duk_hobject_hasprop
 *
 *    - duk_hobject_getprop
 *
 *    - duk_hobject_putprop
 *
 *    - duk_js_instanceof
 *
 *    - duk__get_identifier_reference: walks environments chained using the
 *      prototype reference; these cannot be in a loop and user code cannot
 *      access these
 *
 *  duk_hobject_prototype_chain_contains() call sites:
 *
 *    - Object.prototype.isPrototypeOf()
 *
 *    - duk_err_augment_error_create(): can be exercised by throwing an
 *      object with a prototype loop
 */

/*===
*** test_gc (duk_safe_call)
first gc
make unreachable
second gc
==> rc=0, result='undefined'
*** test_is_prototype_of (duk_safe_call)
Object.prototype.isPrototypeOf result: false
Object.prototype.isPrototypeOf result: true
==> rc=1, result='Error: prototype chain limit'
*** test_error_augment (duk_safe_call)
ret=1
throw value .foo=123
==> rc=0, result='undefined'
*** test_hasprop (duk_safe_call)
hasprop foo: 1
hasprop bar: 1
==> rc=1, result='Error: prototype chain limit'
*** test_getprop (duk_safe_call)
getprop foo: 123
getprop bar: 321
==> rc=1, result='Error: prototype chain limit'
*** test_putprop (duk_safe_call)
putprop foo done
putprop bar done
==> rc=1, result='Error: prototype chain limit'
*** test_instanceof (duk_safe_call)
object function
true
object function
true
object function
==> rc=1, result='Error: prototype chain limit'
still here
===*/

static void prep(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_object(ctx);
	duk_push_object(ctx);
	duk_push_int(ctx, 123);
	duk_put_prop_string(ctx, 0, "foo");
	duk_push_int(ctx, 321);
	duk_put_prop_string(ctx, 1, "bar");

	duk_dup(ctx, 0);
	duk_set_prototype(ctx, 1);
	duk_dup(ctx, 1);
	duk_set_prototype(ctx, 0);

	/* Two objects on stack, in a prototype loop with each other.
	 * One object has a 'foo' property, the other a 'bar' property.
	 */
}

static duk_ret_t test_gc(duk_context *ctx) {
	prep(ctx);

	/* Both objects are now in a prototype loop.  Force garbage
	 * collection to ensure nothing breaks.
	 */

	printf("first gc\n"); fflush(stdout);
	duk_gc(ctx, 0);

	/* Make the objects unreachable and re-run GC.  This triggers
	 * e.g. finalizer checks.
	 */

	printf("make unreachable\n"); fflush(stdout);
	duk_set_top(ctx, 0);

	printf("second gc\n"); fflush(stdout);
	duk_gc(ctx, 0);

	return 0;
}

static duk_ret_t test_is_prototype_of(duk_context *ctx) {
	prep(ctx);

	/* obj0.isPrototypeOf(dummy) -> false, traverses prototype chain of dummy */
	duk_eval_string(ctx, "Object.prototype.isPrototypeOf");
	duk_dup(ctx, 0);
	duk_push_object(ctx);
	duk_call_method(ctx, 1);
	printf("Object.prototype.isPrototypeOf result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* obj0.isPrototypeOf(obj1) -> true, traverses prototype chain of obj1 */
	duk_eval_string(ctx, "Object.prototype.isPrototypeOf");
	duk_dup(ctx, 0);
	duk_dup(ctx, 1);
	duk_call_method(ctx, 1);
	printf("Object.prototype.isPrototypeOf result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* dummy.isPrototypeOf(obj0) -> traverses prototype chain of obj0 and throws */
	duk_eval_string(ctx, "Object.prototype.isPrototypeOf");
	duk_push_object(ctx);
	duk_dup(ctx, 0);
	duk_call_method(ctx, 1);
	printf("Object.prototype.isPrototypeOf result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	return 0;
}

static duk_ret_t augment_raw(duk_context *ctx) {
	duk_throw(ctx);
	return 0;
}

static duk_ret_t test_error_augment(duk_context *ctx) {
	duk_int_t ret;

	prep(ctx);

	/* This case is a bit tricky.  There used to be a problem where the
	 * error augmentation process itself failed when checking whether or
	 * not the throw value inherited from Error.  This has now been fixed
	 * and the error value no longer gets augmented and is thrown correctly.
	 *
	 * The TEST_SAFE_CALL() wrapper uses duk_safe_to_string() to coerce
	 * the throw result.  Since the object doesn't have a toString()
	 * function, this coercion will fail and generate a prototype loop
	 * error!
	 *
	 * So, use a separate duk_safe_call() wrapping here to ensure we treat
	 * the final result value carefully.  We print out 'foo' to be sure
	 * the correct value was thrown.
	 */

	duk_dup(ctx, 0);
	ret = duk_safe_call(ctx, augment_raw, 0 /*nargs*/, 1 /*nrets*/);
	printf("ret=%d\n", (int) ret);
	duk_get_prop_string(ctx, -1, "foo");
	printf("throw value .foo=%d\n", duk_get_int(ctx, -1));
	duk_pop_2(ctx);

	return 0;
}

static duk_ret_t test_hasprop(duk_context *ctx) {
	duk_bool_t ret;

	prep(ctx);

	/* Property exists, own property */
	ret = duk_has_prop_string(ctx, 0, "foo");
	printf("hasprop foo: %d\n", (int) ret);

	/* Property exists, inherited property */
	ret = duk_has_prop_string(ctx, 0, "bar");
	printf("hasprop bar: %d\n", (int) ret);

	/* Property doesn't exist, terminate with error */
	ret = duk_has_prop_string(ctx, 0, "quux");
	printf("hasprop quux: %d\n", (int) ret);

	return 0;
}

static duk_ret_t test_getprop(duk_context *ctx) {
	prep(ctx);

	/* Property exists, own property */
	duk_get_prop_string(ctx, 0, "foo");
	printf("getprop foo: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* Property exists, inherited property */
	duk_get_prop_string(ctx, 0, "bar");
	printf("getprop bar: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* Property doesn't exist, terminate with error */
	duk_get_prop_string(ctx, 0, "quux");
	printf("getprop quux: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	return 0;
}

static duk_ret_t test_putprop(duk_context *ctx) {
	prep(ctx);

	/* Property exists, own property */
	duk_push_int(ctx, 1001);
	duk_put_prop_string(ctx, 0, "foo");
	printf("putprop foo done\n");

	/* Property exists, inherited property */
	duk_push_int(ctx, 1002);
	duk_put_prop_string(ctx, 0, "bar");
	printf("putprop bar done\n");

	/* Property doesn't exist, terminate with error */
	duk_push_int(ctx, 1003);
	duk_put_prop_string(ctx, 0, "quux");
	printf("putprop quux done\n");

	return 0;
}

static duk_ret_t test_instanceof(duk_context *ctx) {
	prep(ctx);

	/* For 'a instanceof b', the [[HasInstance]] algorithm looks up
	 * b.prototype and then walks the internal prototype chain of 'a'
	 * looking for b.prototype.  The rvalue must also be a Function.
	 * So we need a temporary object wrapping one of the objects
	 * created by prep().
	 */

	/* obj0 instanceof { prototype: obj0 } -> true, found */

	duk_eval_string(ctx, "(function (a,b) { print(typeof a, typeof b); print(a instanceof b); })");
	duk_dup(ctx, 0);
	duk_eval_string(ctx, "(function() {})");
	duk_dup(ctx, 0);
	duk_put_prop_string(ctx, -2, "prototype");
	duk_call(ctx, 2);
	duk_pop(ctx);

	/* obj0 instanceof { prototype: obj1 } -> true, found */

	duk_eval_string(ctx, "(function (a,b) { print(typeof a, typeof b); print(a instanceof b); })");
	duk_dup(ctx, 0);
	duk_eval_string(ctx, "(function() {})");
	duk_dup(ctx, 0);
	duk_put_prop_string(ctx, -2, "prototype");
	duk_call(ctx, 2);
	duk_pop(ctx);

	/* obj0 instanceof { prototype: dummy } -> error, loop */

	duk_eval_string(ctx, "(function (a,b) { print(typeof a, typeof b); print(a instanceof b); })");
	duk_dup(ctx, 0);
	duk_eval_string(ctx, "(function() {})");
	duk_push_object(ctx);
	duk_put_prop_string(ctx, -2, "prototype");
	duk_call(ctx, 2);
	duk_pop(ctx);

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_gc);
	TEST_SAFE_CALL(test_is_prototype_of);
	TEST_SAFE_CALL(test_error_augment);
	TEST_SAFE_CALL(test_hasprop);
	TEST_SAFE_CALL(test_getprop);
	TEST_SAFE_CALL(test_putprop);
	TEST_SAFE_CALL(test_instanceof);
	printf("still here\n"); fflush(stdout);
}
