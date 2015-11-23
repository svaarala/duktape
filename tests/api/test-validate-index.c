/*===
top=3, idx=-5, duk_is_valid_index -> 0
top=3, idx=-4, duk_is_valid_index -> 0
top=3, idx=-3, duk_is_valid_index -> 1
top=3, idx=-2, duk_is_valid_index -> 1
top=3, idx=-1, duk_is_valid_index -> 1
top=3, idx=0, duk_is_valid_index -> 1
top=3, idx=1, duk_is_valid_index -> 1
top=3, idx=2, duk_is_valid_index -> 1
top=3, idx=3, duk_is_valid_index -> 0
top=3, idx=4, duk_is_valid_index -> 0
top=3, idx=5, duk_is_valid_index -> 0
req_valid_idx: top 3 after popping arg
idx=-5, duk_require_valid_index -> Error: invalid stack index -5
req_valid_idx: top 3 after popping arg
idx=-4, duk_require_valid_index -> Error: invalid stack index -4
req_valid_idx: top 3 after popping arg
idx=-3, duk_require_valid_index -> true
req_valid_idx: top 3 after popping arg
idx=-2, duk_require_valid_index -> true
req_valid_idx: top 3 after popping arg
idx=-1, duk_require_valid_index -> true
req_valid_idx: top 3 after popping arg
idx=0, duk_require_valid_index -> true
req_valid_idx: top 3 after popping arg
idx=1, duk_require_valid_index -> true
req_valid_idx: top 3 after popping arg
idx=2, duk_require_valid_index -> true
req_valid_idx: top 3 after popping arg
idx=3, duk_require_valid_index -> Error: invalid stack index 3
req_valid_idx: top 3 after popping arg
idx=4, duk_require_valid_index -> Error: invalid stack index 4
req_valid_idx: top 3 after popping arg
idx=5, duk_require_valid_index -> Error: invalid stack index 5
===*/

static duk_ret_t req_valid_idx(duk_context *ctx) {
	duk_idx_t idx = duk_get_int(ctx, -1);

	duk_pop(ctx);
	printf("req_valid_idx: top %ld after popping arg\n", (long) duk_get_top(ctx));

	duk_require_valid_index(ctx, idx);
	duk_push_true(ctx);
	return 1;
}

void test(duk_context *ctx) {
	duk_idx_t idx;

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_push_int(ctx, 345);

	for (idx = -5; idx <= 5; idx++) {
		printf("top=%ld, idx=%ld, duk_is_valid_index -> %d\n",
		       (long) duk_get_top(ctx), (long) idx,
		       (int) duk_is_valid_index(ctx, idx));
	}

	for (idx = -5; idx <= 5; idx++) {
		duk_push_int(ctx, idx);
		duk_safe_call(ctx, req_valid_idx, 1, 1);
		printf("idx=%ld, duk_require_valid_index -> %s\n",
		       (long) idx, duk_to_string(ctx, -1));
		duk_pop(ctx);
	}
}
