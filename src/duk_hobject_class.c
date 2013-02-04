/*
 *  Hobject Ecmascript [[Class]].
 */

#include "duk_internal.h"

/* Maybe better to check these elsewhere */
#if (DUK_HEAP_STRIDX_UC_ARGUMENTS > 255)
#error constant too large
#endif
#if (DUK_HEAP_STRIDX_ARRAY > 255)
#error constant too large
#endif
#if (DUK_HEAP_STRIDX_UC_BOOLEAN > 255)
#error constant too large
#endif
#if (DUK_HEAP_STRIDX_DATE > 255)
#error constant too large
#endif
#if (DUK_HEAP_STRIDX_ERROR > 255)
#error constant too large
#endif
#if (DUK_HEAP_STRIDX_UC_FUNCTION > 255)
#error constant too large
#endif
#if (DUK_HEAP_STRIDX_JSON > 255)
#error constant too large
#endif
#if (DUK_HEAP_STRIDX_MATH > 255)
#error constant too large
#endif
#if (DUK_HEAP_STRIDX_UC_NUMBER > 255)
#error constant too large
#endif
#if (DUK_HEAP_STRIDX_UC_OBJECT > 255)
#error constant too large
#endif
#if (DUK_HEAP_STRIDX_REG_EXP > 255)
#error constant too large
#endif
#if (DUK_HEAP_STRIDX_UC_STRING > 255)
#error constant too large
#endif
#if (DUK_HEAP_STRIDX_GLOBAL > 255)
#error constant too large
#endif
#if (DUK_HEAP_STRIDX_EMPTY_STRING > 255)
#error constant too large
#endif

/* Note: assumes that these string indexes are 8-bit, genstrings.py must ensure that */
duk_u8 duk_class_number_to_stridx[16] = {
	DUK_HEAP_STRIDX_EMPTY_STRING,  /* UNUSED, intentionally empty */
	DUK_HEAP_STRIDX_UC_ARGUMENTS,
	DUK_HEAP_STRIDX_ARRAY,
	DUK_HEAP_STRIDX_UC_BOOLEAN,
	DUK_HEAP_STRIDX_DATE,
	DUK_HEAP_STRIDX_ERROR,
	DUK_HEAP_STRIDX_UC_FUNCTION,
	DUK_HEAP_STRIDX_JSON,
	DUK_HEAP_STRIDX_MATH,
	DUK_HEAP_STRIDX_UC_NUMBER,
	DUK_HEAP_STRIDX_UC_OBJECT,
	DUK_HEAP_STRIDX_REG_EXP,
	DUK_HEAP_STRIDX_UC_STRING,
	DUK_HEAP_STRIDX_GLOBAL,
	DUK_HEAP_STRIDX_EMPTY_STRING,  /* OBJENV, intentionally empty */
	DUK_HEAP_STRIDX_EMPTY_STRING,  /* DECENV, intentionally empty */
};

