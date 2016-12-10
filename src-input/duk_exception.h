/*
 *  Exception for Duktape internal throws when C++ exceptions are used
 *  for long control transfers.
 *
 *  Doesn't inherit from any exception base class to minimize the chance
 *  that user code would accidentally catch this exception.
 */

#if !defined(DUK_EXCEPTION_H_INCLUDED)
#define DUK_EXCEPTION_H_INCLUDED

#if defined(DUK_USE_CPP_EXCEPTIONS)
class duk_internal_exception {
	/* intentionally empty */
};
#endif

#endif  /* DUK_EXCEPTION_H_INCLUDED */
