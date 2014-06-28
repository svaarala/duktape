/*===
index 0 -> type 1, value 'undefined'
index 1 -> type 2, value 'null'
index 2 -> type 3, value 'true'
index 3 -> type 3, value 'false'
index 4 -> type 4, value '-Infinity'
index 5 -> type 4, value '-123'
index 6 -> type 4, value '0'
index 7 -> type 4, value '0'
index 8 -> type 4, value '123'
index 9 -> type 4, value 'Infinity'
index 10 -> type 4, value 'NaN'
index 11 -> type 5, value ''
index 12 -> type 5, value 'foo'
index 13 -> type 5, value 'bar'
index 14 -> type 6, value '[object Object]'
index 15 -> type 6, value ''
index 16 -> type 7, value ''
index 17 -> type 7, value 'foo'
index 18 -> type 7, value 'foo'
index 19 -> type 7, value 'bar'
index 20 -> type 8, value 'null'
index 21 -> type 8, value '0xdeadbeef'
0 vs. 0 -> equals=1, strict_equals=1
0 vs. 1 -> equals=1, strict_equals=0
1 vs. 0 -> equals=1, strict_equals=0
1 vs. 1 -> equals=1, strict_equals=1
2 vs. 2 -> equals=1, strict_equals=1
3 vs. 3 -> equals=1, strict_equals=1
3 vs. 6 -> equals=1, strict_equals=0
3 vs. 7 -> equals=1, strict_equals=0
3 vs. 11 -> equals=1, strict_equals=0
3 vs. 15 -> equals=1, strict_equals=0
3 vs. 16 -> equals=1, strict_equals=0
4 vs. 4 -> equals=1, strict_equals=1
5 vs. 5 -> equals=1, strict_equals=1
6 vs. 3 -> equals=1, strict_equals=0
6 vs. 6 -> equals=1, strict_equals=1
6 vs. 7 -> equals=1, strict_equals=1
6 vs. 11 -> equals=1, strict_equals=0
6 vs. 15 -> equals=1, strict_equals=0
6 vs. 16 -> equals=1, strict_equals=0
7 vs. 3 -> equals=1, strict_equals=0
7 vs. 6 -> equals=1, strict_equals=1
7 vs. 7 -> equals=1, strict_equals=1
7 vs. 11 -> equals=1, strict_equals=0
7 vs. 15 -> equals=1, strict_equals=0
7 vs. 16 -> equals=1, strict_equals=0
8 vs. 8 -> equals=1, strict_equals=1
9 vs. 9 -> equals=1, strict_equals=1
11 vs. 3 -> equals=1, strict_equals=0
11 vs. 6 -> equals=1, strict_equals=0
11 vs. 7 -> equals=1, strict_equals=0
11 vs. 11 -> equals=1, strict_equals=1
11 vs. 15 -> equals=1, strict_equals=0
11 vs. 16 -> equals=1, strict_equals=0
12 vs. 12 -> equals=1, strict_equals=1
12 vs. 17 -> equals=1, strict_equals=0
12 vs. 18 -> equals=1, strict_equals=0
13 vs. 13 -> equals=1, strict_equals=1
13 vs. 19 -> equals=1, strict_equals=0
14 vs. 14 -> equals=1, strict_equals=1
15 vs. 3 -> equals=1, strict_equals=0
15 vs. 6 -> equals=1, strict_equals=0
15 vs. 7 -> equals=1, strict_equals=0
15 vs. 11 -> equals=1, strict_equals=0
15 vs. 15 -> equals=1, strict_equals=1
15 vs. 16 -> equals=1, strict_equals=0
16 vs. 3 -> equals=1, strict_equals=0
16 vs. 6 -> equals=1, strict_equals=0
16 vs. 7 -> equals=1, strict_equals=0
16 vs. 11 -> equals=1, strict_equals=0
16 vs. 15 -> equals=1, strict_equals=0
16 vs. 16 -> equals=1, strict_equals=1
17 vs. 12 -> equals=1, strict_equals=0
17 vs. 17 -> equals=1, strict_equals=1
17 vs. 18 -> equals=1, strict_equals=0
18 vs. 12 -> equals=1, strict_equals=0
18 vs. 17 -> equals=1, strict_equals=0
18 vs. 18 -> equals=1, strict_equals=1
19 vs. 13 -> equals=1, strict_equals=0
19 vs. 19 -> equals=1, strict_equals=1
20 vs. 20 -> equals=1, strict_equals=1
21 vs. 21 -> equals=1, strict_equals=1
===*/

void test(duk_context *ctx) {
	char *buf1, *buf2, *buf3;
	duk_idx_t i, j, n;

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_number(ctx, -INFINITY);
	duk_push_number(ctx, -123.0);
	duk_push_number(ctx, -0.0);
	duk_push_number(ctx, +0.0);
	duk_push_number(ctx, +123.0);
	duk_push_number(ctx, +INFINITY);

	duk_push_number(ctx, NAN);
	duk_push_string(ctx, "");
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	duk_push_object(ctx);
	duk_push_array(ctx);
	(void) duk_push_fixed_buffer(ctx, 0);
	buf1 = (char *) duk_push_fixed_buffer(ctx, 3);
	buf1[0] = 'f';  buf1[1] = 'o'; buf1[2] = 'o';
	buf2 = (char *) duk_push_dynamic_buffer(ctx, 3);
	buf2[0] = 'f';  buf2[1] = 'o'; buf2[2] = 'o';
	buf3 = (char *) duk_push_dynamic_buffer(ctx, 3);
	buf3[0] = 'b';  buf3[1] = 'a'; buf3[2] = 'r';

	duk_push_pointer(ctx, NULL);
	duk_push_pointer(ctx, (void *) 0xdeadbeef);

	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		duk_dup(ctx, i);
		printf("index %ld -> type %d, value '%s'\n", (long) i,
		       (int) duk_get_type(ctx, i), duk_to_string(ctx, -1));
		duk_pop(ctx);
	}

	for (i = 0; i <= n + 1; i++) {
		for (j = 0; j <= n + 1; j++) {
			duk_idx_t idx1, idx2;
			duk_bool_t eq, seq;

			/* Note: i and j run up to 'n + 1' (invalid index) on purpose. */
			idx1 = (i == n + 1 ? DUK_INVALID_INDEX : i);
			idx2 = (j == n + 1 ? DUK_INVALID_INDEX : j);

			eq = duk_equals(ctx, i, j);
			seq = duk_strict_equals(ctx, i, j);

			/* Print nothing if neither equality is true */
			if (!eq && !seq) {
				continue;
			}

			if (idx1 == DUK_INVALID_INDEX) {
				printf("DUK_INVALID_INDEX");
			} else {
				printf("%ld", (long) idx1);
			}
			printf(" vs. ");
			if (idx2 == DUK_INVALID_INDEX) {
				printf("DUK_INVALID_INDEX");
			} else {
				printf("%ld", (long) idx2);
			}
			printf(" -> equals=%d, strict_equals=%d\n", (int) eq, (int) seq);
		}
	}
}
