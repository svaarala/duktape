#ifndef __DUK_MISC_H
#define __DUK_MISC_H 1

/*
 *  Macro hackery to convert e.g. __LINE__ to a string without formatting,
 *  see: http://stackoverflow.com/questions/240353/convert-a-preprocessor-token-to-a-string
 */

#define  _DUK_STRINGIFY_HELPER(x)  #x
#define  DUK_MACRO_STRINGIFY(x)  _DUK_STRINGIFY_HELPER(x)

/*
 *  GCC specific compile time messages
 *
 *  Note: no semicolon should be used after these because they may appear e.g. at top level:
 *
 *      DUK_FIXME("this needs fixing")
 */

/* FIXME: make these conditional to a specific compiler option (don't want to see these normally) */

#if defined(__GNUC__) && defined(FIXME_COMMENTED_OUT)

/* http://gcc.gnu.org/onlinedocs/gcc-4.6.0/gcc/Diagnostic-Pragmas.html */
#define  _DUK_DO_PRAGMA(x)  _Pragma(#x)
#define  DUK_FIXME(x)       _DUK_DO_PRAGMA(message ("FIXME: " DUK_MACRO_STRINGIFY(x)))
#define  DUK_TODO(x)        _DUK_DO_PRAGMA(message ("TODO: " DUK_MACRO_STRINGIFY(x)))
#define  DUK_XXX(x)         _DUK_DO_PRAGMA(message ("XXX: " DUK_MACRO_STRINGIFY(x)))

#else

#define  DUK_FIXME(x)
#define  DUK_TODO(x)
#define  DUK_XXX(x)

#endif  /* __GNUC__ */

#endif  /* _DUK_MISC_H */

