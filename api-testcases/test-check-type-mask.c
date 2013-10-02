/*===
stack[0] --> mask=0
stack[1] --> mask=0
stack[2] --> mask=0
stack[3] --> mask=0
stack[4] --> mask=1
stack[5] --> mask=1
stack[6] --> mask=1
stack[7] --> mask=1
stack[8] --> mask=1
stack[9] --> mask=0
stack[10] --> mask=0
stack[11] --> mask=0
stack[12] --> mask=0
===*/

int my_c_func(duk_context *ctx) {
	return 0;
}

void test(duk_context *ctx) {
	int mask = DUK_TYPE_MASK_STRING |
	           DUK_TYPE_MASK_NUMBER |
	           DUK_TYPE_MASK_OBJECT;
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
		printf("stack[%d] --> mask=%d\n", i, duk_check_type_mask(ctx, i, mask));
	}
}

