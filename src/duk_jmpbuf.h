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
#if defined(DUK_USE_SETJMP) || defined(DUK_USE_UNDERSCORE_SETJMP)
	jmp_buf jb;
#elif defined(DUK_USE_SIGSETJMP)
	sigjmp_buf jb;
#else
#error internal error, no long control transfer provider
#endif
};

#endif  /* DUK_JMPBUF_H_INCLUDED */
