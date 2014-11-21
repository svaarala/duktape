/*
 *  API tests for duk_get_heapptr(), duk_require_heapptr(), duk_push_heapptr().
 */

/*===
*** test_basic (duk_safe_call)
top: 7
idx 0: type 1, duk_get_heapptr() -> NULL
idx 0: type 1, duk_require_heapptr() -> TypeError: unexpected type
top: 7
idx 1: type 2, duk_get_heapptr() -> NULL
idx 1: type 2, duk_require_heapptr() -> TypeError: unexpected type
top: 7
idx 2: type 3, duk_get_heapptr() -> NULL
idx 2: type 3, duk_require_heapptr() -> TypeError: unexpected type
top: 7
idx 3: type 4, duk_get_heapptr() -> NULL
idx 3: type 4, duk_require_heapptr() -> TypeError: unexpected type
top: 7
idx 4: type 5, duk_get_heapptr() -> non-NULL
idx 4: type 5, duk_require_heapptr() -> non-NULL
top: 7
idx 5: type 6, duk_get_heapptr() -> non-NULL
idx 5: type 6, duk_require_heapptr() -> non-NULL
top: 7
idx 6: type 7, duk_get_heapptr() -> non-NULL
idx 6: type 7, duk_require_heapptr() -> non-NULL
top: 7
idx 7: type 0, duk_get_heapptr() -> NULL
idx 7: type 5, duk_require_heapptr() -> Error: invalid index
"test string"
{foo:"bar"}
|deadbeef|
undefined
final top: 0
==> rc=0, result='undefined'
*** test_api_example (duk_safe_call)
obj.foo: bar
final top: 2
==> rc=0, result='undefined'
===*/

static duk_ret_t raw_require_heapptr(duk_context *ctx) {
	duk_idx_t i;
	void *ptr;

	i = duk_require_uint(ctx, -1);
	duk_pop(ctx);

	ptr = duk_require_heapptr(ctx, i);

	printf("idx %ld: type %ld, duk_require_heapptr() -> %s\n",
	       (long) i, (long) duk_get_type(ctx, i), (ptr ? "non-NULL" : "NULL"));

	return 0;
}

static duk_ret_t test_basic(duk_context *ctx) {
	duk_idx_t i, n;
	void *ptr;
	void *p1, *p2, *p3;
	duk_int_t ret;

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_boolean(ctx, 1);
	duk_push_number(ctx, 123.0);
	duk_push_string(ctx, "test string");
	duk_eval_string(ctx, "({ foo: 'bar' })");
	duk_eval_string(ctx, "Duktape.dec('hex', 'deadbeef');");  /* buffer */

	/* Print pointer NULL/non-NULL */

	n = duk_get_top(ctx);
	for (i = 0; i < n + 1; i++) {
		/* Note: access i == n on purpose (invalid index) */

		ptr = duk_get_heapptr(ctx, i);

		printf("top: %ld\n", (long) duk_get_top(ctx));

		printf("idx %ld: type %ld, duk_get_heapptr() -> %s\n",
		       (long) i, (long) duk_get_type(ctx, i), (ptr ? "non-NULL" : "NULL"));

		duk_push_uint(ctx, (duk_uint_t) i);
		ret = duk_safe_call(ctx, raw_require_heapptr, 1 /*nargs*/, 1 /*nrets*/);
		if (ret == DUK_EXEC_SUCCESS) {
			;
		} else {
			printf("idx %ld: type %ld, duk_require_heapptr() -> %s\n",
			       (long) i, (long) duk_get_type(ctx, i), duk_safe_to_string(ctx, -1));
		}
		duk_pop(ctx);
	}

	/* Write the values to the global stash to simulate whatever
	 * reachability mechanisms user code uses.
	 */

	n = duk_get_top(ctx);
	duk_push_global_stash(ctx);
	for (i = 0; i < n; i++) {
		duk_dup(ctx, i);
		duk_put_prop_index(ctx, -2, (duk_uarridx_t) i);
	}

	/* Get borrowed references */

	p1 = duk_get_heapptr(ctx, 4);
	p2 = duk_get_heapptr(ctx, 5);
	p3 = duk_get_heapptr(ctx, 6);

	/* Erase value stack, simulating user code moving on and relying on
	 * stashed values keeping the target values reachable.  Force a GC
	 * for good measure.
	 */

	duk_set_top(ctx, 0);
	duk_gc(ctx, 0);

	/* Push the values back one by one and test that they're intact. */

	duk_eval_string(ctx, "(function (v) { print(Duktape.enc('jx', v)); })");

	duk_dup_top(ctx);
	duk_push_heapptr(ctx, p1);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_dup_top(ctx);
	duk_push_heapptr(ctx, p2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_dup_top(ctx);
	duk_push_heapptr(ctx, p3);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_dup_top(ctx);
	duk_push_heapptr(ctx, NULL);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_pop(ctx);

	/* Done. */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_api_example(duk_context *ctx) {
	void *ptr;

	duk_eval_string(ctx, "({ foo: 'bar' })");
	ptr = duk_get_heapptr(ctx, -1);

	duk_put_global_string(ctx, "ref");
	duk_set_top(ctx, 0);

	duk_push_heapptr(ctx, ptr);
	duk_get_prop_string(ctx, -1, "foo");
	printf("obj.foo: %s\n", duk_safe_to_string(ctx, -1));  /* prints 'bar' */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_api_example);
}
