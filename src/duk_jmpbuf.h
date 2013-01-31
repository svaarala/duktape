/*
 *  Wrapper for jmp_buf.
 *
 *  This is used because jmp_buf is an array type for backward compatibility.
 *  Wrapping jmp_buf in a struct makes pointer references, sizeof, etc,
 *  behave more intuitively.
 *
 *  http://en.wikipedia.org/wiki/Setjmp.h#Member_types
 */

#ifndef __DUK_JMPBUF_H
#define __DUK_JMPBUF_H 1

#include <setjmp.h>

struct duk_jmpbuf {
	jmp_buf jb;
};

#endif  /* __DUK_JMPBUF_H */

