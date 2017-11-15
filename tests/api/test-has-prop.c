/*===
*** test_hasprop_a (duk_safe_call)
obj.foo -> rc=1
obj.nonexistent -> rc=0
obj[123] -> rc=1
arr.nonexistent -> rc=0
arr[2] -> rc=1
arr.length -> rc=1
final top: 3
==> rc=0, result='undefined'
*** test_hasprop_b (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_hasprop_c (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_hasprop_d (duk_safe_call)
==> rc=1, result='TypeError: invalid base value'
*** test_hasprop_e (duk_safe_call)
==> rc=1, result='TypeError: invalid base value'
*** test_haspropstring_a (duk_safe_call)
obj.foo -> rc=1
obj.nonexistent -> rc=0
obj['123'] -> rc=1
arr.nonexistent -> rc=0
arr['2'] -> rc=1
arr.length -> rc=1
final top: 3
==> rc=0, result='undefined'
*** test_haspropstring_b (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_haspropstring_c (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_haspropindex_a (duk_safe_call)
obj[31337] -> rc=0
obj[123] -> rc=1
arr[31337] -> rc=0
arr[2] -> rc=1
final top: 3
==> rc=0, result='undefined'
*** test_haspropindex_b (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_haspropindex_c (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_hasproplstring_a (duk_safe_call)
obj.foo -> rc=1
obj.nonexistent -> rc=0
obj.nul<NUL>key -> rc=1
final top: 3
==> rc=0, result='undefined'
*** test_hasproplstring_b (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_hasproplstring_c (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_haspropliteral_a (duk_safe_call)
obj.foo -> rc=1
obj.nonexistent -> rc=0
obj['123'] -> rc=1
arr.nonexistent -> rc=0
arr['2'] -> rc=1
arr.length -> rc=1
final top: 3
==> rc=0, result='undefined'
*** test_haspropliteral_b (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_haspropliteral_c (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_haspropheapptr_a (duk_safe_call)
obj.foo -> rc=1
obj.nonexistent -> rc=0
obj.undefined -> rc=1
final top: 3
==> rc=0, result='undefined'
*** test_haspropheapptr_b (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_haspropheapptr_c (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_haspropheapptr_d (duk_safe_call)
toString() called
obj.foo -> rc=1
final top: 3
==> rc=0, result='undefined'
===*/

static void prep(duk_context *ctx) {
	duk_set_top(ctx, 0);

	/* 0: object with both string and number keys */
	duk_push_string(ctx, "{\"foo\": \"fooval\", \"bar\": \"barval\", \"123\": \"123val\", \"nul\\u0000key\": \"nulval\", \"undefined\": \"undefinedval\"}");
	(void) duk_json_decode(ctx, -1);

	/* 1: array with 3 elements */
	duk_push_string(ctx, "[ \"foo\", \"bar\", \"quux\" ]");
	(void) duk_json_decode(ctx, -1);

	/* 2: plain string */
	duk_push_string(ctx, "test_string");
}

/* duk_has_prop(), success cases */
static duk_ret_t test_hasprop_a(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	duk_push_string(ctx, "foo");
	rc = duk_has_prop(ctx, 0);
	printf("obj.foo -> rc=%d\n", (int) rc);

	duk_push_string(ctx, "nonexistent");
	rc = duk_has_prop(ctx, 0);
	printf("obj.nonexistent -> rc=%d\n", (int) rc);

	duk_push_int(ctx, 123);
	rc = duk_has_prop(ctx, 0);
	printf("obj[123] -> rc=%d\n", (int) rc);

	duk_push_string(ctx, "nonexistent");
	rc = duk_has_prop(ctx, 1);
	printf("arr.nonexistent -> rc=%d\n", (int) rc);

	duk_push_int(ctx, 2);
	rc = duk_has_prop(ctx, 1);
	printf("arr[2] -> rc=%d\n", (int) rc);

	duk_push_string(ctx, "length");
	rc = duk_has_prop(ctx, 1);
	printf("arr.length -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop(), invalid index */
static duk_ret_t test_hasprop_b(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	duk_push_string(ctx, "foo");
	rc = duk_has_prop(ctx, 234);
	printf("obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop(), DUK_INVALID_INDEX */
static duk_ret_t test_hasprop_c(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	duk_push_string(ctx, "foo");
	rc = duk_has_prop(ctx, DUK_INVALID_INDEX);
	printf("obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop(), not an object */
static duk_ret_t test_hasprop_d(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	duk_set_top(ctx, 0);

	duk_push_string(ctx, "test");
	duk_push_string(ctx, "3");
	rc = duk_has_prop(ctx, -2);
	printf("'test'['3'] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop(), not an object */
static duk_ret_t test_hasprop_e(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	duk_set_top(ctx, 0);

	duk_push_null(ctx);
	duk_push_string(ctx, "foo");
	rc = duk_has_prop(ctx, -2);
	printf("null.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_string(), success cases */
static duk_ret_t test_haspropstring_a(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	rc = duk_has_prop_string(ctx, 0, "foo");
	printf("obj.foo -> rc=%d\n", (int) rc);

	rc = duk_has_prop_string(ctx, 0, "nonexistent");
	printf("obj.nonexistent -> rc=%d\n", (int) rc);

	rc = duk_has_prop_string(ctx, 0, "123");
	printf("obj['123'] -> rc=%d\n", (int) rc);

	rc = duk_has_prop_string(ctx, 1, "nonexistent");
	printf("arr.nonexistent -> rc=%d\n", (int) rc);

	rc = duk_has_prop_string(ctx, 1, "2");
	printf("arr['2'] -> rc=%d\n", (int) rc);

	rc = duk_has_prop_string(ctx, 1, "length");
	printf("arr.length -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_string(), invalid index */
static duk_ret_t test_haspropstring_b(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	rc = duk_has_prop_string(ctx, 234, "foo");
	printf("obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_string(), DUK_INVALID_INDEX */
static duk_ret_t test_haspropstring_c(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	rc = duk_has_prop_string(ctx, DUK_INVALID_INDEX, "foo");
	printf("obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_index(), success cases */
static duk_ret_t test_haspropindex_a(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	rc = duk_has_prop_index(ctx, 0, 31337);
	printf("obj[31337] -> rc=%d\n", (int) rc);

	rc = duk_has_prop_index(ctx, 0, 123);
	printf("obj[123] -> rc=%d\n", (int) rc);

	rc = duk_has_prop_index(ctx, 1, 31337);
	printf("arr[31337] -> rc=%d\n", (int) rc);

	rc = duk_has_prop_index(ctx, 1, 2);
	printf("arr[2] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_index(), invalid index */
static duk_ret_t test_haspropindex_b(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	rc = duk_has_prop_index(ctx, 234, 123);
	printf("obj[123] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_index(), DUK_INVALID_INDEX */
static duk_ret_t test_haspropindex_c(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	rc = duk_has_prop_index(ctx, DUK_INVALID_INDEX, 123);
	printf("obj[123] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_lstring(), success cases */
static duk_ret_t test_hasproplstring_a(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	rc = duk_has_prop_lstring(ctx, 0, "foox", 3);
	printf("obj.foo -> rc=%d\n", (int) rc);

	rc = duk_has_prop_lstring(ctx, 0, "nonexistent", 11);
	printf("obj.nonexistent -> rc=%d\n", (int) rc);

	rc = duk_has_prop_lstring(ctx, 0, "nul" "\x00" "keyx", 7);
	printf("obj.nul<NUL>key -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_lstring(), invalid index */
static duk_ret_t test_hasproplstring_b(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	rc = duk_has_prop_lstring(ctx, 234, "foox", 3);
	printf("obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_lstring(), DUK_INVALID_INDEX */
static duk_ret_t test_hasproplstring_c(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	rc = duk_has_prop_lstring(ctx, DUK_INVALID_INDEX, "foox", 3);
	printf("obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_literal(), success cases */
static duk_ret_t test_haspropliteral_a(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	rc = duk_has_prop_literal(ctx, 0, "foo");
	printf("obj.foo -> rc=%d\n", (int) rc);

	rc = duk_has_prop_literal(ctx, 0, "nonexistent");
	printf("obj.nonexistent -> rc=%d\n", (int) rc);

	rc = duk_has_prop_literal(ctx, 0, "123");
	printf("obj['123'] -> rc=%d\n", (int) rc);

	rc = duk_has_prop_literal(ctx, 1, "nonexistent");
	printf("arr.nonexistent -> rc=%d\n", (int) rc);

	rc = duk_has_prop_literal(ctx, 1, "2");
	printf("arr['2'] -> rc=%d\n", (int) rc);

	rc = duk_has_prop_literal(ctx, 1, "length");
	printf("arr.length -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_literal(), invalid index */
static duk_ret_t test_haspropliteral_b(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	rc = duk_has_prop_literal(ctx, 234, "foo");
	printf("obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_literal(), DUK_INVALID_INDEX */
static duk_ret_t test_haspropliteral_c(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	rc = duk_has_prop_literal(ctx, DUK_INVALID_INDEX, "foo");
	printf("obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
/* duk_has_prop_heapptr(), success cases */
static duk_ret_t test_haspropheapptr_a(duk_context *ctx, void *udata) {
	duk_ret_t rc;
	void *ptr;

	(void) udata;

	prep(ctx);

	duk_push_string(ctx, "foo");
	ptr = duk_require_heapptr(ctx, -1);
	rc = duk_has_prop_heapptr(ctx, 0, ptr);
	printf("obj.foo -> rc=%d\n", (int) rc);
	duk_pop(ctx);

	duk_push_string(ctx, "nonexistent");
	ptr = duk_require_heapptr(ctx, -1);
	rc = duk_has_prop_heapptr(ctx, 0, ptr);
	printf("obj.nonexistent -> rc=%d\n", (int) rc);
	duk_pop(ctx);

	ptr = NULL;
	rc = duk_has_prop_heapptr(ctx, 0, ptr);
	printf("obj.undefined -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_heapptr(), invalid index */
static duk_ret_t test_haspropheapptr_b(duk_context *ctx, void *udata) {
	duk_ret_t rc;
	void *ptr;

	(void) udata;

	prep(ctx);

	duk_push_string(ctx, "foo");
	ptr = duk_require_heapptr(ctx, -1);
	rc = duk_has_prop_heapptr(ctx, 234, ptr);
	printf("obj.foo -> rc=%d\n", (int) rc);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_lstring(), DUK_INVALID_INDEX */
static duk_ret_t test_haspropheapptr_c(duk_context *ctx, void *udata) {
	duk_ret_t rc;
	void *ptr;

	(void) udata;

	prep(ctx);

	duk_push_string(ctx, "foo");
	ptr = duk_require_heapptr(ctx, -1);
	rc = duk_has_prop_heapptr(ctx, DUK_INVALID_INDEX, ptr);
	printf("obj.foo -> rc=%d\n", (int) rc);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_lstring(), non-string heapptr */
static duk_ret_t test_haspropheapptr_d(duk_context *ctx, void *udata) {
	duk_ret_t rc;
	void *ptr;

	(void) udata;

	prep(ctx);

	duk_eval_string(ctx, "({ toString: function () { print('toString() called'); return 'foo'; } })");
	ptr = duk_require_heapptr(ctx, -1);
	rc = duk_has_prop_heapptr(ctx, 0, ptr);
	printf("obj.foo -> rc=%d\n", (int) rc);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_hasprop_a);
	TEST_SAFE_CALL(test_hasprop_b);
	TEST_SAFE_CALL(test_hasprop_c);
	TEST_SAFE_CALL(test_hasprop_d);
	TEST_SAFE_CALL(test_hasprop_e);

	TEST_SAFE_CALL(test_haspropstring_a);
	TEST_SAFE_CALL(test_haspropstring_b);
	TEST_SAFE_CALL(test_haspropstring_c);

	TEST_SAFE_CALL(test_haspropindex_a);
	TEST_SAFE_CALL(test_haspropindex_b);
	TEST_SAFE_CALL(test_haspropindex_c);

	TEST_SAFE_CALL(test_hasproplstring_a);
	TEST_SAFE_CALL(test_hasproplstring_b);
	TEST_SAFE_CALL(test_hasproplstring_c);

	TEST_SAFE_CALL(test_haspropliteral_a);
	TEST_SAFE_CALL(test_haspropliteral_b);
	TEST_SAFE_CALL(test_haspropliteral_c);

	TEST_SAFE_CALL(test_haspropheapptr_a);
	TEST_SAFE_CALL(test_haspropheapptr_b);
	TEST_SAFE_CALL(test_haspropheapptr_c);
	TEST_SAFE_CALL(test_haspropheapptr_d);
}
