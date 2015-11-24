/*===
top=3, idx=-5, duk_normalize_index -> DUK_INVALID_INDEX
top=3, idx=-4, duk_normalize_index -> DUK_INVALID_INDEX
top=3, idx=-3, duk_normalize_index -> 0
top=3, idx=-2, duk_normalize_index -> 1
top=3, idx=-1, duk_normalize_index -> 2
top=3, idx=0, duk_normalize_index -> 0
top=3, idx=1, duk_normalize_index -> 1
top=3, idx=2, duk_normalize_index -> 2
top=3, idx=3, duk_normalize_index -> DUK_INVALID_INDEX
top=3, idx=4, duk_normalize_index -> DUK_INVALID_INDEX
top=3, idx=5, duk_normalize_index -> DUK_INVALID_INDEX
req_norm_idx: top 3 after popping arg
idx=-5 -> duk_require_normalize_index -> Error: invalid stack index -5
req_norm_idx: top 3 after popping arg
idx=-4 -> duk_require_normalize_index -> Error: invalid stack index -4
req_norm_idx: top 3 after popping arg
idx=-3 -> duk_require_normalize_index -> 0
req_norm_idx: top 3 after popping arg
idx=-2 -> duk_require_normalize_index -> 1
req_norm_idx: top 3 after popping arg
idx=-1 -> duk_require_normalize_index -> 2
req_norm_idx: top 3 after popping arg
idx=0 -> duk_require_normalize_index -> 0
req_norm_idx: top 3 after popping arg
idx=1 -> duk_require_normalize_index -> 1
req_norm_idx: top 3 after popping arg
idx=2 -> duk_require_normalize_index -> 2
req_norm_idx: top 3 after popping arg
idx=3 -> duk_require_normalize_index -> Error: invalid stack index 3
req_norm_idx: top 3 after popping arg
idx=4 -> duk_require_normalize_index -> Error: invalid stack index 4
req_norm_idx: top 3 after popping arg
idx=5 -> duk_require_normalize_index -> Error: invalid stack index 5
===*/

static duk_ret_t req_norm_idx(duk_context *ctx) {
	duk_idx_t idx = (duk_idx_t) duk_get_int(ctx, -1);
	duk_idx_t norm_idx;

	duk_pop(ctx);
	printf("req_norm_idx: top %ld after popping arg\n", (long) duk_get_top(ctx));

	norm_idx = duk_require_normalize_index(ctx, idx);
	duk_push_int(ctx, norm_idx);
	return 1;
}

void test(duk_context *ctx) {
	duk_idx_t idx, norm_idx;

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_push_int(ctx, 345);

	for (idx = -5; idx <= 5; idx++) {
		norm_idx = duk_normalize_index(ctx, idx);
		if (norm_idx == DUK_INVALID_INDEX) {
			printf("top=%ld, idx=%ld, duk_normalize_index -> DUK_INVALID_INDEX\n",
			       (long) duk_get_top(ctx), (long) idx);
		} else {
			printf("top=%ld, idx=%ld, duk_normalize_index -> %ld\n",
			       (long) duk_get_top(ctx), (long) idx, (long) norm_idx);
		}
	}

	for (idx = -5; idx <= 5; idx++) {
		duk_push_int(ctx, idx);
		duk_safe_call(ctx, req_norm_idx, 1, 1);
		printf("idx=%ld -> duk_require_normalize_index -> %s\n", (long) idx, duk_to_string(ctx, -1));
		duk_pop(ctx);
	}
}
