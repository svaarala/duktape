/*
 *  Selftest code
 */

#ifndef DUK_SELFTEST_H_INCLUDED
#define DUK_SELFTEST_H_INCLUDED

#if defined(DUK_USE_SELF_TESTS)
DUK_INTERNAL_DECL void duk_selftest_run_tests(void);
#endif

#endif  /* DUK_SELFTEST_H_INCLUDED */
