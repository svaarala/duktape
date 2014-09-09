/*===
basic duk_alloc_raw + duk_free_raw
p is non-NULL
duk_free_raw with NULL
duk_alloc_raw + duk_realloc_raw + duk_free_raw
p is non-NULL
new_p is non-NULL
new_p[0], new_p[1023]: 1 2
duk_alloc_raw + duk_realloc_raw + duk_realloc_raw zero size (= free)
p is non-NULL
new_p is NULL
basic duk_alloc + duk_free
p is non-NULL
duk_free with NULL
duk_alloc + duk_realloc + duk_free
p is non-NULL
new_p is non-NULL
new_p[0], new_p[1023]: 1 2
duk_alloc + duk_realloc + duk_realloc zero size (= free)
p is non-NULL
new_p is NULL
duk_alloc + duk_realloc_raw + duk_free_raw
p is non-NULL
new_p is non-NULL
new_p[0], new_p[1023]: 1 2
duk_alloc_raw + duk_realloc + duk_free
p is non-NULL
new_p is non-NULL
new_p[0], new_p[1023]: 1 2
===*/

/* Very basic test of API memory alloc/realloc/free functions.
 * Does not cover all argument combinations, just ensures that
 * basic alloc/realloc/free sequences work and don't cause
 * valgrind issues.
 */

static void test_gc_variants(duk_context *ctx) {
	void *p, *new_p;

	printf("basic duk_alloc + duk_free\n");
	p = duk_alloc(ctx, 1024);
	if (p) {
		char *q;
		printf("p is non-NULL\n");
		q = (char *) p; q[0] = 1; q[1023] = 2;
		duk_free(ctx, p);
	} else {
		printf("p is NULL\n");
	}

	printf("duk_free with NULL\n");
	duk_free(ctx, NULL);

	printf("duk_alloc + duk_realloc + duk_free\n");
	p = duk_alloc(ctx, 1024);
	if (p) {
		char *q;
		printf("p is non-NULL\n");
		q = (char *) p; q[0] = 1; q[1023] = 2;

		new_p = duk_realloc(ctx, p, 2048);
		if (new_p) {
			printf("new_p is non-NULL\n");
			q = (char *) new_p;
			printf("new_p[0], new_p[1023]: %d %d\n", (int) q[0], (int) q[1023]);
			p = new_p;
		} else {
			printf("new_p is NULL\n");
		}
		duk_free(ctx, p);
	} else {
		printf("p is NULL\n");
	}

	printf("duk_alloc + duk_realloc + duk_realloc zero size (= free)\n");
	p = duk_alloc(ctx, 1024);
	if (p) {
		char *q;
		printf("p is non-NULL\n");
		q = (char *) p; q[0] = 1; q[1023] = 2;

		new_p = duk_realloc(ctx, p, 0);
		if (new_p) {
			printf("new_p is non-NULL\n");
		} else {
			printf("new_p is NULL\n");
		}
	} else {
		printf("p is NULL\n");
	}
}

static void test_raw_variants(duk_context *ctx) {
	void *p, *new_p;

	printf("basic duk_alloc_raw + duk_free_raw\n");
	p = duk_alloc_raw(ctx, 1024);
	if (p) {
		char *q;
		printf("p is non-NULL\n");
		q = (char *) p; q[0] = 1; q[1023] = 2;
		duk_free_raw(ctx, p);
	} else {
		printf("p is NULL\n");
	}

	printf("duk_free_raw with NULL\n");
	duk_free_raw(ctx, NULL);

	printf("duk_alloc_raw + duk_realloc_raw + duk_free_raw\n");
	p = duk_alloc_raw(ctx, 1024);
	if (p) {
		char *q;
		printf("p is non-NULL\n");
		q = (char *) p; q[0] = 1; q[1023] = 2;

		new_p = duk_realloc_raw(ctx, p, 2048);
		if (new_p) {
			printf("new_p is non-NULL\n");
			q = (char *) new_p;
			printf("new_p[0], new_p[1023]: %d %d\n", (int) q[0], (int) q[1023]);
			p = new_p;
		} else {
			printf("new_p is NULL\n");
		}
		duk_free_raw(ctx, p);
	} else {
		printf("p is NULL\n");
	}

	printf("duk_alloc_raw + duk_realloc_raw + duk_realloc_raw zero size (= free)\n");
	p = duk_alloc_raw(ctx, 1024);
	if (p) {
		char *q;
		printf("p is non-NULL\n");
		q = (char *) p; q[0] = 1; q[1023] = 2;

		new_p = duk_realloc_raw(ctx, p, 0);
		if (new_p) {
			printf("new_p is non-NULL\n");
		} else {
			printf("new_p is NULL\n");
		}
	} else {
		printf("p is NULL\n");
	}
}

static void test_mixed_use(duk_context *ctx) {
	void *p, *new_p;

	printf("duk_alloc + duk_realloc_raw + duk_free_raw\n");
	p = duk_alloc(ctx, 1024);
	if (p) {
		char *q;
		printf("p is non-NULL\n");
		q = (char *) p; q[0] = 1; q[1023] = 2;

		new_p = duk_realloc_raw(ctx, p, 2048);
		if (new_p) {
			printf("new_p is non-NULL\n");
			q = (char *) new_p;
			printf("new_p[0], new_p[1023]: %d %d\n", (int) q[0], (int) q[1023]);
			p = new_p;
		} else {
			printf("new_p is NULL\n");
		}
		duk_free_raw(ctx, p);
	} else {
		printf("p is NULL\n");
	}

	printf("duk_alloc_raw + duk_realloc + duk_free\n");
	p = duk_alloc_raw(ctx, 1024);
	if (p) {
		char *q;
		printf("p is non-NULL\n");
		q = (char *) p; q[0] = 1; q[1023] = 2;

		new_p = duk_realloc(ctx, p, 2048);
		if (new_p) {
			printf("new_p is non-NULL\n");
			q = (char *) new_p;
			printf("new_p[0], new_p[1023]: %d %d\n", (int) q[0], (int) q[1023]);
			p = new_p;
		} else {
			printf("new_p is NULL\n");
		}
		duk_free(ctx, p);
	} else {
		printf("p is NULL\n");
	}
}

void test(duk_context *ctx) {
	test_raw_variants(ctx);
	test_gc_variants(ctx);
	test_mixed_use(ctx);
}
