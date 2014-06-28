/*===
funcidx=0, argcount=0 -> result=0
funcidx=0, argcount=1 -> result=0
funcidx=0, argcount=2 -> result=0
funcidx=0, argcount=3 -> result=0
funcidx=0, argcount=4 -> result=0
funcidx=1, argcount=0 -> result=0
funcidx=1, argcount=1 -> result=1
funcidx=1, argcount=2 -> result=1
funcidx=1, argcount=3 -> result=1
funcidx=1, argcount=4 -> result=1
funcidx=2, argcount=0 -> result=0
funcidx=2, argcount=1 -> result=1
funcidx=2, argcount=2 -> result=3
funcidx=2, argcount=3 -> result=3
funcidx=2, argcount=4 -> result=3
funcidx=3, argcount=0 -> result=0
funcidx=3, argcount=1 -> result=1
funcidx=3, argcount=2 -> result=3
funcidx=3, argcount=3 -> result=6
funcidx=3, argcount=4 -> result=10
top after calling my_zero_ret: 1, retval='undefined'
top after calling my_neg_ret: 1, rc=1, retval='Error: unknown error (rc -1)'
top after calling my_type_error_ret: 1, rc=1, retval='TypeError: type error (rc -105)'
===*/

static duk_ret_t my_int_adder(duk_context *ctx) {
	duk_idx_t i, n;
	duk_int_t res = 0;

	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		res += duk_to_int(ctx, i);
	}

	duk_push_int(ctx, res);
	return 1;
}

static duk_ret_t my_zero_ret(duk_context *ctx) {
	duk_push_int(ctx, 123);  /* ignored */
	return 0;
}

static duk_ret_t my_neg_ret(duk_context *ctx) {
	duk_push_int(ctx, 123);  /* ignored */
	return -1;
}

static duk_ret_t my_type_error_ret(duk_context *ctx) {
	duk_push_int(ctx, 123);  /* ignored */
	return DUK_RET_TYPE_ERROR;
}

void test(duk_context *ctx) {
	duk_idx_t i, funcidx, argcount;
	duk_ret_t rc;

	/* test C function arg count variants */

	duk_push_c_function(ctx, my_int_adder, 0);              /* [0] = c func with 0 args */
	duk_push_c_function(ctx, my_int_adder, 1);              /* [1] = c func with 1 arg */
	duk_push_c_function(ctx, my_int_adder, 2);              /* [2] = c func with 2 args */
	duk_push_c_function(ctx, my_int_adder, DUK_VARARGS);    /* [3] = c func with varargs */

	for (funcidx = 0; funcidx < 4; funcidx++) {
		for (argcount = 0; argcount < 5; argcount++) {
			duk_dup(ctx, funcidx);
			for (i = 0; i < argcount; i++) {
				duk_push_int(ctx, i + 1);   /* 1, 2, 3, ... */
			}

			/* [ ... func <args> ] */
			duk_call(ctx, argcount);

			printf("funcidx=%ld, argcount=%ld -> result=%ld\n",
			       (long) funcidx, (long) argcount, (long) duk_to_int(ctx, -1));
			duk_pop(ctx);
		}
	}

	/* test C function return value 0 and negative */

	duk_set_top(ctx, 0);
	duk_push_c_function(ctx, my_zero_ret, 0);
	duk_call(ctx, 0);
	printf("top after calling my_zero_ret: %ld, retval='%s'\n",
	       (long) duk_get_top(ctx), duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_set_top(ctx, 0);
	duk_push_c_function(ctx, my_neg_ret, 0);
	rc = duk_pcall(ctx, 0);
	printf("top after calling my_neg_ret: %ld, rc=%d, retval='%s'\n",
	       (long) duk_get_top(ctx), (int) rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_set_top(ctx, 0);
	duk_push_c_function(ctx, my_type_error_ret, 0);
	rc = duk_pcall(ctx, 0);
	printf("top after calling my_type_error_ret: %ld, rc=%d, retval='%s'\n",
	       (long) duk_get_top(ctx), (int) rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}
