/*
 *  Heap native function representation.
 */

#ifndef DUK_HNATIVEFUNCTION_H_INCLUDED
#define DUK_HNATIVEFUNCTION_H_INCLUDED

#define  DUK_HNATIVEFUNCTION_NARGS_VARARGS  ((duk_i16) -1)
#define  DUK_HNATIVEFUNCTION_NARGS_MAX      ((duk_i16) 0x7fff)

struct duk_hnativefunction {
	/* shared object part */
	duk_hobject obj;

	duk_c_function func;
	duk_i16 nargs;

	/* XXX: there is a nice 16-bit space here.  What to put here?
	 *
	 * One alternative: put a 16-bit 'magic' (or 'salt') here, and allow
	 * C code to get the 'magic' value of their wrapping duk_hnativefunction.
	 * This would allow the same C functions to be used internally, while
	 * flags and small parameter fields could be given through the 'magic'
	 * value.  For instance, there are a bunch of setter/getter functions
	 * in the Date built-in which only differ in a few flags.
	 */

	/* Note: cannot place nargs/magic into the heaphdr flags, because
	 * duk_hobject takes almost all flags already (and needs the spare).
	 */
};

#endif  /* DUK_HNATIVEFUNCTION_H_INCLUDED */

