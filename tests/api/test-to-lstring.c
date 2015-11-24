/*===
*** test_1 (duk_safe_call)
top: 20
index 0, string: 'undefined', length 9
index 0, string: 'undefined'
index 1, string: 'null', length 4
index 1, string: 'null'
index 2, string: 'true', length 4
index 2, string: 'true'
index 3, string: 'false', length 5
index 3, string: 'false'
index 4, string: '1', length 1
index 4, string: '1'
index 5, string: '-123.456', length 8
index 5, string: '-123.456'
index 6, string: 'NaN', length 3
index 6, string: 'NaN'
index 7, string: 'Infinity', length 8
index 7, string: 'Infinity'
index 8, string: '-Infinity', length 9
index 8, string: '-Infinity'
index 9, string: '', length 0
index 9, string: ''
index 10, string: 'foo', length 3
index 10, string: 'foo'
index 11, string: 'foo\x00bar', length 7
index 11, string: 'foo'
index 12, string: '[object Object]', length 15
index 12, string: '[object Object]'
index 13, string: '[object Thread]', length 15
index 13, string: '[object Thread]'
index 14, string: '', length 0
index 14, string: ''
index 15, string: 'abcdefghijklmnop', length 16
index 15, string: 'abcdefghijklmnop'
index 16, string: '', length 0
index 16, string: ''
index 17, string: 'abcdefghijklmnop', length 16
index 17, string: 'abcdefghijklmnop'
index 18, string: 'null', length 4
index 18, string: 'null'
index 19, string: '0xdeadbeef', length 10
index 19, string: '0xdeadbeef'
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='Error: invalid stack index 3'
*** test_3 (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_int_t i, j, n;
	char *ptr;

	duk_set_top(ctx, 0);
	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_int(ctx, 1);
	duk_push_number(ctx, -123.456);
	duk_push_nan(ctx);
	duk_push_number(ctx, INFINITY);
	duk_push_number(ctx, -INFINITY);
	duk_push_string(ctx, "");
	duk_push_string(ctx, "foo");
	duk_push_lstring(ctx, "foo\0bar", 7);
	duk_push_object(ctx);
	duk_push_thread(ctx);
	(void) duk_push_fixed_buffer(ctx, 0);
	ptr = (char *) duk_push_fixed_buffer(ctx, 16);
	for (i = 0; i < 16; i++) {
		ptr[i] = (char) ('a' + i);
	}
	(void) duk_push_dynamic_buffer(ctx, 0);
	ptr = (char *) duk_push_dynamic_buffer(ctx, 16);
	for (i = 0; i < 16; i++) {
		ptr[i] = (char) ('a' + i);
	}
	duk_push_pointer(ctx, (void *) NULL);
	duk_push_pointer(ctx, (void *) 0xdeadbeef);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i < n; i++) {
		const unsigned char *p;
		duk_size_t sz;

		duk_dup(ctx, i);
		sz = (duk_size_t) 0xdeadbeef;
		p = (const unsigned char *) duk_to_lstring(ctx, -1, &sz);
		printf("index %ld, string: '", (long) i);
		for (j = 0; j < sz; j++) {
			if (p[j] >= 0x20 && p[j] <= 0x7e) {
				printf("%c", (int) p[j]);
			} else {
				printf("\\x%02x", (int) p[j]);
			}
		}
		printf("', length %lu\n", (unsigned long) sz);
		duk_pop(ctx);

		duk_dup(ctx, i);
		sz = (duk_size_t) 0xdeadbeef;
		p = (const unsigned char *) duk_to_lstring(ctx, -1, NULL);
		printf("index %ld, string: '%s'\n", (long) i, (const char *) p);
		duk_pop(ctx);
	}

	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	const char *p;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	p = duk_to_lstring(ctx, 3, &sz);
	printf("index 3: %p, %lu\n", (void *) p, (unsigned long) sz);  /* ok to print, not shown in expected case */
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	const char *p;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	p = duk_to_lstring(ctx, DUK_INVALID_INDEX, &sz);
	printf("index DUK_INVALID_INDEX: %p, %ld\n", (void *) p, (unsigned long) sz);  /* ok to print, not shown in expected case */
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
}
