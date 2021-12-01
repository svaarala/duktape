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
 *  ECMAScript requires support for 32-bit long strings.  However, since each
 *  16-bit codepoint can take 3 bytes in CESU-8, this representation can only
 *  support about 1.4G codepoint long strings in extreme cases.  This is not
 *  really a practical issue.
 */

#if !defined(DUK_HSTRING_H_INCLUDED)
#define DUK_HSTRING_H_INCLUDED

/* Impose a maximum string length for now.  Restricted artificially to
 * ensure adding a heap header length won't overflow size_t.  The limit
 * should be synchronized with DUK_HBUFFER_MAX_BYTELEN.
 *
 * E5.1 makes provisions to support strings longer than 4G characters.
 * This limit should be eliminated on 64-bit platforms (and increased
 * closer to maximum support on 32-bit platforms).
 */

#if defined(DUK_USE_STRLEN16)
#define DUK_HSTRING_MAX_BYTELEN (0x0000ffffUL)
#else
#define DUK_HSTRING_MAX_BYTELEN (0x7fffffffUL)
#endif

/* XXX: could add flags for "is valid CESU-8" (ECMAScript compatible strings),
 * "is valid UTF-8", "is valid extended UTF-8" (internal strings are not,
 * regexp bytecode is), and "contains non-BMP characters".  These are not
 * needed right now.
 */

/* With lowmem builds the high 16 bits of duk_heaphdr are used for other
 * purposes, so this leaves 7 duk_heaphdr flags and 9 duk_hstring flags.
 */
#define DUK_HSTRING_FLAG_ASCII  DUK_HEAPHDR_USER_FLAG(0) /* string is ASCII, clen == blen */
#define DUK_HSTRING_FLAG_ARRIDX DUK_HEAPHDR_USER_FLAG(1) /* string is a valid array index */
#define DUK_HSTRING_FLAG_SYMBOL DUK_HEAPHDR_USER_FLAG(2) /* string is a symbol (invalid utf-8) */
#define DUK_HSTRING_FLAG_HIDDEN \
	DUK_HEAPHDR_USER_FLAG(3) /* string is a hidden symbol (implies symbol, Duktape 1.x internal string) */
#define DUK_HSTRING_FLAG_RESERVED_WORD        DUK_HEAPHDR_USER_FLAG(4) /* string is a reserved word (non-strict) */
#define DUK_HSTRING_FLAG_STRICT_RESERVED_WORD DUK_HEAPHDR_USER_FLAG(5) /* string is a reserved word (strict) */
#define DUK_HSTRING_FLAG_EVAL_OR_ARGUMENTS    DUK_HEAPHDR_USER_FLAG(6) /* string is 'eval' or 'arguments' */
#define DUK_HSTRING_FLAG_EXTDATA              DUK_HEAPHDR_USER_FLAG(7) /* string data is external (duk_hstring_external) */
#define DUK_HSTRING_FLAG_PINNED_LITERAL       DUK_HEAPHDR_USER_FLAG(8) /* string is a literal, and pinned */

#define DUK_HSTRING_HAS_ASCII(x)                DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_ASCII)
#define DUK_HSTRING_HAS_ARRIDX(x)               DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_ARRIDX)
#define DUK_HSTRING_HAS_SYMBOL(x)               DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_SYMBOL)
#define DUK_HSTRING_HAS_HIDDEN(x)               DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_HIDDEN)
#define DUK_HSTRING_HAS_RESERVED_WORD(x)        DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_RESERVED_WORD)
#define DUK_HSTRING_HAS_STRICT_RESERVED_WORD(x) DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_STRICT_RESERVED_WORD)
#define DUK_HSTRING_HAS_EVAL_OR_ARGUMENTS(x)    DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_EVAL_OR_ARGUMENTS)
#define DUK_HSTRING_HAS_EXTDATA(x)              DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_EXTDATA)
#define DUK_HSTRING_HAS_PINNED_LITERAL(x)       DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_PINNED_LITERAL)

#define DUK_HSTRING_SET_ASCII(x)                DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_ASCII)
#define DUK_HSTRING_SET_ARRIDX(x)               DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_ARRIDX)
#define DUK_HSTRING_SET_SYMBOL(x)               DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_SYMBOL)
#define DUK_HSTRING_SET_HIDDEN(x)               DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_HIDDEN)
#define DUK_HSTRING_SET_RESERVED_WORD(x)        DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_RESERVED_WORD)
#define DUK_HSTRING_SET_STRICT_RESERVED_WORD(x) DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_STRICT_RESERVED_WORD)
#define DUK_HSTRING_SET_EVAL_OR_ARGUMENTS(x)    DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_EVAL_OR_ARGUMENTS)
#define DUK_HSTRING_SET_EXTDATA(x)              DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_EXTDATA)
#define DUK_HSTRING_SET_PINNED_LITERAL(x)       DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_PINNED_LITERAL)

#define DUK_HSTRING_CLEAR_ASCII(x)                DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_ASCII)
#define DUK_HSTRING_CLEAR_ARRIDX(x)               DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_ARRIDX)
#define DUK_HSTRING_CLEAR_SYMBOL(x)               DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_SYMBOL)
#define DUK_HSTRING_CLEAR_HIDDEN(x)               DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_HIDDEN)
#define DUK_HSTRING_CLEAR_RESERVED_WORD(x)        DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_RESERVED_WORD)
#define DUK_HSTRING_CLEAR_STRICT_RESERVED_WORD(x) DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_STRICT_RESERVED_WORD)
#define DUK_HSTRING_CLEAR_EVAL_OR_ARGUMENTS(x)    DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_EVAL_OR_ARGUMENTS)
#define DUK_HSTRING_CLEAR_EXTDATA(x)              DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_EXTDATA)
#define DUK_HSTRING_CLEAR_PINNED_LITERAL(x)       DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HSTRING_FLAG_PINNED_LITERAL)

/* Marker value; in E5 2^32-1 is not a valid array index (2^32-2 is highest
 * valid).
 */
#define DUK_HSTRING_NO_ARRAY_INDEX (0xffffffffUL)

/* XXX: these actually fit into duk_hstring */
#define DUK_SYMBOL_TYPE_HIDDEN    0
#define DUK_SYMBOL_TYPE_GLOBAL    1
#define DUK_SYMBOL_TYPE_LOCAL     2
#define DUK_SYMBOL_TYPE_WELLKNOWN 3

/* Assertion for duk_hstring validity. */
#if defined(DUK_USE_ASSERTIONS)
DUK_INTERNAL_DECL void duk_hstring_assert_valid(duk_hstring *h);
#define DUK_HSTRING_ASSERT_VALID(h) \
	do { \
		duk_hstring_assert_valid((h)); \
	} while (0)
#else
#define DUK_HSTRING_ASSERT_VALID(h) \
	do { \
	} while (0)
#endif

/*
 *  Misc
 */

struct duk_hstring {
	/* Smaller heaphdr than for other objects, because strings are held
	 * in string intern table which requires no link pointers.  Much of
	 * the 32-bit flags field is unused by flags, so we can stuff a 16-bit
	 * field in there.
	 */
	duk_heaphdr_string hdr;

	/* String hash. */
#if defined(DUK_USE_STRHASH16)
	/* If 16-bit hash is in use, stuff it into duk_heaphdr_string flags. */
#else
	duk_uint32_t hash;
#endif

	/* Precomputed array index (or DUK_HSTRING_NO_ARRAY_INDEX). */
#if defined(DUK_USE_HSTRING_ARRIDX)
	duk_uarridx_t arridx;
#endif

	/* Length in bytes (not counting NUL term). */
#if defined(DUK_USE_STRLEN16)
	/* placed in duk_heaphdr_string */
#else
	duk_uint32_t blen;
#endif

	/* Length in codepoints (must be E5 compatible). */
#if defined(DUK_USE_STRLEN16)
#if defined(DUK_USE_HSTRING_CLEN)
	duk_uint16_t clen16;
#else
	/* computed live */
#endif
#else
	duk_uint32_t clen;
#endif

	/*
	 *  String data of 'blen+1' bytes follows (+1 for NUL termination
	 *  convenience for C API).  No alignment needs to be guaranteed
	 *  for strings, but fields above should guarantee alignment-by-4
	 *  (but not alignment-by-8).
	 */
};

/* The external string struct is defined even when the feature is inactive. */
struct duk_hstring_external {
	duk_hstring str;

	/*
	 *  For an external string, the NUL-terminated string data is stored
	 *  externally.  The user must guarantee that data behind this pointer
	 *  doesn't change while it's used.
	 */

	const duk_uint8_t *extdata;
};

/*
 *  Prototypes
 */

DUK_INTERNAL_DECL duk_bool_t duk_hstring_is_ascii(duk_hstring *h);
DUK_INTERNAL_DECL duk_bool_t duk_hstring_is_empty(duk_hstring *h);
DUK_INTERNAL_DECL duk_uint32_t duk_hstring_get_hash(duk_hstring *h);
DUK_INTERNAL_DECL void duk_hstring_set_hash(duk_hstring *h, duk_uint32_t hash);
DUK_INTERNAL_DECL duk_size_t duk_hstring_get_bytelen(duk_hstring *h);
DUK_INTERNAL_DECL void duk_hstring_set_bytelen(duk_hstring *h, duk_size_t len);
DUK_INTERNAL_DECL duk_size_t duk_hstring_get_charlen(duk_hstring *h);
#if !defined(DUK_USE_HSTRING_LAZY_CLEN)
DUK_INTERNAL_DECL void duk_hstring_init_charlen(duk_hstring *h);
#endif
/* No duk_hstring_set_charlen(), set via duk_hstring_init_charlen(). */
DUK_INTERNAL_DECL duk_uarridx_t duk_hstring_get_arridx_fast(duk_hstring *h);
DUK_INTERNAL_DECL duk_uarridx_t duk_hstring_get_arridx_fast_known(duk_hstring *h);
DUK_INTERNAL_DECL duk_uarridx_t duk_hstring_get_arridx_slow(duk_hstring *h);
#if defined(DUK_USE_HSTRING_EXTDATA)
DUK_INTERNAL_DECL const duk_uint8_t *duk_hstring_get_extdata(duk_hstring *h);
#endif
DUK_INTERNAL_DECL const duk_uint8_t *duk_hstring_get_data(duk_hstring *h);
DUK_INTERNAL_DECL const duk_uint8_t *duk_hstring_get_data_and_bytelen(duk_hstring *h, duk_size_t *out_blen);
DUK_INTERNAL_DECL const duk_uint8_t *duk_hstring_get_data_end(duk_hstring *h);

DUK_INTERNAL_DECL duk_ucodepoint_t duk_hstring_char_code_at_raw(duk_hthread *thr,
                                                                duk_hstring *h,
                                                                duk_uint_t pos,
                                                                duk_bool_t surrogate_aware);
DUK_INTERNAL_DECL duk_bool_t duk_hstring_equals_ascii_cstring(duk_hstring *h, const char *cstr);

#endif /* DUK_HSTRING_H_INCLUDED */
