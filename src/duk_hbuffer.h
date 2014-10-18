/*
 *  Heap buffer representation.
 *
 *  Heap allocated user data buffer which is either:
 *
 *    1. A fixed size buffer (data follows header statically)
 *    2. A dynamic size buffer (data pointer follows header)
 *
 *  The data pointer for a variable size buffer of zero size may be NULL.
 */

#ifndef DUK_HBUFFER_H_INCLUDED
#define DUK_HBUFFER_H_INCLUDED

/* Impose a maximum buffer length for now.  Restricted artificially to
 * ensure resize computations or adding a heap header length won't
 * overflow size_t.  The limit should be synchronized with
 * DUK_HSTRING_MAX_BYTELEN.
 */
#define DUK_HBUFFER_MAX_BYTELEN                   (0x7fffffffUL)

#define DUK_HBUFFER_FLAG_DYNAMIC                  DUK_HEAPHDR_USER_FLAG(0)  /* buffer is resizable */

#define DUK_HBUFFER_HAS_DYNAMIC(x)                DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HBUFFER_FLAG_DYNAMIC)

#define DUK_HBUFFER_SET_DYNAMIC(x)                DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HBUFFER_FLAG_DYNAMIC)

#define DUK_HBUFFER_CLEAR_DYNAMIC(x)              DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HBUFFER_FLAG_DYNAMIC)

#define DUK_HBUFFER_FIXED_GET_DATA_PTR(x)         ((duk_uint8_t *) (((duk_hbuffer_fixed *) (x)) + 1))
#define DUK_HBUFFER_FIXED_GET_SIZE(x)             ((x)->u.s.size)

#define DUK_HBUFFER_DYNAMIC_GET_ALLOC_SIZE(x)     ((x)->usable_size)
#define DUK_HBUFFER_DYNAMIC_GET_USABLE_SIZE(x)    ((x)->usable_size)
#define DUK_HBUFFER_DYNAMIC_GET_SPARE_SIZE(x)     ((x)->usable_size - (x)->size)
#define DUK_HBUFFER_DYNAMIC_GET_CURR_DATA_PTR(x)  ((x)->curr_alloc)

/* gets the actual buffer contents which matches the current allocation size
 * (may be NULL for zero size dynamic buffer)
 */
#define DUK_HBUFFER_GET_DATA_PTR(x)  ( \
	DUK_HBUFFER_HAS_DYNAMIC((x)) ? \
		DUK_HBUFFER_DYNAMIC_GET_CURR_DATA_PTR((duk_hbuffer_dynamic *) (x)) : \
		DUK_HBUFFER_FIXED_GET_DATA_PTR((duk_hbuffer_fixed *) (x)) \
	)

/* gets the current user visible size, without accounting for a dynamic
 * buffer's "spare" (= usable size).
 */
#define DUK_HBUFFER_GET_SIZE(x)         ((x)->size)

#define DUK_HBUFFER_SET_SIZE(x,val)  do { \
		(x)->size = (val); \
	} while (0)

/* growth parameters */
#define DUK_HBUFFER_SPARE_ADD      16
#define DUK_HBUFFER_SPARE_DIVISOR  16   /* 2^4 -> 1/16 = 6.25% spare */

struct duk_hbuffer {
	duk_heaphdr hdr;

	/* it's not strictly necessary to track the current size, but
	 * it is useful for writing robust native code.
	 */

	duk_size_t size;  /* current size (not counting a dynamic buffer's "spare") */

	/*
	 *  Data following the header depends on the DUK_HBUFFER_FLAG_DYNAMIC
	 *  flag.
	 *
	 *  If the flag is clear (the buffer is a fixed size one), the buffer
	 *  data follows the header directly, consisting of 'size' bytes.
	 *
	 *  If the flag is set, the actual buffer is allocated separately, and
	 *  a few control fields follow the header.  Specifically:
	 *
	 *    - a "void *" pointing to the current allocation
	 *    - a duk_size_t indicating the full allocated size (always >= 'size')
	 *
	 *  Unlike strings, no terminator byte (NUL) is guaranteed after the
	 *  data.  This would be convenient, but would pad aligned user buffers
	 *  unnecessarily upwards in size.  For instance, if user code requested
	 *  a 64-byte dynamic buffer, 65 bytes would actually be allocated which
	 *  would then potentially round upwards to perhaps 68 or 72 bytes.
	 */
};

#if defined(DUK_USE_ALIGN_8) && defined(DUK_USE_PACK_MSVC_PRAGMA)
#pragma pack(push, 8)
#endif
struct duk_hbuffer_fixed {
	/* A union is used here as a portable struct size / alignment trick:
	 * by adding a 32-bit or a 64-bit (unused) union member, the size of
	 * the struct is effectively forced to be a multiple of 4 or 8 bytes
	 * (respectively) without increasing the size of the struct unless
	 * necessary.
	 */
	union {
		struct {
			duk_heaphdr hdr;
			duk_size_t size;
		} s;
#if defined(DUK_USE_ALIGN_4)
		duk_uint32_t dummy_for_align4;
#elif defined(DUK_USE_ALIGN_8)
		duk_uint64_t dummy_for_align8;
#else
		/* no extra padding */
#endif
	} u;

	/*
	 *  Data follows the struct header.  The struct size is padded by the
	 *  compiler based on the struct members.  This guarantees that the
	 *  buffer data will be aligned-by-4 but not necessarily aligned-by-8.
	 *
	 *  On platforms where alignment does not matter, the struct padding
	 *  could be removed (if there is any).  On platforms where alignment
	 *  by 8 is required, the struct size must be forced to be a multiple
	 *  of 8 by some means.  Without it, some user code may break, and also
	 *  Duktape itself breaks (e.g. the compiler stores duk_tvals in a
	 *  dynamic buffer).
	 */
}
#if defined(DUK_USE_ALIGN_8) && defined(DUK_USE_PACK_GCC_ATTR)
__attribute__ ((aligned (8)))
#elif defined(DUK_USE_ALIGN_8) && defined(DUK_USE_PACK_CLANG_ATTR)
__attribute__ ((aligned (8)))
#endif
;
#if defined(DUK_USE_ALIGN_8) && defined(DUK_USE_PACK_MSVC_PRAGMA)
#pragma pack(pop)
#endif

struct duk_hbuffer_dynamic {
	duk_heaphdr hdr;
	duk_size_t size;

	void *curr_alloc;  /* may be NULL if usable_size == 0 */
	duk_size_t usable_size;

	/*
	 *  Allocation size for 'curr_alloc' is usable_size directly.
	 *  There is no automatic NUL terminator for buffers (see above
	 *  for rationale).
	 *
	 *  'curr_alloc' is explicitly allocated with heap allocation
	 *  primitives and will thus always have alignment suitable for
	 *  e.g. duk_tval and an IEEE double.
	 */
};

/*
 *  Prototypes
 */

DUK_INTERNAL_DECL duk_hbuffer *duk_hbuffer_alloc(duk_heap *heap, duk_size_t size, duk_bool_t dynamic);
DUK_INTERNAL_DECL void *duk_hbuffer_get_dynalloc_ptr(void *ud);  /* indirect allocs */

/* dynamic buffer ops */
DUK_INTERNAL_DECL void duk_hbuffer_resize(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t new_size, duk_size_t new_usable_size);
DUK_INTERNAL_DECL void duk_hbuffer_reset(duk_hthread *thr, duk_hbuffer_dynamic *buf);
#if 0  /*unused*/
DUK_INTERNAL_DECL void duk_hbuffer_compact(duk_hthread *thr, duk_hbuffer_dynamic *buf);
#endif
DUK_INTERNAL_DECL void duk_hbuffer_append_bytes(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_uint8_t *data, duk_size_t length);
DUK_INTERNAL_DECL void duk_hbuffer_append_byte(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_uint8_t byte);
DUK_INTERNAL_DECL duk_size_t duk_hbuffer_append_cstring(duk_hthread *thr, duk_hbuffer_dynamic *buf, const char *str);
DUK_INTERNAL_DECL duk_size_t duk_hbuffer_append_hstring(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_hstring *str);
DUK_INTERNAL_DECL duk_size_t duk_hbuffer_append_xutf8(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_ucodepoint_t codepoint);
DUK_INTERNAL_DECL duk_size_t duk_hbuffer_append_cesu8(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_ucodepoint_t codepoint);
#if 0
DUK_INTERNAL_DECL void duk_hbuffer_append_native_u32(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_uint32_t val);
#endif
DUK_INTERNAL_DECL void duk_hbuffer_insert_bytes(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t offset, duk_uint8_t *data, duk_size_t length);
#if 0  /*unused*/
DUK_INTERNAL_DECL void duk_hbuffer_insert_byte(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t offset, duk_uint8_t byte);
#endif
#if 0  /*unused*/
DUK_INTERNAL_DECL duk_size_t duk_hbuffer_insert_cstring(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t offset, const char *str);
#endif
#if 0  /*unused*/
DUK_INTERNAL_DECL duk_size_t duk_hbuffer_insert_hstring(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t offset, duk_hstring *str);
#endif
DUK_INTERNAL_DECL duk_size_t duk_hbuffer_insert_xutf8(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t offset, duk_ucodepoint_t codepoint);
#if 0  /*unused*/
DUK_INTERNAL_DECL duk_size_t duk_hbuffer_insert_cesu8(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t offset, duk_ucodepoint_t codepoint);
#endif
DUK_INTERNAL_DECL void duk_hbuffer_remove_slice(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t offset, duk_size_t length);
DUK_INTERNAL_DECL void duk_hbuffer_insert_slice(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t dst_offset, duk_size_t src_offset, duk_size_t length);
DUK_INTERNAL_DECL void duk_hbuffer_append_slice(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t src_offset, duk_size_t length);

#endif  /* DUK_HBUFFER_H_INCLUDED */
