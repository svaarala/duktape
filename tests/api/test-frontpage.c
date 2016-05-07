/*
 *  Test code on the front page.
 */

/*===
1+2=3
2+3=5
===*/

static duk_ret_t native_print(duk_context *ctx) {
	printf("%s\n", duk_to_string(ctx, 0));
	return 0;
}

static duk_ret_t native_adder(duk_context *ctx) {
	duk_idx_t i, n;
	double res = 0.0;

	n = duk_get_top(ctx);  /* number of args */
	for (i = 0; i < n; i++) {
		res += duk_to_number(ctx, i);
	}

	duk_push_number(ctx, res);
	return 1;
}

void test(duk_context *ctx) {
	duk_eval_string(ctx, "1+2");
	printf("1+2=%d\n", (int) duk_get_int(ctx, -1));
	duk_pop(ctx);

	duk_push_c_function(ctx, native_print, 1);
	duk_put_global_string(ctx, "print");
	duk_push_c_function(ctx, native_adder, DUK_VARARGS);
	duk_put_global_string(ctx, "adder");

	duk_eval_string_noresult(ctx, "print('2+3=' + adder(2, 3));");
}
