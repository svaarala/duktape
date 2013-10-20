/*
 *  Sanity check of duk_features.h defines.  This is a separate file
 *  because we also check for consistency between duktape.h and
 *  duk_features.h defines.  Duktape.h cannot be included first
 *  because feature selection macros would then be incorrect.
 */

#ifndef DUK_FEATURES_SANITY_H_INCLUDED
#define DUK_FEATURES_SANITY_H_INCLUDED

#if defined(DUK_DDEBUG) && !defined(DUK_DEBUG)
#error DUK_DEBUG and DUK_DDEBUG should not be defined (obsolete)
#endif

#if defined(DUK_USE_DDEBUG) && !defined(DUK_USE_DEBUG)
#error DUK_USE_DDEBUG defined without DUK_USE_DEBUG
#endif

#if defined(DUK_USE_DDDEBUG) && !defined(DUK_USE_DEBUG)
#error DUK_USE_DDDEBUG defined without DUK_USE_DEBUG
#endif

#if defined(DUK_USE_DDDEBUG) && !defined(DUK_USE_DDEBUG)
#error DUK_USE_DDDEBUG defined without DUK_USE_DDEBUG
#endif

#if defined(DUK_USE_REFERENCE_COUNTING) && !defined(DUK_USE_DOUBLE_LINKED_HEAP)
#error DUK_USE_REFERENCE_COUNTING defined without DUK_USE_DOUBLE_LINKED_HEAP
#endif

#if defined(DUK_USE_GC_TORTURE) && !defined(DUK_USE_MARK_AND_SWEEP)
#error DUK_USE_GC_TORTURE defined without DUK_USE_MARK_AND_SWEEP
#endif

#if (defined(DUK_USE_VARIADIC_MACROS) && !defined(DUK_API_VARIADIC_MACROS)) || \
    (!defined(DUK_USE_VARIADIC_MACROS) && defined(DUK_API_VARIADIC_MACROS))
#error DUK_USE_VARIADIC_MACROS and DUK_API_VARIADIC_MACROS must agree
#endif

#endif  /* DUK_FEATURES_SANITY_H_INCLUDED */
