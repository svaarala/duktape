#ifndef DUK_REPLACEMENTS_H_INCLUDED
#define DUK_REPLACEMENTS_H_INCLUDED

#ifdef DUK_USE_REPL_FPCLASSIFY
int duk_repl_fpclassify(double x);
#endif

#ifdef DUK_USE_REPL_SIGNBIT
int duk_repl_signbit(double x);
#endif

#ifdef DUK_USE_REPL_ISFINITE
int duk_repl_isfinite(double x);
#endif

#endif  /* DUK_REPLACEMENTS_H_INCLUDED */
