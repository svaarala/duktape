/*
 *  Very simple example program, pointer compression and ROM built-ins enabled
 */

#include "duktape.h"
#include "duk_alloc_pool.h"

#define POOL_BUFFER_SIZE (255 * 1024)
#define POOL_NUM_POOLS 28

static const duk_pool_config pool_config[POOL_NUM_POOLS] = {
	{ 8,      10 * 8,     0 },
	{ 12,     600 * 12,   0 },
	{ 16,     300 * 16,   0 },
	{ 20,     300 * 20,   0 },
	{ 24,     300 * 24,   0 },
	{ 28,     250 * 28,   0 },
	{ 32,     150 * 32,   0 },
	{ 40,     150 * 40,   0 },
	{ 48,     50 * 48,    0 },
	{ 52,     50 * 52,    0 },
	{ 56,     50 * 56,    0 },
	{ 60,     50 * 60,    0 },
	{ 64,     50 * 64,    0 },
	{ 96,     50 * 96,    0 },
	{ 196,    0,          196 },  /* duk_heap, with heap ptr compression, ROM strings+objects */
	{ 232,    0,          232 },  /* duk_hthread, with heap ptr compression, ROM strings+objects */
	{ 256,    16 * 256,   0 },
	{ 288,    1 * 288,    0 },
	{ 320,    1 * 320,    0 },
	{ 400,    0,          400 },  /* duk_hthread, with heap ptr compression, RAM strings+objects */
	{ 520,    0,          520 },  /* duk_heap, with heap ptr compression, RAM strings+objects */
	{ 512,    16 * 512,   0 },
	{ 768,    0,          768 },  /* initial value stack for packed duk_tval */
	{ 1024,   6 * 1024,   0 },
	{ 2048,   5 * 2048,   0 },
	{ 4096,   3 * 4096,   0 },
	{ 8192,   3 * 8192,   0 },
	{ 16384,  1 * 16384,  0 },
};

static duk_pool_state pool_state[POOL_NUM_POOLS];

static duk_pool_global pool_global;

static duk_ret_t native_print(duk_context *ctx) {
	duk_push_string(ctx, " ");
	duk_insert(ctx, 0);
	duk_join(ctx, duk_get_top(ctx) - 1);
	printf("%s\n", duk_safe_to_string(ctx, -1));
	return 0;
}

static duk_ret_t native_adder(duk_context *ctx) {
	int i;
	int n = duk_get_top(ctx);  /* #args */
	double res = 0.0;

	for (i = 0; i < n; i++) {
		res += duk_to_number(ctx, i);
	}

	duk_push_number(ctx, res);
	return 1;  /* one return value */
}

static void dump_pool_stats(void) {
	duk_pool_global_stats st;
	duk_alloc_pool_get_global_stats(&pool_global, &st);
	printf("POOL STATS: used=%ld free=%ld waste=%ld; hwm used=%ld, hwm waste=%ld (bytes)\n",
	       (long) st.used_bytes, (long) st.free_bytes, (long) st.waste_bytes,
	       (long) st.hwm_used_bytes, (long) st.hwm_waste_bytes);
}

int main(int argc, char *argv[]) {
	duk_context *ctx;
	void *heap_udata;
	void *pool_buffer;

	(void) argc; (void) argv;  /* suppress warning */

	pool_buffer = malloc(POOL_BUFFER_SIZE);
	printf("pool_buffer: %p\n", pool_buffer);

	heap_udata = duk_alloc_pool_init(pool_buffer,
	                                 POOL_BUFFER_SIZE,
	                                 pool_config,
	                                 pool_state,
	                                 POOL_NUM_POOLS,
	                                 &pool_global);
	printf("heap_udata: %p\n", heap_udata);

	ctx = duk_create_heap(duk_alloc_pool,
	                      duk_realloc_pool,
	                      duk_free_pool,
	                      heap_udata,
	                      NULL);
	printf("ctx: %p\n", ctx);

	dump_pool_stats();

	duk_push_c_function(ctx, native_print, DUK_VARARGS);
	duk_put_global_string(ctx, "print");
	duk_push_c_function(ctx, native_adder, DUK_VARARGS);
	duk_put_global_string(ctx, "adder");

	duk_eval_string(ctx, "print('Hello world!');");

	duk_eval_string(ctx, "print('2+3=' + adder(2, 3));");
	duk_pop(ctx);  /* pop eval result */

	dump_pool_stats();

	duk_destroy_heap(ctx);

	free(pool_buffer);

	return 0;
}
