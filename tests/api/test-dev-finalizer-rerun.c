/*
 *  A finalizer shouldn't be re-entered unless the finalizer explicitly
 *  rescued the object.
 */

/*===
*** test_heap_destruction (duk_safe_call)
creating heap
heap created
object 1 finalizer
destroying heap
heap destroyed
==> rc=0, result='undefined'
===*/

/* Create an object, force one round of GC and destroy heap immediately.
 * Object has been finalized but not yet rescued, and should not be
 * finalized again on destruction.
 */
static duk_ret_t test_heap_destruction(duk_context *ignored_ctx) {
	duk_context *my_ctx;

	printf("creating heap\n"); fflush(stdout);
	my_ctx = duk_create_heap_default();
	if (!my_ctx) {
		printf("failed to create heap\n"); fflush(stdout);
		return 0;
	}
	printf("heap created\n"); fflush(stdout);

	duk_eval_string_noresult(my_ctx,
		"(function () {\n"
		"    var obj1 = {}; var obj2 = {};\n"
		"    obj1.ref = obj2; obj2.ref = obj1;\n"
		"    Duktape.fin(obj1, function obj1fin() { print('object 1 finalizer'); });\n"
		"    obj1 = obj2 = null;\n"
		"    Duktape.gc();\n"
		"})()");

	printf("destroying heap\n"); fflush(stdout);
	duk_destroy_heap(my_ctx);
	printf("heap destroyed\n"); fflush(stdout);

	return 0;
}

#if 0  /* Disabled: this is too fragile because it relies on dangling references;
        * unfortunately the potential re-run scenario is difficult to confirm
        * otherwise.
        */
/* Create a circular reference with a finalizer, force a GC run which runs the
 * finalizer.  Use a dangling (!) C heaphdr reference to break the loop so that
 * refzero code processes the object again.  Finalization should not happen again
 * in the refcount path.
 */
static duk_ret_t test_markandsweep_finalize_then_refzero(duk_context *ignored_ctx) {
	duk_context *my_ctx;
	void *ptr;

	printf("creating heap\n"); fflush(stdout);
	my_ctx = duk_create_heap_default();
	if (!my_ctx) {
		printf("failed to create heap\n"); fflush(stdout);
		return 0;
	}
	printf("heap created\n"); fflush(stdout);

	duk_eval_string(my_ctx,
		"(function () {\n"
		"    var obj1 = {}; var obj2 = {};\n"
		"    obj1.ref = obj2; obj2.ref = obj1;\n"
		"    Duktape.fin(obj1, function obj1fin() { print('object 1 finalizer'); });\n"
		"    return obj1;\n"
		"})()");

	ptr = duk_get_heapptr(my_ctx, -1);
	duk_pop(my_ctx);
	if (!ptr) {
		printf("heap ptr was NULL\n"); fflush(stdout);
	} else {
		printf("forcing gc\n"); fflush(stdout);
		duk_gc(my_ctx, 0);
		printf("gc done\n"); fflush(stdout);

		/* Because 'ptr' has a finalizer it's been finalized but would
		 * require a second round of mark-and-sweep the be actually freed.
		 * So 'ptr' is reachable but technically dangling so this is not
		 * fully safe.  But it's safe enough to break the circular reference
		 * unless something else triggers GC before that.
		 */
		duk_push_heapptr(my_ctx, ptr);
		duk_del_prop_string(my_ctx, -1, "ref");
		printf("popping final reference after breaking cycle\n"); fflush(stdout);
		duk_pop(my_ctx);
		printf("pop completed\n"); fflush(stdout);
	}

	printf("destroying heap\n"); fflush(stdout);
	duk_destroy_heap(my_ctx);
	printf("heap destroyed\n"); fflush(stdout);

	return 0;
}
#endif  /* 0 */

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_heap_destruction);
#if 0
	TEST_SAFE_CALL(test_markandsweep_finalize_then_refzero);
#endif
}
