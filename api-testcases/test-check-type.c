/*===
stack[0] --> DUK_TYPE_NUMBER=0 DUK_TYPE_NONE=0
stack[1] --> DUK_TYPE_NUMBER=0 DUK_TYPE_NONE=0
stack[2] --> DUK_TYPE_NUMBER=0 DUK_TYPE_NONE=0
stack[3] --> DUK_TYPE_NUMBER=0 DUK_TYPE_NONE=0
stack[4] --> DUK_TYPE_NUMBER=1 DUK_TYPE_NONE=0
stack[5] --> DUK_TYPE_NUMBER=0 DUK_TYPE_NONE=0
stack[6] --> DUK_TYPE_NUMBER=0 DUK_TYPE_NONE=0
stack[7] --> DUK_TYPE_NUMBER=0 DUK_TYPE_NONE=0
stack[8] --> DUK_TYPE_NUMBER=0 DUK_TYPE_NONE=0
stack[9] --> DUK_TYPE_NUMBER=0 DUK_TYPE_NONE=0
stack[10] --> DUK_TYPE_NUMBER=0 DUK_TYPE_NONE=0
stack[11] --> DUK_TYPE_NUMBER=0 DUK_TYPE_NONE=0
stack[12] --> DUK_TYPE_NUMBER=0 DUK_TYPE_NONE=1
===*/

int my_c_func(duk_context *ctx) {
	return 0;
}

void test(duk_context *ctx) {
	int i, n;

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_boolean(ctx, 0);
	duk_push_boolean(ctx, 123);
	duk_push_number(ctx, 234);
	duk_push_string(ctx, "foo");
	duk_push_object(ctx);
	duk_push_array(ctx);
	duk_push_c_function(ctx, my_c_func, DUK_VARARGS);
	duk_push_fixed_buffer(ctx, 1024);
	duk_push_dynamic_buffer(ctx, 1024);
	duk_push_pointer(ctx, (void *) 0xdeadbeef);

	n = duk_get_top(ctx);
	for (i = 0; i < n + 1; i++) {  /* end on invalid index on purpose */
		printf("stack[%d] --> DUK_TYPE_NUMBER=%d DUK_TYPE_NONE=%d\n",
		       i, duk_check_type(ctx, i, DUK_TYPE_NUMBER), duk_check_type(ctx, i, DUK_TYPE_NONE));
	}
}

