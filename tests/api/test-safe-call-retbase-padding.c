/*
 *  If duk_safe_call() target pops too many value stcak elements,
 *  'undefined' values are inserted at the index where return
 *  values begin to restore the stack shape.
 *
 *  Example, duk_safe_call() with nargs=4, nrets=5:
 *
 *    [ a b c d e f g X Y Z W ]
 *                    <----->   args
 *                    ^
 *                    `---- retbase=7 (index from bottom)
 *
 *  Caller pops too much:
 *
 *    [ a b ]
 *
 *  then pushes two return values L and M:
 *
 *    [ a b L M ]
 *
 *  and returns rc=2 to indicate two actual return values are present.
 *
 *  This is resolved by injecting 'undefined' values to lift the intended
 *  return values to 'retbase' index (U=undefined):
 *
 *    [ a b U U U U U L M ]
 *
 *  Finally, because 5 values were expected, further 'undefined' values are
 *  pushed to fulfill the API contract:
 *
 *    [ a b U U U U U L M U U U ]
 *                    <------->
 *                    ^
 *                    `---- retbase=7
 */

/*===
a b c d e f g X Y Z W
top before: 11
a b L M
duk_safe_call() rc: 0
a b undefined undefined undefined undefined undefined L M undefined undefined undefined
final top: 12
===*/

static void dump_value_stack(duk_context *ctx) {
	duk_idx_t i;

	for (i = 0; i < duk_get_top(ctx); i++) {
		if (i > 0) {
			printf(" ");
		}
		duk_dup(ctx, i);
		printf("%s", duk_to_string(ctx, -1));
		duk_pop(ctx);
	}
	printf("\n");
}

static duk_ret_t my_func(duk_context *ctx, void *udata) {
	(void) udata;
	duk_set_top(ctx, 2);
	duk_push_string(ctx, "L");
	duk_push_string(ctx, "M");
	dump_value_stack(ctx);
	return 2;
}

void test(duk_context *ctx) {
	duk_int_t rc;

	duk_push_string(ctx, "a");
	duk_push_string(ctx, "b");
	duk_push_string(ctx, "c");
	duk_push_string(ctx, "d");
	duk_push_string(ctx, "e");
	duk_push_string(ctx, "f");
	duk_push_string(ctx, "g");

	duk_push_string(ctx, "X");
	duk_push_string(ctx, "Y");
	duk_push_string(ctx, "Z");
	duk_push_string(ctx, "W");

	dump_value_stack(ctx);

	printf("top before: %ld\n", (long) duk_get_top(ctx));

	rc = duk_safe_call(ctx, my_func, NULL, 4, 5);
	printf("duk_safe_call() rc: %ld\n", (long) rc);

	dump_value_stack(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
}
