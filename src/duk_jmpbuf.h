/*
 *  Wrapper for jmp_buf.
 *
 *  This is used because jmp_buf is an array type for backward compatibility.
 *  Wrapping jmp_buf in a struct makes pointer references, sizeof, etc,
 *  behave more intuitively.
 *
 *  http://en.wikipedia.org/wiki/Setjmp.h#Member_types
 */

#ifndef DUK_JMPBUF_H_INCLUDED
#define DUK_JMPBUF_H_INCLUDED

struct duk_jmpbuf {
	jmp_buf jb;
};

#endif  /* DUK_JMPBUF_H_INCLUDED */
