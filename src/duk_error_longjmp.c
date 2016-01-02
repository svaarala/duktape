/*
 *  Do a longjmp call, calling the fatal error handler if no
 *  catchpoint exists.
 */

#include "duk_internal.h"

DUK_INTERNAL void duk_err_longjmp(duk_hthread *thr) {
	DUK_ASSERT(thr != NULL);

#if defined(DUK_USE_CPP_EXCEPTIONS)
	/* XXX: detecting uncaught exception case for C++ case; perhaps need
	 * some marker in heap->lj state that a try-catch is active.  For now,
	 * invokes C++ uncaught exception handling.
	 */
#else
	if (!thr->heap->lj.jmpbuf_ptr) {
		/*
		 *  If we don't have a jmpbuf_ptr, there is little we can do
		 *  except panic.  The caller's expectation is that we never
		 *  return.
		 */

		DUK_D(DUK_DPRINT("uncaught error: type=%d iserror=%d value1=%!T value2=%!T",
		                 (int) thr->heap->lj.type, (int) thr->heap->lj.iserror,
		                 &thr->heap->lj.value1, &thr->heap->lj.value2));

		duk_fatal((duk_context *) thr, DUK_ERR_UNCAUGHT_ERROR, "uncaught error");
		DUK_UNREACHABLE();
	}
#endif

#if defined(DUK_USE_CPP_EXCEPTIONS)
	{
		duk_internal_exception exc;  /* dummy */
		throw exc;
	}
#else
	DUK_LONGJMP(thr->heap->lj.jmpbuf_ptr->jb);
#endif
	DUK_UNREACHABLE();
}
