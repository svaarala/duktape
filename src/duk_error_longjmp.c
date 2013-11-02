/*
 *  Do a longjmp call, calling the fatal error handler if no
 *  catchpoint exists.
 */

#include "duk_internal.h"

void duk_err_longjmp(duk_hthread *thr) {
	DUK_ASSERT(thr != NULL);

	if (!thr->heap->lj.jmpbuf_ptr) {
		/*
		 *  If we don't have a jmpbuf_ptr, there is little we can do
		 *  except panic.  The caller's expectation is that we never
		 *  return.
		 */

		duk_fatal((duk_context *) thr, DUK_ERR_UNCAUGHT_ERROR);
		DUK_UNREACHABLE();
	}

	longjmp(thr->heap->lj.jmpbuf_ptr->jb, DUK_LONGJMP_DUMMY_VALUE);
	DUK_UNREACHABLE();
}

