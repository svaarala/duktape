/*
 *  JSON built-ins
 */

/*
 *  Serialization notes:
 *
 *    - It would be nice to change the standard algorithm to be based around
 *      a "serializeValue()" primitive.  The standard algorithm assumes access
 *      to the "holder" of the value, especially in E5 Section 15.12.3, Str()
 *      algoritm, step 3.a: the holder is passed to the ReplacerFunction.
 *      So, the implementation here is based on the standard algorithm set.
 *
 *    - Similarly, serialization of a value 'val' begins from a dummy wrapper
 *      object: { "": val }.  This seems to be quite awkward and unnecessary.
 *      However, the wrapper object is accessible to the ReplacerFunction!
 *
 *    - String serialization should be fast for pure ASCII strings as they
 *      are very common.  Unfortunately we may still need to escape characters
 *      in them, so there is no explicit fast path now.  We could use ordinary
 *      character lookups during serialization (note that ASCII string lookups
 *      would not affect the stringcache).  This would be quite slow, so we
 *      decode the extended UTF-8 directly instead.
 *
 *    - Strings may contain non-BMP characters.  These don't really need to be
 *      supported from a specification standpoint.  We could encode them as
 *      surrogate pairs, but that would only work up to U+10FFFF, whereas e.g.
 *      regexp bytecode may contain much higher values.
 *
 *      FIXME: surrogate pair + \uXXXX ?
 *      FIXME: custom escape: \UXXXXXXXX ?
 *      FIXME: non-escape hack: <U+XXXXXXXX> or something (not decodable)
 */

#include "duk_internal.h"

int duk_builtin_json_object_parse(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_json_object_stringify(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

