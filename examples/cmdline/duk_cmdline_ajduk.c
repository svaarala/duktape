/*
 *  'ajduk' specific functionality, examples for low memory techniques
 */

#ifdef DUK_CMDLINE_AJSHEAP
/*
 *  Heap initialization when using AllJoyn.js pool allocator (without any
 *  other AllJoyn.js integration).  This serves as an example of how to
 *  integrate Duktape with a pool allocator and is useful for low memory
 *  testing.
 *
 *  The pool sizes are not optimized here.  The sizes are chosen so that
 *  you can look at the high water mark (hwm) and use counts (use) and see
 *  how much allocations are needed for each pool size.  To optimize pool
 *  sizes more accurately, you can use --alloc-logging and inspect the memory
 *  allocation log which provides exact byte counts etc.
 *
 *  https://git.allseenalliance.org/cgit/core/alljoyn-js.git
 *  https://git.allseenalliance.org/cgit/core/alljoyn-js.git/tree/ajs.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ajs.h"
#include "ajs_heap.h"

static const AJS_HeapConfig ajsheap_config[] = {
	{ 8,      10,   AJS_POOL_BORROW,  0 },
	{ 12,     10,   AJS_POOL_BORROW,  0 },
	{ 16,     200,  AJS_POOL_BORROW,  0 },
	{ 20,     400,  AJS_POOL_BORROW,  0 },
	{ 24,     400,  AJS_POOL_BORROW,  0 },
	{ 28,     200,  AJS_POOL_BORROW,  0 },
	{ 32,     200,  AJS_POOL_BORROW,  0 },
	{ 40,     200,  AJS_POOL_BORROW,  0 },
	{ 48,     50,   AJS_POOL_BORROW,  0 },
	{ 52,     50,   AJS_POOL_BORROW,  0 },
	{ 56,     50,   AJS_POOL_BORROW,  0 },
	{ 60,     50,   AJS_POOL_BORROW,  0 },
	{ 64,     50,   0,                0 },
	{ 128,    80,   0,                0 },
	{ 256,    16,   0,                0 },
	{ 512,    16,   0,                0 },
	{ 1024,   6,    0,                0 },
	{ 2048,   5,    0,                0 },
	{ 4096,   3,    0,                0 },
	{ 8192,   1,    0,                0 }
};

uint8_t *ajsheap_ram = NULL;

/* Example pointer compression functions.
 * 'base' is chosen so that no non-NULL pointer results in a zero result
 * which is reserved for NULL pointers.
 */
duk_uint16_t ajsheap_enc16(void *p) {
	duk_uint32_t ret;
	char *base = (char *) ajsheap_ram - 4;

	if (p == NULL) {
		ret = 0;
	} else {
		ret = (duk_uint32_t) (((char *) p - base) >> 2);
	}
#if 0
	printf("ajsheap_enc16: %p -> %u\n", p, (unsigned int) ret);
#endif
	if (ret > 0xffffUL) {
		fprintf(stderr, "Failed to compress pointer\n");
		fflush(stderr);
		abort();
	}
	return (duk_uint16_t) ret;
}
void *ajsheap_dec16(duk_uint16_t x) {
	void *ret;
	char *base = (char *) ajsheap_ram - 4;

	if (x == 0) {
		ret = NULL;
	} else {
		ret = (void *) (base + (((duk_uint32_t) x) << 2));
	}
#if 0
	printf("ajsheap_dec16: %u -> %p\n", (unsigned int) x, ret);
#endif
	return ret;
}

/* Simplified example of an external strings strategy where incoming strings
 * are writted sequentially into a fixed flash memory area which is memory
 * mapped.  The example first scans if the string is already in the flash
 * (which may happen if the same string is interned multiple times), then
 * adds it to flash if there is space.
 *
 * This example is too slow to be used in a real world application: there
 * should be e.g. a hash table to quickly check for strings that are already
 * present in the string data (similarly to how string interning works in
 * Duktape itself).
 */
static uint8_t ajsheap_strdata[65536];
static size_t ajsheap_strdata_used = 0;

const void *ajsheap_ext_str_check(const void *ptr, duk_size_t len) {
	uint8_t *p, *p_end;
	uint8_t initial;
	uint8_t *ret;
	size_t left;

	if (len <= 3) {
		/* It's not worth it to make very small strings external, as
		 * they would take the same space anyway.  Also avoids zero
		 * length degenerate case.
		 */
	}

	/*
	 *  Check if we already have the string.  Be careful to compare for
	 *  NUL terminator too, it is NOT present in 'ptr'.  This algorithm
	 *  is too simplistic and way too slow for actual use.
	 */

	initial = ((const uint8_t *) ptr)[0];
	for (p = ajsheap_strdata, p_end = p + ajsheap_strdata_used; p != p_end; p++) {
		if (*p != initial) {
			continue;
		}
		left = (size_t) (p_end - p);
		if (left >= len + 1 &&
		    memcmp(p, ptr, len) == 0 &&
		    p[len] == 0) {
			ret = p;
#if 0
			printf("ajsheap_ext_str_check: ptr=%p, len=%ld -> existing %p (used=%ld)\n", (void *) ptr, (long) len, (void *) ret, (long) ajsheap_strdata_used);
#endif
			return ret;
		}
	}

	/*
	 *  Not present yet, check if we have space.  Again, be careful to
	 *  ensure there is space for a NUL following the input data.
	 */

	if (ajsheap_strdata_used + len + 1 > sizeof(ajsheap_strdata)) {
#if 0
		printf("ajsheap_ext_str_check: ptr=%p, len=%ld -> no space (used=%ld)\n", (void *) ptr, (long) len, (long) ajsheap_strdata_used);
#endif
		return NULL;
	}

	/*
	 *  There is space, add the string to our collection, being careful
	 *  to append the NUL.
	 */

	ret = ajsheap_strdata + ajsheap_strdata_used;
	memcpy(ret, ptr, len);
	ret[len] = (uint8_t) 0;
	ajsheap_strdata_used += len + 1;

#if 0
	printf("ajsheap_ext_str_check: ptr=%p, len=%ld -> %p (used=%ld)\n", (void *) ptr, (long) len, (void *) ret, (long) ajsheap_strdata_used);
#endif
	return (const void *) ret;
}

void ajsheap_init(void) {
	size_t heap_sz[1];
	uint8_t *heap_array[1];
	uint8_t num_pools, i;
	AJ_Status ret;

	num_pools = (uint8_t) (sizeof(ajsheap_config) / sizeof(AJS_HeapConfig));
	heap_sz[0] = AJS_HeapRequired(ajsheap_config,  /* heapConfig */
	                              num_pools,       /* numPools */
	                              0);              /* heapNum */
	ajsheap_ram = (uint8_t *) malloc(heap_sz[0]);
	if (!ajsheap_ram) {
		fprintf(stderr, "Failed to allocate AJS heap\n");
		fflush(stderr);
		exit(1);
	}
	heap_array[0] = ajsheap_ram;

	fprintf(stderr, "Allocated AJS heap of %ld bytes, pools:", (long) heap_sz[0]);
	for (i = 0; i < num_pools; i++) {
		fprintf(stderr, " (sz:%ld,num:%ld,brw:%ld,idx:%ld)",
		        (long) ajsheap_config[i].size, (long) ajsheap_config[i].entries,
		        (long) ajsheap_config[i].borrow, (long) ajsheap_config[i].heapIndex);
	}
	fprintf(stderr, "\n");
	fflush(stderr);

	ret = AJS_HeapInit((void **) heap_array,   /* heap */
	                   (size_t *) heap_sz,     /* heapSz */
	                   ajsheap_config,         /* heapConfig */
	                   num_pools,              /* numPools */
	                   1);                     /* numHeaps */
	fprintf(stderr, "AJS_HeapInit() -> %ld\n", (long) ret);
	fflush(stderr);
}

/* AjsHeap.dump(), allows Ecmascript code to dump heap status at suitable
 * points.
 */
duk_ret_t ajsheap_dump_binding(duk_context *ctx) {
	AJS_HeapDump();
	fflush(stdout);
	return 0;
}

void ajsheap_dump(void) {
	AJS_HeapDump();
	fflush(stdout);
}

void ajsheap_register(duk_context *ctx) {
	duk_push_object(ctx);
	duk_push_c_function(ctx, ajsheap_dump_binding, 0);
	duk_put_prop_string(ctx, -2, "dump");
	duk_put_global_string(ctx, "AjsHeap");
}
#else
int ajs_dummy = 0;  /* to avoid empty source file */
#endif  /* DUK_CMDLINE_AJSHEAP */
