/*===
alloc matches: 1
realloc matches: 1
free matches: 1
udata: 0x12345678
===*/

int alloc_count = 0;
int realloc_count = 0;
int free_count = 0;

void *my_alloc(void *udata, size_t size) {
#if 0
	printf("my_alloc: %d\n", (int) size);
	alloc_count++;
#endif
	return malloc(size);
}

void *my_realloc(void *udata, void *ptr, size_t size) {
#if 0
	printf("my_realloc: %d\n", (int) size);
	realloc_count++;
#endif
	return realloc(ptr, size);
}

void my_free(void *udata, void *ptr) {
#if 0
	printf("my_free\n");
	free_count++;
#endif
	free(ptr);
}

void test(duk_context *ctx) {
	duk_memory_functions funcs;
	duk_context *new_ctx;

	new_ctx = duk_create_heap(my_alloc, my_realloc, my_free, (void *) 0x12345678, NULL);
	duk_get_memory_functions(new_ctx, &funcs);
	printf("alloc matches: %d\n", my_alloc == funcs.alloc);
	printf("realloc matches: %d\n", my_realloc == funcs.realloc);
	printf("free matches: %d\n", my_free == funcs.free);
	printf("udata: %p\n", funcs.udata);
	duk_destroy_heap(new_ctx);

#if 0
	printf("counts: alloc=%d, realloc=%d, free=%d\n", alloc_count, realloc_count, free_count);
#endif
}

