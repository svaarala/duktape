/*
 *  Do a longjmp call, calling the fatal error handler if no
 *  catchpoint exists.
 */

#include "duk_internal.h"

DUK_INTERNAL void duk_err_longjmp(duk_hthread *thr) {
	DUK_ASSERT(thr != NULL);

	DUK_DD(DUK_DDPRINT("longjmp error: type=%d iserror=%d value1=%!T value2=%!T",
	                   (int) thr->heap->lj.type, (int) thr->heap->lj.iserror,
	                   &thr->heap->lj.value1, &thr->heap->lj.value2));

#if !defined(DUK_USE_CPP_EXCEPTIONS)
	/* If we don't have a jmpbuf_ptr, there is little we can do
	 * except panic.  The caller's expectation is that we never
	 * return.
	 *
	 * With C++ exceptions we now just propagate an uncaught error
	 * instead of invoking the fatal error handler.  Because there's
	 * a dummy jmpbuf for C++ exceptions now, this could be changed.
	 */
	if (!thr->heap->lj.jmpbuf_ptr) {

		DUK_D(DUK_DPRINT("uncaught error: type=%d iserror=%d value1=%!T value2=%!T",
		                 (int) thr->heap->lj.type, (int) thr->heap->lj.iserror,
		                 &thr->heap->lj.value1, &thr->heap->lj.value2));

		duk_fatal((duk_context *) thr, DUK_ERR_UNCAUGHT_ERROR, "uncaught error");
		DUK_UNREACHABLE();
	}
#endif  /* DUK_USE_CPP_EXCEPTIONS */

#if defined(DUK_USE_CPP_EXCEPTIONS)
	{
		duk_internal_exception exc;  /* dummy */
		throw exc;
	}
#else  /* DUK_USE_CPP_EXCEPTIONS */
	DUK_LONGJMP(thr->heap->lj.jmpbuf_ptr->jb);
#endif  /* DUK_USE_CPP_EXCEPTIONS */

	DUK_UNREACHABLE();
}
