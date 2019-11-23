/*
 *  Testcase for object prototype manipulation
 *
 *  Test also prototype loops.  The ECMAScript primitives like Object.create()
 *  and Object.setPrototypeOf() guard against creating a prototype loop (as
 *  required by the specification) but no such safeguard is used for the C API.
 *  A prototype loop is expected to be terminated by a sanity limit inside
 *  Duktape which is explicitly implemented for all prototype traversals.
 *  Even so, user code is expected to never create a prototype loop on purpose.
 */

/* XXX: missing tests for API call type validation */

/*===
*** test_basic (duk_safe_call)
top before set: 2
top after set: 1
top before get: 1
top after get: 2
prototype: undefined
top before get: 2
top after get: 3
obj1 proto === Object.prototype: 1
obj1.isPrototypeOf is undefined: 0
top before set: 3
top after set: 2
top before get: 2
top after get: 3
prototype: TypeError: coercion to primitive failed
top before get: 2
top after get: 3
obj1 proto === Object.prototype: 0
obj1 proto === obj0: 1
obj1.isPrototypeOf is undefined: 1
obj1.foo=123
final top: 2
==> rc=0, result='undefined'
*** test_loop (duk_safe_call)
set obj0 prototype to obj1
set obj1 prototype to obj0
obj0.foo=123
obj0.bar=123
==> rc=1, result='RangeError: prototype chain limit'
===*/

/* Multiple basic tests in one: test duk_set_prototype() and duk_get_prototype()
 * stack top changes, and object/undefined for duk_set_prototype().  Also checks
 * how a bare object works.
 */
static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

	/* Prototype object: { foo: 123 }, internal prototype is null. */
	duk_push_object(ctx);
	duk_push_undefined(ctx);
	printf("top before set: %ld\n", (long) duk_get_top(ctx));
	duk_set_prototype(ctx, -2);
	printf("top after set: %ld\n", (long) duk_get_top(ctx));
	duk_push_int(ctx, 123);
	duk_put_prop_string(ctx, -2, "foo");

	/* The prototype object is "bare", read back its prototype. */
	printf("top before get: %ld\n", (long) duk_get_top(ctx));
	duk_get_prototype(ctx, 0);
	printf("top after get: %ld\n", (long) duk_get_top(ctx));
	printf("prototype: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* Target object, initially inherits from Object.prototype. */
	duk_push_object(ctx);

	/* Check original prototype. */
	printf("top before get: %ld\n", (long) duk_get_top(ctx));
	duk_get_prototype(ctx, 1);
	printf("top after get: %ld\n", (long) duk_get_top(ctx));
	duk_eval_string(ctx, "Object.prototype");
	printf("obj1 proto === Object.prototype: %d\n", (int) duk_strict_equals(ctx, -2, -1));
	duk_pop_2(ctx);

	/* Check Object.prototype inheritance in practice. */
	duk_get_prop_string(ctx, 1, "isPrototypeOf");
	printf("obj1.isPrototypeOf is undefined: %d\n", (int) duk_is_undefined(ctx, -1));
	duk_pop(ctx);

	/* Change prototype. */
	duk_dup(ctx, 0);
	printf("top before set: %ld\n", (long) duk_get_top(ctx));
	duk_set_prototype(ctx, -2);
	printf("top after set: %ld\n", (long) duk_get_top(ctx));

	/* Read back the prototype.  The object is "bare" and doesn't have
	 * valueOf() or toString(), so that the string coercion will fail
	 * here on purpose.  Unfortunately this check depends on the specific
	 * error message and is brittle.
	 */
	printf("top before get: %ld\n", (long) duk_get_top(ctx));
	duk_get_prototype(ctx, 1);
	printf("top after get: %ld\n", (long) duk_get_top(ctx));
	printf("prototype: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* Check that prototype is no longer Object.prototype. */
	printf("top before get: %ld\n", (long) duk_get_top(ctx));
	duk_get_prototype(ctx, 1);
	printf("top after get: %ld\n", (long) duk_get_top(ctx));
	duk_eval_string(ctx, "Object.prototype");
	printf("obj1 proto === Object.prototype: %d\n", (int) duk_strict_equals(ctx, -2, -1));
	printf("obj1 proto === obj0: %d\n", (int) duk_strict_equals(ctx, -2, 0));
	duk_pop_2(ctx);

	/* Check that isPrototypeOf can no longer be accessed. */
	duk_get_prop_string(ctx, 1, "isPrototypeOf");
	printf("obj1.isPrototypeOf is undefined: %d\n", (int) duk_is_undefined(ctx, -1));
	duk_pop(ctx);

	/* Check that 'foo' can be accessed. */
	duk_get_prop_string(ctx, 1, "foo");
	printf("obj1.foo=%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_loop(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_object(ctx);
	duk_push_int(ctx, 123);
	duk_put_prop_string(ctx, -2, "foo");

	duk_push_object(ctx);
	duk_push_int(ctx, 123);
	duk_put_prop_string(ctx, -2, "bar");

	/* Set object at index 0 and index 1 to use each other as their
	 * prototype and check that Duktape sanity bails out for a prototype
	 * lookup.
	 *
	 * NOTE: User code should always avoid creating prototype loops!
	 */

	printf("set obj0 prototype to obj1\n");
	duk_dup(ctx, 0);
	duk_set_prototype(ctx, 1);

	printf("set obj1 prototype to obj0\n");
	duk_dup(ctx, 1);
	duk_set_prototype(ctx, 0);

	/* For existing property, prototype loop has no impact. */

	duk_get_prop_string(ctx, 0, "foo");
	printf("obj0.foo=%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "bar");
	printf("obj0.bar=%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* Non-existent property causes the prototype loop to be traversed
	 * until Duktape hits a sanity limit.
	 */

	duk_get_prop_string(ctx, 0, "nonexistent");
	printf("obj0.foo=%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* NOTE: there are much more extensive prototype loop tests in
	 * test-dev-prototype-loop.c
	 */

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_loop);
}
