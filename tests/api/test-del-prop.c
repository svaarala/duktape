/*===
*** test_delprop_a_safecall (duk_safe_call)
delete obj.foo -> rc=1
delete obj.nonexistent -> rc=1
delete obj[123] -> rc=1
delete arr.nonexistent -> rc=1
delete arr[2] -> rc=1
final object: {"bar":"barval","nul\u0000key":"nulval","undefined":"undefinedval"}
final array: ["foo","bar",null]
final top: 3
==> rc=0, result='undefined'
*** test_delprop_b_safecall (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_delprop_b (duk_pcall)
==> rc=1, result='TypeError: not configurable'
*** test_delprop_c_safecall (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_delprop_c (duk_pcall)
==> rc=1, result='TypeError: not configurable'
*** test_delprop_d_safecall (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_delprop_d (duk_pcall)
==> rc=1, result='TypeError: not configurable'
*** test_delprop_e_safecall (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_delprop_e (duk_pcall)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_delprop_f_safecall (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_delprop_f (duk_pcall)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_delprop_g_safecall (duk_safe_call)
==> rc=1, result='TypeError: cannot delete property 'foo' of null'
*** test_delprop_g (duk_pcall)
==> rc=1, result='TypeError: cannot delete property 'foo' of null'
*** test_delpropstring_a_safecall (duk_safe_call)
delete obj.foo -> rc=1
delete obj.nonexistent -> rc=1
delete obj['123'] -> rc=1
delete arr.nonexistent -> rc=1
delete arr['2'] -> rc=1
final object: {"bar":"barval","nul\u0000key":"nulval","undefined":"undefinedval"}
final array: ["foo","bar",null]
final top: 3
==> rc=0, result='undefined'
*** test_delpropstring_b_safecall (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_delpropstring_b (duk_pcall)
==> rc=1, result='TypeError: not configurable'
*** test_delpropstring_c_safecall (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_delpropstring_c (duk_pcall)
==> rc=1, result='TypeError: not configurable'
*** test_delpropstring_d_safecall (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_delpropstring_d (duk_pcall)
==> rc=1, result='TypeError: not configurable'
*** test_delpropstring_e_safecall (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_delpropstring_e (duk_pcall)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_delpropstring_f_safecall (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_delpropstring_f (duk_pcall)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_delpropstring_g_safecall (duk_safe_call)
==> rc=1, result='TypeError: cannot delete property 'foo' of null'
*** test_delpropstring_g (duk_pcall)
==> rc=1, result='TypeError: cannot delete property 'foo' of null'
*** test_delpropindex_a_safecall (duk_safe_call)
delete obj[31337] -> rc=1
delete obj[123] -> rc=1
delete arr[31337] -> rc=1
delete arr[2] -> rc=1
final object: {"foo":"fooval","bar":"barval","nul\u0000key":"nulval","undefined":"undefinedval"}
final array: ["foo","bar",null]
final top: 3
==> rc=0, result='undefined'
*** test_delpropindex_b_safecall (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_delpropindex_b (duk_pcall)
==> rc=1, result='TypeError: not configurable'
*** test_delpropindex_c_safecall (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_delpropindex_c (duk_pcall)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_delpropindex_d_safecall (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_delpropindex_d (duk_pcall)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_delproplstring_a_safecall (duk_safe_call)
delete obj.nul<NUL>key -> rc=1
{"123":"123val","foo":"fooval","bar":"barval","undefined":"undefinedval"}
final top: 3
==> rc=0, result='undefined'
*** test_delproplstring_a (duk_pcall)
delete obj.nul<NUL>key -> rc=1
{"123":"123val","foo":"fooval","bar":"barval","undefined":"undefinedval"}
final top: 3
==> rc=0, result='undefined'
*** test_delpropliteral_a_safecall (duk_safe_call)
delete obj.foo -> rc=1
{"123":"123val","bar":"barval","nul\u0000key":"nulval","undefined":"undefinedval"}
final top: 3
==> rc=0, result='undefined'
*** test_delpropliteral_a (duk_pcall)
delete obj.foo -> rc=1
{"123":"123val","bar":"barval","nul\u0000key":"nulval","undefined":"undefinedval"}
final top: 3
==> rc=0, result='undefined'
*** test_delpropheapptr_a_safecall (duk_safe_call)
delete obj.foo -> rc=1
delete obj.nonexistent -> rc=1
delete obj.undefined -> rc=1
final object: {"123":"123val","bar":"barval","nul\u0000key":"nulval"}
final array: ["foo","bar","quux"]
final top: 3
==> rc=0, result='undefined'
*** test_delpropheapptr_a (duk_pcall)
delete obj.foo -> rc=1
delete obj.nonexistent -> rc=1
delete obj.undefined -> rc=1
final object: {"123":"123val","bar":"barval","nul\u0000key":"nulval"}
final array: ["foo","bar","quux"]
final top: 3
==> rc=0, result='undefined'
*** test_delpropheapptr_b_safecall (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_delpropheapptr_b (duk_pcall)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_delpropheapptr_c_safecall (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_delpropheapptr_c (duk_pcall)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_delpropheapptr_d_safecall (duk_safe_call)
toString() called
delete obj.foo -> rc=1
final top: 3
==> rc=0, result='undefined'
*** test_delpropheapptr_d (duk_pcall)
toString() called
delete obj.foo -> rc=1
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

/* duk_del_prop(), success cases */
static duk_ret_t test_delprop_a_safecall(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	/* existing, configurable */
	duk_push_string(ctx, "foo");
	rc = duk_del_prop(ctx, 0);
	printf("delete obj.foo -> rc=%d\n", (int) rc);

	/* nonexistent */
	duk_push_string(ctx, "nonexistent");
	rc = duk_del_prop(ctx, 0);
	printf("delete obj.nonexistent -> rc=%d\n", (int) rc);

	/* nonexistent */
	duk_push_int(ctx, 123);
	rc = duk_del_prop(ctx, 0);
	printf("delete obj[123] -> rc=%d\n", (int) rc);

	/* nonexistent, array */
	duk_push_string(ctx, "nonexistent");
	rc = duk_del_prop(ctx, 1);
	printf("delete arr.nonexistent -> rc=%d\n", (int) rc);

	/* existing, configurable, array */
	duk_push_int(ctx, 2);
	rc = duk_del_prop(ctx, 1);
	printf("delete arr[2] -> rc=%d\n", (int) rc);

	duk_json_encode(ctx, 0);
	printf("final object: %s\n", duk_to_string(ctx, 0));
	duk_json_encode(ctx, 1);
	printf("final array: %s\n", duk_to_string(ctx, 1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop(), non-configurable property (array 'length' property).
 * Same behavior when called inside/outside of a Duktape/C activation
 * (since Duktape 0.12.0 both cases are considered strict).
 */
static duk_ret_t test_delprop_b(duk_context *ctx) {
	duk_ret_t rc;
	prep(ctx);

	duk_push_string(ctx, "length");
	rc = duk_del_prop(ctx, 1);
	printf("delete arr.length -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delprop_b_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delprop_b(ctx);
}

/* duk_del_prop(), non-configurable virtual property of a plain string.
 * Same behavior when called inside/outside of a Duktape/C activation.
 */
static duk_ret_t test_delprop_c(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	duk_push_int(ctx, 5);
	rc = duk_del_prop(ctx, 2);
	printf("delete 'test_string'[5] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delprop_c_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delprop_c(ctx);
}

/* duk_del_prop(), non-configurable virtual property of a plain string.
 * Same behavior when called inside/outside of a Duktape/C activation.
 */
static duk_ret_t test_delprop_d(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	duk_push_string(ctx, "length");
	rc = duk_del_prop(ctx, 2);
	printf("delete 'test_string'.length -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delprop_d_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delprop_d(ctx);
}

/* duk_del_prop(), invalid index */
static duk_ret_t test_delprop_e(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	duk_push_string(ctx, "foo");
	rc = duk_del_prop(ctx, 234);
	printf("delete obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delprop_e_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delprop_e(ctx);
}

/* duk_del_prop(), DUK_INVALID_INDEX */
static duk_ret_t test_delprop_f(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	duk_push_string(ctx, "foo");
	rc = duk_del_prop(ctx, DUK_INVALID_INDEX);
	printf("delete obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delprop_f_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delprop_f(ctx);
}

/* duk_del_prop(), not object coercible */
static duk_ret_t test_delprop_g(duk_context *ctx) {
	duk_ret_t rc;

	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	duk_push_string(ctx, "foo");
	rc = duk_del_prop(ctx, -2);
	printf("delete null.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delprop_g_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delprop_g(ctx);
}

/* duk_del_prop_string(), success cases */
static duk_ret_t test_delpropstring_a_safecall(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	rc = duk_del_prop_string(ctx, 0, "foo");
	printf("delete obj.foo -> rc=%d\n", (int) rc);

	rc = duk_del_prop_string(ctx, 0, "nonexistent");
	printf("delete obj.nonexistent -> rc=%d\n", (int) rc);

	rc = duk_del_prop_string(ctx, 0, "123");
	printf("delete obj['123'] -> rc=%d\n", (int) rc);

	rc = duk_del_prop_string(ctx, 1, "nonexistent");
	printf("delete arr.nonexistent -> rc=%d\n", (int) rc);

	rc = duk_del_prop_string(ctx, 1, "2");
	printf("delete arr['2'] -> rc=%d\n", (int) rc);

	duk_json_encode(ctx, 0);
	printf("final object: %s\n", duk_to_string(ctx, 0));
	duk_json_encode(ctx, 1);
	printf("final array: %s\n", duk_to_string(ctx, 1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_string(), non-configurable property (array 'length' property).
 * Same behavior when called inside/outside of a Duktape/C activation.
 */
static duk_ret_t test_delpropstring_b(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, 1, "length");
	printf("delete arr.length -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delpropstring_b_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delpropstring_b(ctx);
}

/* duk_del_prop_string(), non-configurable virtual property of a plain string.
 * Same behavior when called inside/outside of a Duktape/C activation.
 */
static duk_ret_t test_delpropstring_c(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, 2, "5");
	printf("delete 'test_string'['5'] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delpropstring_c_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delpropstring_c(ctx);
}

/* duk_del_prop_string(), non-configurable virtual property of a plain string.
 * Same behavior when called inside/outside of a Duktape/C activation.
 */
static duk_ret_t test_delpropstring_d(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, 2, "length");
	printf("delete 'test_string'.length -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delpropstring_d_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delpropstring_d(ctx);
}

/* duk_del_prop_string(), invalid index */
static duk_ret_t test_delpropstring_e(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, 234, "foo");
	printf("delete obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delpropstring_e_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delpropstring_e(ctx);
}

/* duk_del_prop_string(), DUK_INVALID_INDEX */
static duk_ret_t test_delpropstring_f(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, DUK_INVALID_INDEX, "foo");
	printf("delete obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delpropstring_f_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delpropstring_f(ctx);
}

/* duk_del_prop_string(), not object coercible */
static duk_ret_t test_delpropstring_g(duk_context *ctx) {
	duk_ret_t rc;

	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	rc = duk_del_prop_string(ctx, -1, "foo");
	printf("delete null.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delpropstring_g_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delpropstring_g(ctx);
}

/* duk_del_prop_index(), success cases */
static duk_ret_t test_delpropindex_a_safecall(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	prep(ctx);

	rc = duk_del_prop_index(ctx, 0, 31337);
	printf("delete obj[31337] -> rc=%d\n", (int) rc);

	rc = duk_del_prop_index(ctx, 0, 123);
	printf("delete obj[123] -> rc=%d\n", (int) rc);

	rc = duk_del_prop_index(ctx, 1, 31337);
	printf("delete arr[31337] -> rc=%d\n", (int) rc);

	rc = duk_del_prop_index(ctx, 1, 2);
	printf("delete arr[2] -> rc=%d\n", (int) rc);

	duk_json_encode(ctx, 0);
	printf("final object: %s\n", duk_to_string(ctx, 0));
	duk_json_encode(ctx, 1);
	printf("final array: %s\n", duk_to_string(ctx, 1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_index(), non-configurable virtual property of a plain string.
 * Same behavior when called inside/outside of a Duktape/C activation.
 */
static duk_ret_t test_delpropindex_b(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_index(ctx, 2, 5);
	printf("delete 'test_string'[5] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delpropindex_b_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delpropindex_b(ctx);
}

/* duk_del_prop_index(), invalid index */
static duk_ret_t test_delpropindex_c(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_index(ctx, 234, 123);
	printf("delete obj[123] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delpropindex_c_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delpropindex_c(ctx);
}

/* duk_del_prop_index(), DUK_INVALID_INDEX */
static duk_ret_t test_delpropindex_d(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_index(ctx, DUK_INVALID_INDEX, 123);
	printf("delete obj[123] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delpropindex_d_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delpropindex_d(ctx);
}

/* duk_del_prop_lstring(), success case */
static duk_ret_t test_delproplstring_a(duk_context *ctx) {
	duk_ret_t rc;
	prep(ctx);

	rc = duk_del_prop_lstring(ctx, 0, "nul" "\x00" "keyx", 7);
	printf("delete obj.nul<NUL>key -> rc=%d\n", (int) rc);

	duk_json_encode(ctx, 0);
	printf("%s\n", duk_to_string(ctx, 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delproplstring_a_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delproplstring_a(ctx);
}

/* duk_del_prop_literal(), success case */
static duk_ret_t test_delpropliteral_a(duk_context *ctx) {
	duk_ret_t rc;
	prep(ctx);

	rc = duk_del_prop_literal(ctx, 0, "foo");
	printf("delete obj.foo -> rc=%d\n", (int) rc);

	duk_json_encode(ctx, 0);
	printf("%s\n", duk_to_string(ctx, 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delpropliteral_a_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_delpropliteral_a(ctx);
}

/* duk_del_prop_heapptr(), success cases */
static duk_ret_t test_delpropheapptr_a_safecall(duk_context *ctx, void *udata) {
	duk_ret_t rc;
	void *ptr;

	(void) udata;

	prep(ctx);

	duk_push_string(ctx, "foo");
	ptr = duk_require_heapptr(ctx, -1);
	rc = duk_del_prop_heapptr(ctx, 0, ptr);
	printf("delete obj.foo -> rc=%d\n", (int) rc);
	duk_pop(ctx);

	duk_push_string(ctx, "nonexistent");
	ptr = duk_require_heapptr(ctx, -1);
	rc = duk_del_prop_heapptr(ctx, 0, ptr);
	printf("delete obj.nonexistent -> rc=%d\n", (int) rc);
	duk_pop(ctx);

	ptr = NULL;
	rc = duk_del_prop_heapptr(ctx, 0, ptr);
	printf("delete obj.undefined -> rc=%d\n", (int) rc);

	duk_json_encode(ctx, 0);
	printf("final object: %s\n", duk_to_string(ctx, 0));
	duk_json_encode(ctx, 1);
	printf("final array: %s\n", duk_to_string(ctx, 1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delpropheapptr_a(duk_context *ctx) {
	return test_delpropheapptr_a_safecall(ctx, NULL);
}

/* duk_del_prop_heapptr(), invalid index */
static duk_ret_t test_delpropheapptr_b_safecall(duk_context *ctx, void *udata) {
	duk_ret_t rc;
	void *ptr;

	(void) udata;

	prep(ctx);

	duk_push_string(ctx, "foo");
	ptr = duk_require_heapptr(ctx, -1);
	rc = duk_del_prop_heapptr(ctx, 234, ptr);
	printf("delete obj.foo -> rc=%d\n", (int) rc);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delpropheapptr_b(duk_context *ctx) {
	return test_delpropheapptr_b_safecall(ctx, NULL);
}

/* duk_del_prop_heapptr(), DUK_INVALID_INDEX */
static duk_ret_t test_delpropheapptr_c_safecall(duk_context *ctx, void *udata) {
	duk_ret_t rc;
	void *ptr;

	(void) udata;

	prep(ctx);

	duk_push_string(ctx, "foo");
	ptr = duk_require_heapptr(ctx, -1);
	rc = duk_del_prop_heapptr(ctx, DUK_INVALID_INDEX, ptr);
	printf("delete obj.foo -> rc=%d\n", (int) rc);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delpropheapptr_c(duk_context *ctx) {
	return test_delpropheapptr_c_safecall(ctx, NULL);
}

/* duk_del_prop_heapptr(), non-string heapptr */
static duk_ret_t test_delpropheapptr_d_safecall(duk_context *ctx, void *udata) {
	duk_ret_t rc;
	void *ptr;

	(void) udata;

	prep(ctx);

	duk_eval_string(ctx, "({ toString: function () { print('toString() called'); return 'foo'; } })");
	ptr = duk_require_heapptr(ctx, -1);
	rc = duk_del_prop_heapptr(ctx, 0, ptr);
	printf("delete obj.foo -> rc=%d\n", (int) rc);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_delpropheapptr_d(duk_context *ctx) {
	return test_delpropheapptr_d_safecall(ctx, NULL);
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_delprop_a_safecall);
	TEST_SAFE_CALL(test_delprop_b_safecall);
	TEST_PCALL(test_delprop_b);
	TEST_SAFE_CALL(test_delprop_c_safecall);
	TEST_PCALL(test_delprop_c);
	TEST_SAFE_CALL(test_delprop_d_safecall);
	TEST_PCALL(test_delprop_d);
	TEST_SAFE_CALL(test_delprop_e_safecall);
	TEST_PCALL(test_delprop_e);
	TEST_SAFE_CALL(test_delprop_f_safecall);
	TEST_PCALL(test_delprop_f);
	TEST_SAFE_CALL(test_delprop_g_safecall);
	TEST_PCALL(test_delprop_g);

	TEST_SAFE_CALL(test_delpropstring_a_safecall);
	TEST_SAFE_CALL(test_delpropstring_b_safecall);
	TEST_PCALL(test_delpropstring_b);
	TEST_SAFE_CALL(test_delpropstring_c_safecall);
	TEST_PCALL(test_delpropstring_c);
	TEST_SAFE_CALL(test_delpropstring_d_safecall);
	TEST_PCALL(test_delpropstring_d);
	TEST_SAFE_CALL(test_delpropstring_e_safecall);
	TEST_PCALL(test_delpropstring_e);
	TEST_SAFE_CALL(test_delpropstring_f_safecall);
	TEST_PCALL(test_delpropstring_f);
	TEST_SAFE_CALL(test_delpropstring_g_safecall);
	TEST_PCALL(test_delpropstring_g);

	TEST_SAFE_CALL(test_delpropindex_a_safecall);
	TEST_SAFE_CALL(test_delpropindex_b_safecall);
	TEST_PCALL(test_delpropindex_b);
	TEST_SAFE_CALL(test_delpropindex_c_safecall);
	TEST_PCALL(test_delpropindex_c);
	TEST_SAFE_CALL(test_delpropindex_d_safecall);
	TEST_PCALL(test_delpropindex_d);

	TEST_SAFE_CALL(test_delproplstring_a_safecall);
	TEST_PCALL(test_delproplstring_a);

	TEST_SAFE_CALL(test_delpropliteral_a_safecall);
	TEST_PCALL(test_delpropliteral_a);

	TEST_SAFE_CALL(test_delpropheapptr_a_safecall);
	TEST_PCALL(test_delpropheapptr_a);
	TEST_SAFE_CALL(test_delpropheapptr_b_safecall);
	TEST_PCALL(test_delpropheapptr_b);
	TEST_SAFE_CALL(test_delpropheapptr_c_safecall);
	TEST_PCALL(test_delpropheapptr_c);
	TEST_SAFE_CALL(test_delpropheapptr_d_safecall);
	TEST_PCALL(test_delpropheapptr_d);
}
