/*
 *  Duktape 2.0 and prior required that duk_push_heapptr() argument was fully
 *  reachable all the way between duk_get_heapptr() and duk_push_heapptr().
 *  This also meant that if an object became unreachable but its finalizer had
 *  not yet executed, it was an assertion violation to use duk_push_heapptr()
 *  for it.  Such a push also caused incorrect, memory unsafe behavior besides
 *  just assertion errors.
 *
 *  Duktape 2.1 allows the duk_push_heapptr() argument to be an object on the
 *  finalize_list, provided that the finalizer has not yet executed (it's OK
 *  for the finalizer to be executing, but not finished) without rescue.
 *
 *  There are a few separate cases here:
 *
 *  - Refzero collection: duk_push_heapptr() within the finalizer for the
 *    object itself.
 *
 *  - Refzero collection: duk_push_heapptr() in a finalizer for another
 *    object not yet finalized (but on finalize_list).
 *
 *  - Mark-and-sweep: duk_push_heapptr() for object being finalized.
 *
 *  - Mark-and-sweep: duk_push_heapptr() for another object on finalize_list.
 */

/*===
*** test_refcount (duk_safe_call)
trigger refzero
fin1 called
fin2 called
fin2 rescues ptr3
refzero done
fin3 called
fin1 called
null writes done
final top: 0
==> rc=0, result='undefined'
*** test_mark_and_sweep (duk_safe_call)
make unreachable
call duk_gc()
fin1 called
fin2 called
fin2 rescues ptr3
duk_gc returned()
null writes done
call duk_gc()
fin3 called
duk_gc returned()
final top: 0
==> rc=0, result='undefined'
*** test_mark_and_sweep_2 (duk_safe_call)
make unreachable
call duk_gc()
fin_norescue called
fin_norescue called
duk_gc returned()
final top: 0
==> rc=0, result='undefined'
===*/

static void *ptr1 = NULL;
static void *ptr2 = NULL;
static void *ptr3 = NULL;

static duk_ret_t fin1(duk_context *ctx) {
	printf("fin1 called\n");

	duk_push_heapptr(ctx, ptr1);
	duk_put_global_string(ctx, "rescue1");

	ptr1 = NULL;
	return 0;
}

static duk_ret_t fin2(duk_context *ctx) {
	printf("fin2 called\n");

	if (ptr3 != NULL) {
		printf("fin2 rescues ptr3\n");

		duk_push_heapptr(ctx, ptr3);
		duk_put_global_string(ctx, "rescue3");
	}

	ptr2 = NULL;
	return 0;
}

static duk_ret_t fin3(duk_context *ctx) {
	printf("fin3 called\n");

	if (ptr2 != NULL) {
		printf("fin3 rescues ptr2\n");

		duk_push_heapptr(ctx, ptr2);
		duk_put_global_string(ctx, "rescue2");
	}

	ptr3 = NULL;
	return 0;
}

static duk_ret_t fin_norescue(duk_context *ctx) {
	printf("fin_norescue called\n");

	duk_del_prop_string(ctx, 0, "ref");
	return 0;
}

static duk_ret_t test_refcount(duk_context *ctx, void *udata) {
	(void) udata;

	/* Refcount case:
	 *   - Object 1 rescues itself.
	 *   - Object 2 rescues object 3 or vice versa.  This depends on
	 *     the finalization order which is not guaranteed, so the test
	 *     is a bit fragile for this part.
	 */

	duk_push_object(ctx);
	duk_push_c_function(ctx, fin1, 1);
	duk_set_finalizer(ctx, -2);
	ptr1 = duk_get_heapptr(ctx, -1);

	duk_push_object(ctx);
	duk_push_c_function(ctx, fin2, 1);
	duk_set_finalizer(ctx, -2);
	ptr2 = duk_get_heapptr(ctx, -1);

	duk_push_object(ctx);
	duk_push_c_function(ctx, fin3, 1);
	duk_set_finalizer(ctx, -2);
	ptr3 = duk_get_heapptr(ctx, -1);

	printf("trigger refzero\n");
	duk_set_top(ctx, 0);  /* REFZERO for all objects at the same time. */
	printf("refzero done\n");

	duk_eval_string_noresult(ctx, "rescue1 = rescue2 = rescue3 = null;");

	printf("null writes done\n");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_mark_and_sweep(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string(ctx, "(function () { var obj = {}; obj.ref = {}; obj.ref.ref = obj; return obj; })()");
	duk_push_c_function(ctx, fin1, 1);
	duk_set_finalizer(ctx, -2);
	ptr1 = duk_get_heapptr(ctx, -1);

	duk_eval_string(ctx, "(function () { var obj = {}; obj.ref = {}; obj.ref.ref = obj; return obj; })()");
	duk_push_c_function(ctx, fin2, 1);
	duk_set_finalizer(ctx, -2);
	ptr2 = duk_get_heapptr(ctx, -1);

	duk_eval_string(ctx, "(function () { var obj = {}; obj.ref = {}; obj.ref.ref = obj; return obj; })()");
	duk_push_c_function(ctx, fin3, 1);
	duk_set_finalizer(ctx, -2);
	ptr3 = duk_get_heapptr(ctx, -1);

	printf("make unreachable\n");
	duk_set_top(ctx, 0);  /* Now unreachable. */
	printf("call duk_gc()\n");
	duk_gc(ctx, 0);
	printf("duk_gc returned()\n");

	duk_eval_string_noresult(ctx, "rescue1 = rescue2 = rescue3 = null;");

	printf("null writes done\n");

	printf("call duk_gc()\n");
	duk_gc(ctx, 0);
	printf("duk_gc returned()\n");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_mark_and_sweep_2(duk_context *ctx, void *udata) {
	(void) udata;

	/* Mark-and-sweep case: two objects with finalizers point to each
	 * other using '.ref'.  When an object is finalized, the other
	 * object gets refzero'ed.
	 */

	duk_push_object(ctx);
	duk_push_c_function(ctx, fin_norescue, 1);
	duk_set_finalizer(ctx, -2);

	duk_push_object(ctx);
	duk_push_c_function(ctx, fin_norescue, 1);
	duk_set_finalizer(ctx, -2);

	duk_dup(ctx, 1);
	duk_put_prop_string(ctx, 0, "ref");
	duk_dup(ctx, 0);
	duk_put_prop_string(ctx, 1, "ref");

	printf("make unreachable\n");
	duk_set_top(ctx, 0);  /* Now unreachable. */
	printf("call duk_gc()\n");
	duk_gc(ctx, 0);
	printf("duk_gc returned()\n");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_refcount);
	TEST_SAFE_CALL(test_mark_and_sweep);
	TEST_SAFE_CALL(test_mark_and_sweep_2);
}
