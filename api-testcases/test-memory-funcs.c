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
duk_alloc memory is zeroed
p is non-NULL
bytes: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
===*/

/* Very basic test of API memory alloc/realloc/free functions.
 * Does not cover all argument combinations, just ensurer that
 * basic alloc/realloc/free sequences work and don't cause
 * valgrind issues.
 */

void test_gc_variants(duk_context *ctx) {
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

void test_raw_variants(duk_context *ctx) {
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

void test_mixed_use(duk_context *ctx) {
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

void test_alloc_zeroed(duk_context *ctx) {
	void *p;
	size_t sz = 64;

	/* FIXME: currently allocated memory is NOT zeroed - should it be? */

	printf("duk_alloc memory is zeroed\n");
	p = duk_alloc(ctx, sz);
	if (p) {
		unsigned char *q = (unsigned char *) p;
		int i;

		printf("p is non-NULL\n");
		printf("bytes:");
		for (i = 0; i < (int) sz; i++) {
			printf(" %d", (int) q[i]);
		}
		printf("\n");

		duk_free(ctx, p);
	} else {
		printf("p is NULL\n");
	}

	/* Reallocated memory cannot be automatically zeroed in case where
	 * the buffer grows: the allocation function doesn't know what the
	 * original size was, so it cannot zero the new part only.
	 */
}

void test(duk_context *ctx) {
	test_raw_variants(ctx);
	test_gc_variants(ctx);
	test_mixed_use(ctx);
	test_alloc_zeroed(ctx);
}

