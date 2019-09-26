/*===
*** test_1 (duk_safe_call)
0: 123
1: 345
2: foo
3: 234
final top: 4
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
pull at 3 ok
0: 123
1: 234
2: 345
3: foo
pull at -1 ok
0: 123
1: 234
2: 345
3: foo
==> rc=1, result='RangeError: invalid stack index 4'
*** test_3 (duk_safe_call)
pull at 0 ok
0: 234
1: 345
2: foo
3: 123
pull at -4 ok
0: 345
1: foo
2: 123
3: 234
==> rc=1, result='RangeError: invalid stack index -5'
*** test_4 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_5 (duk_safe_call)
0: val-0
1: val-1
2: val-2
3: val-3
4: val-4
5: val-5
6: val-6
7: val-7
8: val-8
9: val-9
0: val-1
1: val-2
2: val-3
3: val-4
4: val-5
5: val-6
6: val-7
7: val-8
8: val-9
9: val-0
0: val-1
1: val-2
2: val-3
3: val-5
4: val-6
5: val-7
6: val-8
7: val-9
8: val-0
9: val-4
0: val-1
1: val-2
2: val-3
3: val-5
4: val-6
5: val-7
6: val-9
7: val-0
8: val-4
9: val-8
final top: 10
==> rc=0, result='undefined'
*** test_6 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index 0'
===*/

static void prep(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_push_int(ctx, 345);       /* -> [ 123 234 345 ] */
	duk_push_string(ctx, "foo");  /* -> [ 123 234 345 "foo" ] */
}

static void dump_stack(duk_context *ctx) {
	duk_idx_t i, n;

	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		printf("%ld: %s\n", (long) i, duk_to_string(ctx, i));
	}
}

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	prep(ctx);
	duk_pull(ctx, -3);            /* -> [ 123 345 "foo" 234 ] */

	dump_stack(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx, void *udata) {
	(void) udata;

	prep(ctx);
	duk_pull(ctx, 3);             /* -> [ 123 234 345 "foo" ]  (legal, keep top) */
	printf("pull at 3 ok\n");
	dump_stack(ctx);
	duk_pull(ctx, -1);            /* -> [ 123 234 345 "foo" ]  (legal, keep top) */
	printf("pull at -1 ok\n");
	dump_stack(ctx);
	duk_pull(ctx, 4);             /* (illegal: index too high) */
	printf("pull at 4 ok\n");
	dump_stack(ctx);
	return 0;
}

static duk_ret_t test_3(duk_context *ctx, void *udata) {
	(void) udata;

	prep(ctx);
	duk_pull(ctx, 0);             /* -> [ 234 345 "foo" 123 ]  (legal) */
	printf("pull at 0 ok\n");
	dump_stack(ctx);
	duk_pull(ctx, -4);            /* -> [ 345 "foo" 123 234 ]  (legal) */
	printf("pull at -4 ok\n");
	dump_stack(ctx);
	duk_pull(ctx, -5);            /* (illegal: index too low) */
	printf("pull at -5 ok\n");
	dump_stack(ctx);
	return 0;
}

static duk_ret_t test_4(duk_context *ctx, void *udata) {
	(void) udata;

	prep(ctx);
	duk_pull(ctx, DUK_INVALID_INDEX);  /* (illegal: invalid index) */
	printf("pull at DUK_INVALID_INDEX ok\n");
	dump_stack(ctx);
	return 0;
}

static duk_ret_t test_5(duk_context *ctx, void *udata) {
	duk_idx_t i;

	(void) udata;

	for (i = 0; i < 10; i++) {
		duk_push_sprintf(ctx, "val-%d", (int) i);
	}
	dump_stack(ctx);
	duk_pull(ctx, 0);
	dump_stack(ctx);
	duk_pull(ctx, 3);
	dump_stack(ctx);
	duk_pull(ctx, -4);
	dump_stack(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));

	return 0;
}

static duk_ret_t test_6(duk_context *ctx, void *udata) {
	(void) udata;

	duk_set_top(ctx, 0);
	duk_pull(ctx, 0);
	printf("pull on empty stack\n");
	dump_stack(ctx);
	return 0;
}
void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
	TEST_SAFE_CALL(test_5);
	TEST_SAFE_CALL(test_6);
}
