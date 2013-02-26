/*
 *  Run an duk_hobject finalizer.  Used for both reference counting
 *  and mark-and-sweep algorithms.  Must never throw an error.
 *
 *  There is no return value.  Any return value or error thrown by
 *  the finalizer is ignored (although errors are debug logged).
 *
 *  Notes:
 *
 *    - The thread used for calling the finalizer is the same as the
 *      'thr' argument.  This may need to change later.
 *
 *    - The finalizer thread 'top' assertions are there because it is
 *      critical that strict stack policy is observed (i.e. no cruft
 *      left on the finalizer stack).
 */

#include "duk_internal.h"

static int _finalize_helper(duk_context *ctx) {
	DUK_ASSERT(ctx != NULL);

	DUK_DDDPRINT("protected finalization helper running");

	/* [... obj] */

	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_FINALIZER);  /* -> [... obj finalizer] */
	if (!duk_is_callable(ctx, -1)) {
		DUK_DDDPRINT("-> no finalizer or finalizer not callable");
		return 0;
	}
	duk_dup(ctx, -2);  /* -> [... obj finalizer obj] */
	DUK_DDDPRINT("-> finalizer found, calling finalizer");
	duk_call(ctx, 1);  /* -> [... obj retval] */
	DUK_DDDPRINT("finalizer finished successfully");
	return 0;

	/* Note: we rely on duk_safe_call() to fix up the stack for the caller,
	 * so we don't need to pop stuff here.  There is no return value;
	 * caller determines rescued status based on object refcount.
	 */
}

void duk_hobject_run_finalizer(duk_hthread *thr, duk_hobject *obj) {
	duk_context *ctx = (duk_context *) thr;
	int rc;
#ifdef DUK_USE_ASSERTIONS
	int entry_top;
#endif

	DUK_DDDPRINT("running object finalizer for object: %p", (void *) obj);

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(obj != NULL);

	/* FIXME: assert stack space */

#ifdef DUK_USE_ASSERTIONS
	entry_top = duk_get_top(ctx);
#endif
	/*
	 *  Get and call the finalizer.  All of this must be wrapped
	 *  in a protected call, because even getting the finalizer
	 *  may trigger an error (getter may throw one, for instance).
	 */

	/* FIXME: use a NULL error handler for the finalizer call? */

	DUK_DDDPRINT("-> finalizer found, calling wrapped finalize helper");
	duk_push_hobject(ctx, obj);  /* this also increases refcount by one */
	rc = duk_safe_call(ctx, _finalize_helper, 0 /*nargs*/, 1 /*nrets*/, DUK_INVALID_INDEX);  /* -> [... obj retval/error] */
	DUK_ASSERT(duk_get_top(ctx) == entry_top + 2);  /* duk_safe_call discipline */

	if (rc != DUK_ERR_EXEC_SUCCESS) {
		/* Note: we ask for one return value from duk_safe_call to get this
		 * error debugging here.
		 */
		DUK_DPRINT("wrapped finalizer call failed for object %p (ignored); error: %!T",
		           (void *) obj, duk_get_tval(ctx, -1));
	}
	duk_pop_2(ctx);  /* -> [...] */

	DUK_ASSERT(duk_get_top(ctx) == entry_top);
}

