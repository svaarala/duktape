/*
 *  Test code on the front page.
 */

/*===
2+3 is 5
===*/

int my_adder(duk_context *ctx) {
	int i;
	int n = duk_get_top(ctx);  /* number of args */
	double res = 0.0;

	for (i = 0; i < n; i++) {
		res += duk_to_number(ctx, i);
	}

	duk_push_number(ctx, res);
	return 1;
}

void test(duk_context *ctx) {
	duk_push_global_object(ctx);
	duk_push_new_c_function(ctx, my_adder, DUK_VARARGS);
	duk_put_prop_string(ctx, -2, "myAdder");
	duk_pop(ctx);  /* pop global */

	duk_push_string(ctx, "print('2+3 is', myAdder(2, 3));");
	duk_eval(ctx);
	duk_pop(ctx);  /* pop eval result */
}

