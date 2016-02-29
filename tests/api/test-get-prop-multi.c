/*
 *  duk_get_prop_multi()
 */

static duk_ret_t test_basic(duk_context *ctx) {
	duk_int_t my_int1, my_int2;
	duk_uint_t my_uint1, my_uint2;
	duk_double_t my_num1, my_num2;
	duk_bool_t my_bool1, my_bool2;
	const char *my_str1;
	const char *my_str2;
	void *my_ptr1;
	void *my_ptr2;
	void *my_buf1;
	duk_size_t my_buflen1;
	void *my_buf2;
	duk_size_t my_buflen2;

	duk_eval_string(ctx, "({ undefinedValue: void 0, \n"
	                     "   nullValue: null, \n"
	                     "   falseValue: false, \n"
	                     "   trueValue: true, \n"
	                     "   integerValue: -321, \n"
	                     "   numberValue: 123.4, \n"
	                     "   stringValue: 'foo bar', \n"
	                     "   pointerValue: Duktape.Pointer('foo'), \n"
	                     "   bufferValue: Duktape.dec('hex', 'deadbeef') })");

	my_int1 = 12345;  /* dummy values to ensure they get overwritten */
	my_int2 = 12345;
	my_uint1 = 12345;
	my_uint2 = 12345;
	my_num1 = 12345.0;
	my_num2 = 12345.0;
	my_bool1 = 123;
	my_bool2 = 123;

	printf("top before: %ld\n", (long) duk_get_top(ctx));

	duk_get_prop_multi(ctx, -1,
		"stringValue:v "  /* push to value stack */
		"integerValue:d numberValue:d "
		"integerValue:u numberValue:u "
		"integerValue:n numberValue:n "
		"falseValue:b trueValue:b "
		"integerValue:s stringValue:s "
		"pointerValue:p stringValue:p "
		"bufferValue:x stringValue:x "
		"nullValue:v",
		&my_int1, &my_int2,
		&my_uint1, &my_uint2,
		&my_num1, &my_num2,
		&my_bool1, &my_bool2,
		&my_str1, &my_str2,
		&my_ptr1, &my_ptr2,
		&my_buf1, &my_buflen1, &my_buf2, &my_buflen2);

	printf("top after: %ld\n", (long) duk_get_top(ctx));

	printf("my_int1: %ld\n", (long) my_int1);
	printf("my_int2: %ld\n", (long) my_int2);
	printf("my_uint1: %lu\n", (unsigned long) my_uint1);
	printf("my_uint2: %lu\n", (unsigned long) my_uint2);
	printf("my_num1: %lf\n", (double) my_num1);
	printf("my_num2: %lf\n", (double) my_num2);
	printf("my_bool1: %d\n", (int) my_bool1);
	printf("my_bool2: %d\n", (int) my_bool2);
	printf("my_str1: '%s'\n", my_str1);
	printf("my_str2: '%s'\n", my_str2);
	printf("my_ptr1: %p\n", my_ptr1);
	printf("my_ptr2: %p\n", my_ptr2);
	printf("my_buf1: %p %ld\n", my_buf1, (long) my_buflen1);
	printf("my_buf2: %p %ld\n", my_buf2, (long) my_buflen2);
	printf("value stack -2 is string: %d\n", (int) duk_is_string(ctx, -2));
	printf("value stack -1 is null: %d\n", (int) duk_is_null(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* FIXME: missing property */
/* FIXME: NULL value pointer */
/* FIXME: invalid character */
/* FIXME: missing colon */
/* FIXME: truncation test */

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
