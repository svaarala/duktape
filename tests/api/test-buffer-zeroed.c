/*===
fixed: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
dynamic: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
===*/

/* Test that buffers pushed to the stack are automatically zeroed. */

void test(duk_context *ctx) {
	int i;
	unsigned char *p;

	p = (unsigned char *) duk_push_fixed_buffer(ctx, 32);
	printf("fixed:");
	for (i = 0; i < 32; i++) {
		printf(" %d", (int) p[i]);
	}
	printf("\n");

	p = (unsigned char *) duk_push_dynamic_buffer(ctx, 32);
	printf("dynamic:");
	for (i = 0; i < 32; i++) {
		printf(" %d", (int) p[i]);
	}
	printf("\n");
}
