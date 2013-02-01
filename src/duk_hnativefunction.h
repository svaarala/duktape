/*
 *  Heap native function representation.
 */

#ifndef __DUK_HNATIVEFUNCTION_H
#define __DUK_HNATIVEFUNCTION_H 1

#define  DUK_HNATIVEFUNCTION_NARGS_VARARGS  ((duk_i16) -1)
#define  DUK_HNATIVEFUNCTION_NARGS_MAX      ((duk_i16) 0x7fff)

struct duk_hnativefunction {
	/* shared object part */
	duk_hobject obj;

	duk_c_function func;
	duk_i16 nargs;
	/* XXX: 16-bit space here */
};

#endif  /* __DUK_HNATIVEFUNCTION_H */

