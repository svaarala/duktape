/*
 *  Heap string representation.
 *
 *  Strings are byte sequences ordinarily stored in extended UTF-8 format,
 *  allowing values larger than the official UTF-8 range (used internally)
 *  and also allowing UTF-8 encoding of surrogate pairs (CESU-8 format).
 *  Strings may also be invalid UTF-8 altogether which is the case e.g. with
 *  strings used as internal property names and raw buffers converted to
 *  strings.  In such cases the 'clen' field contains an inaccurate value.
 *
 *  Ecmascript requires support for 32-bit long strings.  However, since each
 *  16-bit codepoint can take 3 bytes in CESU-8, this representation can only
 *  support about 1.4G codepoint long strings in extreme cases.  This is not
 *  really a practical issue.
 */

#ifndef DUK_HSTRING_H_INCLUDED
#define DUK_HSTRING_H_INCLUDED

/* Impose a maximum string length for now.  Restricted artificially to
 * ensure adding a heap header length won't overflow size_t.  The limit
 * should be synchronized with DUK_HBUFFER_MAX_BYTELEN.
 *
 * E5.1 makes provisions to support strings longer than 4G characters.
 * This limit should be eliminated on 64-bit platforms (and increased
 * closer to maximum support on 32-bit platforms).
 */
#define DUK_HSTRING_MAX_BYTELEN                     (0x7fffffffUL)

/* XXX: could add flags for "is valid CESU-8" (Ecmascript compatible strings),
 * "is valid UTF-8", "is valid extended UTF-8" (internal strings are not,
 * regexp bytecode is), and "contains non-BMP characters".  These are not
 * needed right now.
 */

#define DUK_HSTRING_FLAG_ARRIDX                     DUK_HEAPHDR_USER_FLAG(0)  /* string is a valid array index */
#define DUK_HSTRING_FLAG_INTERNAL                   DUK_HEAPHDR_USER_FLAG(1)  /* string is internal */
#define DUK_HSTRING_FLAG_RESERVED_WORD              DUK_HEAPHDR_USER_FLAG(2)  /* string is a reserved word (non-strict) */
#define DUK_HSTRING_FLAG_STRICT_RESERVED_WORD       DUK_HEAPHDR_USER_FLAG(3)  /* string is a reserved word (strict) */
#define DUK_HSTRING_FLAG_EVAL_OR_ARGUMENTS          DUK_HEAPHDR_USER_FLAG(4)  /* string is 'eval' or 'arguments' */

#define DUK_HSTRING_HAS_ARRIDX(x)                   DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_ARRIDX)
#define DUK_HSTRING_HAS_INTERNAL(x)                 DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_INTERNAL)
#define DUK_HSTRING_HAS_RESERVED_WORD(x)            DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_RESERVED_WORD)
#define DUK_HSTRING_HAS_STRICT_RESERVED_WORD(x)     DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_STRICT_RESERVED_WORD)
#define DUK_HSTRING_HAS_EVAL_OR_ARGUMENTS(x)        DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_EVAL_OR_ARGUMENTS)

#define DUK_HSTRING_SET_ARRIDX(x)                   DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_ARRIDX)
#define DUK_HSTRING_SET_INTERNAL(x)                 DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_INTERNAL)
#define DUK_HSTRING_SET_RESERVED_WORD(x)            DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_RESERVED_WORD)
#define DUK_HSTRING_SET_STRICT_RESERVED_WORD(x)     DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_STRICT_RESERVED_WORD)
#define DUK_HSTRING_SET_EVAL_OR_ARGUMENTS(x)        DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_EVAL_OR_ARGUMENTS)

#define DUK_HSTRING_CLEAR_ARRIDX(x)                 DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_ARRIDX)
#define DUK_HSTRING_CLEAR_INTERNAL(x)               DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_INTERNAL)
#define DUK_HSTRING_CLEAR_RESERVED_WORD(x)          DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_RESERVED_WORD)
#define DUK_HSTRING_CLEAR_STRICT_RESERVED_WORD(x)   DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_STRICT_RESERVED_WORD)
#define DUK_HSTRING_CLEAR_EVAL_OR_ARGUMENTS(x)      DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_EVAL_OR_ARGUMENTS)

#define DUK_HSTRING_IS_ASCII(x)                     ((x)->blen == (x)->clen)
#define DUK_HSTRING_IS_EMPTY(x)                     ((x)->blen == 0)

#define DUK_HSTRING_GET_HASH(x)                     ((x)->hash)
#define DUK_HSTRING_GET_BYTELEN(x)                  ((x)->blen)
#define DUK_HSTRING_GET_CHARLEN(x)                  ((x)->clen)
#define DUK_HSTRING_GET_DATA(x)                     ((duk_uint8_t *) ((x) + 1))
#define DUK_HSTRING_GET_DATA_END(x)                 (((duk_uint8_t *) ((x) + 1)) + ((x)->blen))

/* marker value; in E5 2^32-1 is not a valid array index (2^32-2 is highest valid) */
#define DUK_HSTRING_NO_ARRAY_INDEX  (0xffffffffUL)

/* get array index related to string (or return DUK_HSTRING_NO_ARRAY_INDEX);
 * avoids helper call if string has no array index value.
 */
#define DUK_HSTRING_GET_ARRIDX_FAST(h)  \
	(DUK_HSTRING_HAS_ARRIDX((h)) ? duk_js_to_arrayindex_string_helper((h)) : DUK_HSTRING_NO_ARRAY_INDEX)

/* slower but more compact variant */
#define DUK_HSTRING_GET_ARRIDX_SLOW(h)  \
	(duk_js_to_arrayindex_string_helper((h)))

/*
 *  Misc
 */

struct duk_hstring {
	/* smaller heaphdr than for other objects, because strings are held
	 * in string intern table which requires no link pointers.
	 */
	duk_heaphdr_string hdr;

	/* Note: we could try to stuff a partial hash (e.g. 16 bits) into the
	 * shared heap header.  Good hashing needs more hash bits though.
	 */

	duk_uint32_t hash;         /* string hash */
	duk_uint32_t blen;         /* length in bytes (not counting NUL term) */
	duk_uint32_t clen;         /* length in codepoints (must be E5 compatible) */

	/*
	 *  String value of 'blen+1' bytes follows (+1 for NUL termination
	 *  convenience for C API).  No alignment needs to be guaranteed
	 *  for strings, but fields above should guarantee alignment-by-4
	 *  (but not alignment-by-8).
	 */
};

/*
 *  Prototypes
 */

DUK_INTERNAL_DECL duk_ucodepoint_t duk_hstring_char_code_at_raw(duk_hthread *thr, duk_hstring *h, duk_uint_t pos);

#endif  /* DUK_HSTRING_H_INCLUDED */
