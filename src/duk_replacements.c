/*
 *  Replacements for missing platform functions.
 *
 *  Unlike the originals, fpclassify() and signbit() replacements don't
 *  work on any floating point types, only doubles.
 */

#ifdef DUK_USE_COMPUTED_NAN
double duk_computed_nan;
#endif

#ifdef DUK_USE_COMPUTED_INFINITY
double duk_computed_infinity;
#endif

#ifdef DUK_USE_REPL_FPCLASSIFY
int duk_repl_fpclassify(double x) {
	return 0;  /*FIXME*/
}
#endif

#ifdef DUK_USE_REPL_SIGNBIT
int duk_repl_signbit(double x) {
	return 0;  /*FIXME*/
}
#endif

#ifdef DUK_USE_REPL_ISFINITE
int duk_repl_isfinite(double x) {
	return 0;  /*FIXME*/
}
#endif

#ifdef DUK_USE_REPL_ISNAN
int duk_repl_isnan(double x) {
	return 0;  /*FIXME*/
}
#endif

