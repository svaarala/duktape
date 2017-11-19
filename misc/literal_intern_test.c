/*
 *  Rough performance test for comparing the difference between xxx_string(),
 *  xxx_lstring(), xxx_literal(), and xxx_heapptr() in application code.
 */

#include "duktape.h"

int main(int argc, const char *argv[]) {
	duk_context *ctx;
	int i;
	int mode;
	void *hptr1;
	void *hptr2;
	void *hptr3;
	void *hptr4;
	void *hptr5;
	void *hptr6;
	void *hptr7;
	void *hptr8;
	void *hptr9;
	void *hptr10;

	if (argc < 2) {
		return 1;
	}
	mode = atoi(argv[1]);

	ctx = duk_create_heap_default();

	duk_push_object(ctx);

	/* Push the strings we're dealing with, get their heapptrs, and keep
	 * them reachable throughout the test.  This is necessary for the
	 * heapptr test to work.  But we also want this so that the strings
	 * won't be freed and reallocated repeatedly during the test.  That
	 * might happen in real application code too, but this test focuses
	 * on the differences between the intern checks only, assuming the
	 * string itself is already in the string table.
	 */
	duk_push_string(ctx, "foo1");
	hptr1 = duk_get_heapptr(ctx, -1);
	duk_push_string(ctx, "foo2");
	hptr2 = duk_get_heapptr(ctx, -1);
	duk_push_string(ctx, "foo3");
	hptr3 = duk_get_heapptr(ctx, -1);
	duk_push_string(ctx, "foo4");
	hptr4 = duk_get_heapptr(ctx, -1);
	duk_push_string(ctx, "foo5");
	hptr5 = duk_get_heapptr(ctx, -1);
	duk_push_string(ctx, "foo6");
	hptr6 = duk_get_heapptr(ctx, -1);
	duk_push_string(ctx, "foo7");
	hptr7 = duk_get_heapptr(ctx, -1);
	duk_push_string(ctx, "foo8");
	hptr8 = duk_get_heapptr(ctx, -1);
	duk_push_string(ctx, "foo9");
	hptr9 = duk_get_heapptr(ctx, -1);
	duk_push_string(ctx, "foo10");
	hptr10 = duk_get_heapptr(ctx, -1);

	for (i = 0; i < 10000000; i++) {
		switch (mode) {
		case 0:
			duk_push_string(ctx, "foo1");
			duk_push_string(ctx, "foo2");
			duk_push_string(ctx, "foo3");
			duk_push_string(ctx, "foo4");
			duk_push_string(ctx, "foo5");
			duk_push_string(ctx, "foo6");
			duk_push_string(ctx, "foo7");
			duk_push_string(ctx, "foo8");
			duk_push_string(ctx, "foo9");
			duk_push_string(ctx, "foo10");
			break;
		case 1:
			duk_push_lstring(ctx, "foo1", 4);
			duk_push_lstring(ctx, "foo2", 4);
			duk_push_lstring(ctx, "foo3", 4);
			duk_push_lstring(ctx, "foo4", 4);
			duk_push_lstring(ctx, "foo5", 4);
			duk_push_lstring(ctx, "foo6", 4);
			duk_push_lstring(ctx, "foo7", 4);
			duk_push_lstring(ctx, "foo8", 4);
			duk_push_lstring(ctx, "foo9", 4);
			duk_push_lstring(ctx, "foo10", 5);
			break;
		case 2:
			duk_push_literal(ctx, "foo1");
			duk_push_literal(ctx, "foo2");
			duk_push_literal(ctx, "foo3");
			duk_push_literal(ctx, "foo4");
			duk_push_literal(ctx, "foo5");
			duk_push_literal(ctx, "foo6");
			duk_push_literal(ctx, "foo7");
			duk_push_literal(ctx, "foo8");
			duk_push_literal(ctx, "foo9");
			duk_push_literal(ctx, "foo10");
			break;
		case 3:
			duk_push_heapptr(ctx, hptr1);
			duk_push_heapptr(ctx, hptr2);
			duk_push_heapptr(ctx, hptr3);
			duk_push_heapptr(ctx, hptr4);
			duk_push_heapptr(ctx, hptr5);
			duk_push_heapptr(ctx, hptr6);
			duk_push_heapptr(ctx, hptr7);
			duk_push_heapptr(ctx, hptr8);
			duk_push_heapptr(ctx, hptr9);
			duk_push_heapptr(ctx, hptr10);
			break;
		case 10:
			duk_push_uint(ctx, 123);
			duk_put_prop_string(ctx, 0, "foo1");
			duk_push_uint(ctx, 123);
			duk_put_prop_string(ctx, 0, "foo2");
			duk_push_uint(ctx, 123);
			duk_put_prop_string(ctx, 0, "foo3");
			duk_push_uint(ctx, 123);
			duk_put_prop_string(ctx, 0, "foo4");
			duk_push_uint(ctx, 123);
			duk_put_prop_string(ctx, 0, "foo5");
			duk_push_uint(ctx, 123);
			duk_put_prop_string(ctx, 0, "foo6");
			duk_push_uint(ctx, 123);
			duk_put_prop_string(ctx, 0, "foo7");
			duk_push_uint(ctx, 123);
			duk_put_prop_string(ctx, 0, "foo8");
			duk_push_uint(ctx, 123);
			duk_put_prop_string(ctx, 0, "foo9");
			duk_push_uint(ctx, 123);
			duk_put_prop_string(ctx, 0, "foo10");
			duk_push_uint(ctx, 123);
			duk_get_prop_string(ctx, 0, "foo1");
			duk_get_prop_string(ctx, 0, "foo2");
			duk_get_prop_string(ctx, 0, "foo3");
			duk_get_prop_string(ctx, 0, "foo4");
			duk_get_prop_string(ctx, 0, "foo5");
			duk_get_prop_string(ctx, 0, "foo6");
			duk_get_prop_string(ctx, 0, "foo7");
			duk_get_prop_string(ctx, 0, "foo8");
			duk_get_prop_string(ctx, 0, "foo9");
			duk_get_prop_string(ctx, 0, "foo10");
			break;
		case 11:
			duk_push_uint(ctx, 123);
			duk_put_prop_lstring(ctx, 0, "foo1", 4);
			duk_push_uint(ctx, 123);
			duk_put_prop_lstring(ctx, 0, "foo2", 4);
			duk_push_uint(ctx, 123);
			duk_put_prop_lstring(ctx, 0, "foo3", 4);
			duk_push_uint(ctx, 123);
			duk_put_prop_lstring(ctx, 0, "foo4", 4);
			duk_push_uint(ctx, 123);
			duk_put_prop_lstring(ctx, 0, "foo5", 4);
			duk_push_uint(ctx, 123);
			duk_put_prop_lstring(ctx, 0, "foo6", 4);
			duk_push_uint(ctx, 123);
			duk_put_prop_lstring(ctx, 0, "foo7", 4);
			duk_push_uint(ctx, 123);
			duk_put_prop_lstring(ctx, 0, "foo8", 4);
			duk_push_uint(ctx, 123);
			duk_put_prop_lstring(ctx, 0, "foo9", 4);
			duk_push_uint(ctx, 123);
			duk_put_prop_lstring(ctx, 0, "foo10", 5);
			duk_push_uint(ctx, 123);
			duk_get_prop_lstring(ctx, 0, "foo1", 4);
			duk_get_prop_lstring(ctx, 0, "foo2", 4);
			duk_get_prop_lstring(ctx, 0, "foo3", 4);
			duk_get_prop_lstring(ctx, 0, "foo4", 4);
			duk_get_prop_lstring(ctx, 0, "foo5", 4);
			duk_get_prop_lstring(ctx, 0, "foo6", 4);
			duk_get_prop_lstring(ctx, 0, "foo7", 4);
			duk_get_prop_lstring(ctx, 0, "foo8", 4);
			duk_get_prop_lstring(ctx, 0, "foo9", 4);
			duk_get_prop_lstring(ctx, 0, "foo10", 5);
			break;
		case 12:
			duk_push_uint(ctx, 123);
			duk_put_prop_literal(ctx, 0, "foo1");
			duk_push_uint(ctx, 123);
			duk_put_prop_literal(ctx, 0, "foo2");
			duk_push_uint(ctx, 123);
			duk_put_prop_literal(ctx, 0, "foo3");
			duk_push_uint(ctx, 123);
			duk_put_prop_literal(ctx, 0, "foo4");
			duk_push_uint(ctx, 123);
			duk_put_prop_literal(ctx, 0, "foo5");
			duk_push_uint(ctx, 123);
			duk_put_prop_literal(ctx, 0, "foo6");
			duk_push_uint(ctx, 123);
			duk_put_prop_literal(ctx, 0, "foo7");
			duk_push_uint(ctx, 123);
			duk_put_prop_literal(ctx, 0, "foo8");
			duk_push_uint(ctx, 123);
			duk_put_prop_literal(ctx, 0, "foo9");
			duk_push_uint(ctx, 123);
			duk_put_prop_literal(ctx, 0, "foo10");
			duk_push_uint(ctx, 123);
			duk_get_prop_literal(ctx, 0, "foo1");
			duk_get_prop_literal(ctx, 0, "foo2");
			duk_get_prop_literal(ctx, 0, "foo3");
			duk_get_prop_literal(ctx, 0, "foo4");
			duk_get_prop_literal(ctx, 0, "foo5");
			duk_get_prop_literal(ctx, 0, "foo6");
			duk_get_prop_literal(ctx, 0, "foo7");
			duk_get_prop_literal(ctx, 0, "foo8");
			duk_get_prop_literal(ctx, 0, "foo9");
			duk_get_prop_literal(ctx, 0, "foo10");
			break;
		case 13:
			duk_push_uint(ctx, 123);
			duk_put_prop_heapptr(ctx, 0, hptr1);
			duk_push_uint(ctx, 123);
			duk_put_prop_heapptr(ctx, 0, hptr2);
			duk_push_uint(ctx, 123);
			duk_put_prop_heapptr(ctx, 0, hptr3);
			duk_push_uint(ctx, 123);
			duk_put_prop_heapptr(ctx, 0, hptr4);
			duk_push_uint(ctx, 123);
			duk_put_prop_heapptr(ctx, 0, hptr5);
			duk_push_uint(ctx, 123);
			duk_put_prop_heapptr(ctx, 0, hptr6);
			duk_push_uint(ctx, 123);
			duk_put_prop_heapptr(ctx, 0, hptr7);
			duk_push_uint(ctx, 123);
			duk_put_prop_heapptr(ctx, 0, hptr8);
			duk_push_uint(ctx, 123);
			duk_put_prop_heapptr(ctx, 0, hptr9);
			duk_push_uint(ctx, 123);
			duk_put_prop_heapptr(ctx, 0, hptr10);
			duk_push_uint(ctx, 123);
			duk_get_prop_heapptr(ctx, 0, hptr1);
			duk_get_prop_heapptr(ctx, 0, hptr2);
			duk_get_prop_heapptr(ctx, 0, hptr3);
			duk_get_prop_heapptr(ctx, 0, hptr4);
			duk_get_prop_heapptr(ctx, 0, hptr5);
			duk_get_prop_heapptr(ctx, 0, hptr6);
			duk_get_prop_heapptr(ctx, 0, hptr7);
			duk_get_prop_heapptr(ctx, 0, hptr8);
			duk_get_prop_heapptr(ctx, 0, hptr9);
			duk_get_prop_heapptr(ctx, 0, hptr10);
			break;
		case 20:
			duk_push_uint(ctx, 123);
			duk_put_global_string(ctx, "foo1");
			duk_push_uint(ctx, 123);
			duk_put_global_string(ctx, "foo2");
			duk_push_uint(ctx, 123);
			duk_put_global_string(ctx, "foo3");
			duk_push_uint(ctx, 123);
			duk_put_global_string(ctx, "foo4");
			duk_push_uint(ctx, 123);
			duk_put_global_string(ctx, "foo5");
			duk_push_uint(ctx, 123);
			duk_put_global_string(ctx, "foo6");
			duk_push_uint(ctx, 123);
			duk_put_global_string(ctx, "foo7");
			duk_push_uint(ctx, 123);
			duk_put_global_string(ctx, "foo8");
			duk_push_uint(ctx, 123);
			duk_put_global_string(ctx, "foo9");
			duk_push_uint(ctx, 123);
			duk_put_global_string(ctx, "foo10");
			duk_push_uint(ctx, 123);
			duk_get_global_string(ctx, "foo1");
			duk_get_global_string(ctx, "foo2");
			duk_get_global_string(ctx, "foo3");
			duk_get_global_string(ctx, "foo4");
			duk_get_global_string(ctx, "foo5");
			duk_get_global_string(ctx, "foo6");
			duk_get_global_string(ctx, "foo7");
			duk_get_global_string(ctx, "foo8");
			duk_get_global_string(ctx, "foo9");
			duk_get_global_string(ctx, "foo10");
			break;
		case 21:
			duk_push_uint(ctx, 123);
			duk_put_global_lstring(ctx, "foo1", 4);
			duk_push_uint(ctx, 123);
			duk_put_global_lstring(ctx, "foo2", 4);
			duk_push_uint(ctx, 123);
			duk_put_global_lstring(ctx, "foo3", 4);
			duk_push_uint(ctx, 123);
			duk_put_global_lstring(ctx, "foo4", 4);
			duk_push_uint(ctx, 123);
			duk_put_global_lstring(ctx, "foo5", 4);
			duk_push_uint(ctx, 123);
			duk_put_global_lstring(ctx, "foo6", 4);
			duk_push_uint(ctx, 123);
			duk_put_global_lstring(ctx, "foo7", 4);
			duk_push_uint(ctx, 123);
			duk_put_global_lstring(ctx, "foo8", 4);
			duk_push_uint(ctx, 123);
			duk_put_global_lstring(ctx, "foo9", 4);
			duk_push_uint(ctx, 123);
			duk_put_global_lstring(ctx, "foo10", 5);
			duk_push_uint(ctx, 123);
			duk_get_global_lstring(ctx, "foo1", 4);
			duk_get_global_lstring(ctx, "foo2", 4);
			duk_get_global_lstring(ctx, "foo3", 4);
			duk_get_global_lstring(ctx, "foo4", 4);
			duk_get_global_lstring(ctx, "foo5", 4);
			duk_get_global_lstring(ctx, "foo6", 4);
			duk_get_global_lstring(ctx, "foo7", 4);
			duk_get_global_lstring(ctx, "foo8", 4);
			duk_get_global_lstring(ctx, "foo9", 4);
			duk_get_global_lstring(ctx, "foo10", 5);
			break;
		case 22:
			duk_push_uint(ctx, 123);
			duk_put_global_literal(ctx, "foo1");
			duk_push_uint(ctx, 123);
			duk_put_global_literal(ctx, "foo2");
			duk_push_uint(ctx, 123);
			duk_put_global_literal(ctx, "foo3");
			duk_push_uint(ctx, 123);
			duk_put_global_literal(ctx, "foo4");
			duk_push_uint(ctx, 123);
			duk_put_global_literal(ctx, "foo5");
			duk_push_uint(ctx, 123);
			duk_put_global_literal(ctx, "foo6");
			duk_push_uint(ctx, 123);
			duk_put_global_literal(ctx, "foo7");
			duk_push_uint(ctx, 123);
			duk_put_global_literal(ctx, "foo8");
			duk_push_uint(ctx, 123);
			duk_put_global_literal(ctx, "foo9");
			duk_push_uint(ctx, 123);
			duk_put_global_literal(ctx, "foo10");
			duk_push_uint(ctx, 123);
			duk_get_global_literal(ctx, "foo1");
			duk_get_global_literal(ctx, "foo2");
			duk_get_global_literal(ctx, "foo3");
			duk_get_global_literal(ctx, "foo4");
			duk_get_global_literal(ctx, "foo5");
			duk_get_global_literal(ctx, "foo6");
			duk_get_global_literal(ctx, "foo7");
			duk_get_global_literal(ctx, "foo8");
			duk_get_global_literal(ctx, "foo9");
			duk_get_global_literal(ctx, "foo10");
			break;
		case 23:
			duk_push_uint(ctx, 123);
			duk_put_global_heapptr(ctx, hptr1);
			duk_push_uint(ctx, 123);
			duk_put_global_heapptr(ctx, hptr2);
			duk_push_uint(ctx, 123);
			duk_put_global_heapptr(ctx, hptr3);
			duk_push_uint(ctx, 123);
			duk_put_global_heapptr(ctx, hptr4);
			duk_push_uint(ctx, 123);
			duk_put_global_heapptr(ctx, hptr5);
			duk_push_uint(ctx, 123);
			duk_put_global_heapptr(ctx, hptr6);
			duk_push_uint(ctx, 123);
			duk_put_global_heapptr(ctx, hptr7);
			duk_push_uint(ctx, 123);
			duk_put_global_heapptr(ctx, hptr8);
			duk_push_uint(ctx, 123);
			duk_put_global_heapptr(ctx, hptr9);
			duk_push_uint(ctx, 123);
			duk_put_global_heapptr(ctx, hptr10);
			duk_push_uint(ctx, 123);
			duk_get_global_heapptr(ctx, hptr1);
			duk_get_global_heapptr(ctx, hptr2);
			duk_get_global_heapptr(ctx, hptr3);
			duk_get_global_heapptr(ctx, hptr4);
			duk_get_global_heapptr(ctx, hptr5);
			duk_get_global_heapptr(ctx, hptr6);
			duk_get_global_heapptr(ctx, hptr7);
			duk_get_global_heapptr(ctx, hptr8);
			duk_get_global_heapptr(ctx, hptr9);
			duk_get_global_heapptr(ctx, hptr10);
			break;
		default:
			return 1;
		}

		/* Keep strings and test object. */
		duk_set_top(ctx, 11);
	}

	duk_destroy_heap(ctx);
	return 0;
}
