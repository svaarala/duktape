/*
 *  Plain buffer changes (2.x)
 */

/*===
*** test_basic (duk_safe_call)
duk_is_buffer: 1
duk_is_primitive: 0
duk_is_object_coercible: 1
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	unsigned char *buf;

	(void) udata;

	buf = (unsigned char *) duk_push_fixed_buffer(ctx, 16);
	(void) buf;

	printf("duk_is_buffer: %ld\n", (long) duk_is_buffer(ctx, -1));
	printf("duk_is_primitive: %ld\n", (long) duk_is_primitive(ctx, -1));
	printf("duk_is_object_coercible: %ld\n", (long) duk_is_object_coercible(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_buffer_to_string (duk_safe_call)
duk_buffer_to_string: 'abcdefghijklmnop'
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_buffer_to_string(duk_context *ctx, void *udata) {
	unsigned char *buf;
	int i;

	(void) udata;

	buf = (unsigned char *) duk_push_fixed_buffer(ctx, 16);
	for (i = 0; i < 16; i++) {
		buf[i] = 0x61 + i;
	}

	/* duk_buffer_to_string() creates a string which uses the bytes as is
	 * for the string internal representation.
	 */
	printf("duk_buffer_to_string: '%s'\n", duk_buffer_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_to_buffer (duk_safe_call)
length: 7
[0] = 102
[1] = 111
[2] = 111
[3] = 255
[4] = 98
[5] = 97
[6] = 114
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_to_buffer(duk_context *ctx, void *udata) {
	void *ptr;
	duk_size_t i, len;
	unsigned char *buf;

	(void) udata;

	duk_push_string(ctx, "foo\xff" "bar");

	ptr = duk_to_buffer(ctx, -1, &len);
	printf("length: %ld\n", (long) len);
	buf = (unsigned char *) ptr;

	for (i = 0; i < len; i++) {
		printf("[%ld] = %ld\n", (long) i, (long) buf[i]);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_to_string (duk_safe_call)
duk_to_string: '[object Uint8Array]'
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_to_string(duk_context *ctx, void *udata) {
	unsigned char *buf;
	int i;

	(void) udata;

	buf = (unsigned char *) duk_push_fixed_buffer(ctx, 16);
	for (i = 0; i < 16; i++) {
		buf[i] = 0x61 + i;
	}

	/* duk_to_string() coerces the buffer to a string, which usually
	 * results in [object Uint8Array], just like for an Uint8Array
	 * object.  This is a change from Duktape 1.x behavior; in 1.x
	 * a plain buffer would be coerced to a string with the buffer
	 * bytes (like duk_buffer_to_string()).
	 */
	printf("duk_to_string: '%s'\n", duk_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_enum (duk_safe_call)
flag index: 0, top: 2
- 0: 100
- 1: 101
- 2: 102
- 3: 103
- 4: 104
- 5: 105
- 6: 106
- 7: 107
- 8: 108
- 9: 109
- 10: 110
- 11: 111
- 12: 112
- 13: 113
- 14: 114
- 15: 115
flag index: 1, top: 2
- 0: 100
- 1: 101
- 2: 102
- 3: 103
- 4: 104
- 5: 105
- 6: 106
- 7: 107
- 8: 108
- 9: 109
- 10: 110
- 11: 111
- 12: 112
- 13: 113
- 14: 114
- 15: 115
flag index: 2, top: 2
- 0: 100
- 1: 101
- 2: 102
- 3: 103
- 4: 104
- 5: 105
- 6: 106
- 7: 107
- 8: 108
- 9: 109
- 10: 110
- 11: 111
- 12: 112
- 13: 113
- 14: 114
- 15: 115
- length: 16
- constructor: function Uint8Array() { [native code] }
- BYTES_PER_ELEMENT: 1
- byteLength: 16
- byteOffset: 0
- buffer: [object ArrayBuffer]
- set: function set() { [native code] }
- subarray: function subarray() { [native code] }
- __proto__: [object Object]
- toString: function toString() { [native code] }
- toLocaleString: function toLocaleString() { [native code] }
- valueOf: function valueOf() { [native code] }
- hasOwnProperty: function hasOwnProperty() { [native code] }
- isPrototypeOf: function isPrototypeOf() { [native code] }
- propertyIsEnumerable: function propertyIsEnumerable() { [native code] }
- __defineGetter__: function __defineGetter__() { [native code] }
- __defineSetter__: function __defineSetter__() { [native code] }
- __lookupGetter__: function __lookupGetter__() { [native code] }
- __lookupSetter__: function __lookupSetter__() { [native code] }
flag index: 3, top: 2
- 0: 100
- 1: 101
- 2: 102
- 3: 103
- 4: 104
- 5: 105
- 6: 106
- 7: 107
- 8: 108
- 9: 109
- 10: 110
- 11: 111
- 12: 112
- 13: 113
- 14: 114
- 15: 115
- length: 16
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_enum(duk_context *ctx, void *udata) {
	char *buf;
	duk_uint_t flags[] = {
		0,
		DUK_ENUM_OWN_PROPERTIES_ONLY,
		DUK_ENUM_INCLUDE_NONENUMERABLE,
		DUK_ENUM_OWN_PROPERTIES_ONLY | DUK_ENUM_INCLUDE_NONENUMERABLE
	};
	int i;

	(void) udata;

	buf = (char *) duk_push_fixed_buffer(ctx, 16);
	for (i = 0; i < 16; i++) {
		buf[i] = 100 + i;
	}

	for (i = 0; i < sizeof(flags) / sizeof(duk_uint_t); i++) {
		duk_enum(ctx, -1, flags[i]);
		printf("flag index: %ld, top: %ld\n", (long) i, (long) duk_get_top(ctx));
		while (duk_next(ctx, -1, 1)) {
			printf("- %s: %s\n", duk_to_string(ctx, -2), duk_to_string(ctx, -1));
			duk_pop_2(ctx);
		}
		duk_pop(ctx);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_buffer_to_string);
	TEST_SAFE_CALL(test_to_buffer);
	TEST_SAFE_CALL(test_to_string);
	TEST_SAFE_CALL(test_enum);
}
