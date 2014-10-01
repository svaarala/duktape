/*===
alloc matches: 1
realloc matches: 1
free matches: 1
udata: 0x12345678
===*/

static int alloc_count = 0;
static int realloc_count = 0;
static int free_count = 0;

static void *my_alloc(void *udata, size_t size) {
#if 0
	printf("my_alloc: %d\n", (int) size);
#endif
	alloc_count++;
	return malloc(size);
}

static void *my_realloc(void *udata, void *ptr, size_t size) {
#if 0
	printf("my_realloc: %d\n", (int) size);
#endif
	realloc_count++;
	return realloc(ptr, size);
}

static void my_free(void *udata, void *ptr) {
#if 0
	printf("my_free\n");
#endif
	free_count++;
	free(ptr);
}

void test(duk_context *ctx) {
	duk_memory_functions funcs;
	duk_context *new_ctx;

	new_ctx = duk_create_heap(my_alloc, my_realloc, my_free, (void *) 0x12345678, NULL);
	duk_get_memory_functions(new_ctx, &funcs);
	printf("alloc matches: %d\n", my_alloc == funcs.alloc_func);
	printf("realloc matches: %d\n", my_realloc == funcs.realloc_func);
	printf("free matches: %d\n", my_free == funcs.free_func);
	printf("udata: %p\n", funcs.udata);
	duk_destroy_heap(new_ctx);

#if 0
	printf("counts: alloc=%d, realloc=%d, free=%d\n", alloc_count, realloc_count, free_count);
#endif
}
