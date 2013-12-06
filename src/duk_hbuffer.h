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

#define DUK_HBUFFER_FLAG_DYNAMIC        DUK_HEAPHDR_USER_FLAG(0)  /* buffer is resizable */

#define DUK_HBUFFER_HAS_DYNAMIC(x)      DUK_HEAPHDR_CHECK_FLAG_BITS(&(x)->hdr, DUK_HBUFFER_FLAG_DYNAMIC)

#define DUK_HBUFFER_SET_DYNAMIC(x)      DUK_HEAPHDR_SET_FLAG_BITS(&(x)->hdr, DUK_HBUFFER_FLAG_DYNAMIC)

#define DUK_HBUFFER_CLEAR_DYNAMIC(x)    DUK_HEAPHDR_CLEAR_FLAG_BITS(&(x)->hdr, DUK_HBUFFER_FLAG_DYNAMIC)

#define DUK_HBUFFER_FIXED_GET_DATA_PTR(x)         ((duk_uint8_t *) (((duk_hbuffer_fixed *) (x)) + 1))

#define DUK_HBUFFER_DYNAMIC_GET_ALLOC_SIZE(x)     ((x)->usable_size + 1)
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

	size_t size;  /* current size (not counting a dynamic buffer's "spare") */

	/*
	 *  Data following the header depends on the DUK_HBUFFER_FLAG_DYNAMIC
	 *  flag.
	 *
	 *  If the flag is clear (the buffer is a fixed size one), the buffer
	 *  data follows the header directly, consisting of 'size' bytes,
	 *  followed by a zero byte for robustness.
	 *
	 *  If the flag is set, the actual buffer is allocated separately, and
	 *  a few control fields follow the header.  Specifically:
	 *
	 *    - a "void *" pointing to the current allocation
	 *    - a size_t indicating the full allocated size (always >= 'size')
	 */
};

struct duk_hbuffer_fixed {
	duk_heaphdr hdr;
	size_t size;

	/* Data follows the struct header.  The struct size is padded by the
	 * compiler so the buffer data area begins at an aligned address.
	 * This potentially a few bytes but would be difficult to implement
	 * portably (for instance, there is no portable way of figuring out
	 * the packed struct size, since packed attributes are non-portable).
	 */
};

struct duk_hbuffer_dynamic {
	duk_heaphdr hdr;
	size_t size;

	void *curr_alloc;  /* may be NULL if usable_size == 0 */
	size_t usable_size;

	/* alloc size is usable_size + 1; a zero byte always follows the buffer */
};

/*
 *  Prototypes
 */

duk_hbuffer *duk_hbuffer_alloc(duk_heap *heap, size_t size, int dynamic);
void *duk_hbuffer_get_dynalloc_ptr(void *ud);  /* indirect allocs */

/* dynamic buffer ops */
void duk_hbuffer_resize(duk_hthread *thr, duk_hbuffer_dynamic *buf, size_t new_size, size_t new_usable_size);
void duk_hbuffer_reset(duk_hthread *thr, duk_hbuffer_dynamic *buf);
void duk_hbuffer_compact(duk_hthread *thr, duk_hbuffer_dynamic *buf);
void duk_hbuffer_append_bytes(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_uint8_t *data, size_t length);
void duk_hbuffer_append_byte(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_uint8_t byte);
size_t duk_hbuffer_append_cstring(duk_hthread *thr, duk_hbuffer_dynamic *buf, const char *str);
size_t duk_hbuffer_append_hstring(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_hstring *str);
size_t duk_hbuffer_append_xutf8(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_uint32_t codepoint);
size_t duk_hbuffer_append_cesu8(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_uint32_t codepoint);
void duk_hbuffer_append_native_u32(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_uint32_t val);
void duk_hbuffer_insert_bytes(duk_hthread *thr, duk_hbuffer_dynamic *buf, size_t offset, duk_uint8_t *data, size_t length);
void duk_hbuffer_insert_byte(duk_hthread *thr, duk_hbuffer_dynamic *buf, size_t offset, duk_uint8_t byte);
size_t duk_hbuffer_insert_cstring(duk_hthread *thr, duk_hbuffer_dynamic *buf, size_t offset, const char *str);
size_t duk_hbuffer_insert_hstring(duk_hthread *thr, duk_hbuffer_dynamic *buf, size_t offset, duk_hstring *str);
size_t duk_hbuffer_insert_xutf8(duk_hthread *thr, duk_hbuffer_dynamic *buf, size_t offset, duk_uint32_t codepoint);
size_t duk_hbuffer_insert_cesu8(duk_hthread *thr, duk_hbuffer_dynamic *buf, size_t offset, duk_uint32_t codepoint);
void duk_hbuffer_remove_slice(duk_hthread *thr, duk_hbuffer_dynamic *buf, size_t offset, size_t length);
void duk_hbuffer_insert_slice(duk_hthread *thr, duk_hbuffer_dynamic *buf, size_t dst_offset, size_t src_offset, size_t length);
void duk_hbuffer_append_slice(duk_hthread *thr, duk_hbuffer_dynamic *buf, size_t src_offset, size_t length);

#endif  /* DUK_HBUFFER_H_INCLUDED */

