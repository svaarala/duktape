/*===
*** test_basic (duk_safe_call)
0: 1 0
1: 1 0
2: 1 1
3: 1 1
4: 1 1
5: 1 0
6: 1 0
7: 1 0
8: 1 0
9: 1 0
10: 1 0
11: 1 0
12: 1 0
13: 1 0
14: 1 0
15: 1 0
16: 1 0
17: 1 0
18: 1 0
19: 1 0
20: 1 0
21: 1 0
22: 1 0
23: 1 1
24: 0 0
final top: 24
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_idx_t i, n;

	(void) udata;

	duk_push_string(ctx, "");
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "\x80" "bar");
	duk_push_string(ctx, "\x81" "quux");
	duk_push_string(ctx, "\x82" "baz");
	duk_push_string(ctx, "\x83" "notsymbol");  /* 0x83 to 0xbf are reserved but not symbols, sample some values */
	duk_push_string(ctx, "\x84" "notsymbol");
	duk_push_string(ctx, "\x85" "notsymbol");
	duk_push_string(ctx, "\x86" "notsymbol");
	duk_push_string(ctx, "\x87" "notsymbol");
	duk_push_string(ctx, "\x88" "notsymbol");
	duk_push_string(ctx, "\x89" "notsymbol");
	duk_push_string(ctx, "\x8a" "notsymbol");
	duk_push_string(ctx, "\x8b" "notsymbol");
	duk_push_string(ctx, "\x8c" "notsymbol");
	duk_push_string(ctx, "\x8d" "notsymbol");
	duk_push_string(ctx, "\x8e" "notsymbol");
	duk_push_string(ctx, "\x8f" "notsymbol");
	duk_push_string(ctx, "\x9f" "notsymbol");
	duk_push_string(ctx, "\xaf" "notsymbol");
	duk_push_string(ctx, "\xbf" "notsymbol");
	duk_push_string(ctx, "\xc0" "quuuux");
	duk_push_string(ctx, "\xfe" "quuuuux");
	duk_push_string(ctx, "\xff" "quuuuuux");

	for (i = 0, n = duk_get_top(ctx) + 1; i < n; i++) {
		printf("%ld: %ld %ld\n", (long) i, (long) duk_is_string(ctx, i), (long) duk_is_symbol(ctx, i));
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
