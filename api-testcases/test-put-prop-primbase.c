/*===
*** test_put
put rc=0
final top: 0
rc=0, result='undefined'
*** test_put (wrapped)
rc=1, result='TypeError: non-object base reference'
===*/

int test_put(duk_context *ctx) {
	int rc;

	/* In Ecmascript, '(0).foo = "bar"' should work and evaluate to "bar"
	 * in non-strict mode, but cause an error to be thrown in strict mode
	 * (E5.1, Section 8.7.2, special [[Put]] variant, step 7.
	 */

	duk_push_int(ctx, 0);
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	rc = duk_put_prop(ctx, -3);

	printf("put rc=%d\n", rc);

	printf("final top: %d\n", rc);
	return 0;
}

/* execute test outside of a Duktape/C activation (= non-strict mode) */
#define  TEST(func)  do { \
		printf("*** %s\n", #func); \
		rc = duk_safe_call(ctx, (func), 0, 1, DUK_INVALID_INDEX); \
		printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1)); \
		duk_pop(ctx); \
	} while(0)

/* execute test inside of a Duktape/C activation (= strict mode) */
#define  TESTWRAPPED(func)  do { \
		printf("*** %s (wrapped)\n", #func); \
		duk_push_new_c_function(ctx, (func), 0); \
		rc = duk_pcall(ctx, 0, DUK_INVALID_INDEX); \
		printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1)); \
		duk_pop(ctx); \
	} while(0)

void test(duk_context *ctx) {
	int rc;

	TEST(test_put);
	TESTWRAPPED(test_put);
}
