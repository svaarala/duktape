#ifndef DUK_REPLACEMENTS_H_INCLUDED
#define DUK_REPLACEMENTS_H_INCLUDED

#ifdef DUK_USE_REPL_FPCLASSIFY
DUK_INTERNAL_DECL int duk_repl_fpclassify(double x);
#endif

#ifdef DUK_USE_REPL_SIGNBIT
DUK_INTERNAL_DECL int duk_repl_signbit(double x);
#endif

#ifdef DUK_USE_REPL_ISFINITE
DUK_INTERNAL_DECL int duk_repl_isfinite(double x);
#endif

#ifdef DUK_USE_REPL_ISNAN
DUK_INTERNAL_DECL int duk_repl_isnan(double x);
#endif

#ifdef DUK_USE_REPL_ISINF
DUK_INTERNAL_DECL int duk_repl_isinf(double x);
#endif

#endif  /* DUK_REPLACEMENTS_H_INCLUDED */
