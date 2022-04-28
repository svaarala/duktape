/*
 *  Array object representation, used for actual Array and Arguments instances.
 */

#if !defined(DUK_HARRAY_H_INCLUDED)
#define DUK_HARRAY_H_INCLUDED

#if defined(DUK_USE_ASSERTIONS)
DUK_INTERNAL_DECL void duk_harray_assert_valid(duk_heap *heap, duk_harray *h);
#define DUK_HARRAY_ASSERT_VALID(heap, h) \
	do { \
		duk_harray_assert_valid((heap), (h)); \
	} while (0)
#else
#define DUK_HARRAY_ASSERT_VALID(heap, h) \
	do { \
	} while (0)
#endif

#define DUK_HARRAY_LENGTH_WRITABLE(h)    (!(h)->length_nonwritable)
#define DUK_HARRAY_LENGTH_NONWRITABLE(h) ((h)->length_nonwritable)
#define DUK_HARRAY_SET_LENGTH_WRITABLE(h) \
	do { \
		(h)->length_nonwritable = 0; \
	} while (0)
#define DUK_HARRAY_SET_LENGTH_NONWRITABLE(h) \
	do { \
		(h)->length_nonwritable = 1; \
	} while (0)

#if defined(DUK_USE_HEAPPTR16)
#define DUK_HARRAY_GET_ITEMS(heap, h) DUK_DEC_HEAPPTR((heap), (h)->items16, duk_tval *)
#define DUK_HARRAY_SET_ITEMS(heap, h, v) \
	do { \
		(h)->items16 = DUK_ENC_HEAPPTR((heap), (v)); \
	} while (0)
#else
#define DUK_HARRAY_GET_ITEMS(heap, h) (h)->items
#define DUK_HARRAY_SET_ITEMS(heap, h, v) \
	do { \
		(h)->items = (v); \
	} while (0)
#endif

#if defined(DUK_USE_OBJSIZES16)
#define DUK_HARRAY_GET_ITEMS_LENGTH(h) ((duk_uint32_t) (h)->items_length)
#define DUK_HARRAY_SET_ITEMS_LENGTH(h, v) \
	do { \
		(h)->items_length = (duk_uint16_t) (v); \
	} while (0)
#else
#define DUK_HARRAY_GET_ITEMS_LENGTH(h) ((h)->items_length)
#define DUK_HARRAY_SET_ITEMS_LENGTH(h, v) \
	do { \
		(h)->items_length = (v); \
	} while (0)
#endif

#define DUK_HARRAY_GET_LENGTH(h) ((h)->length)
#define DUK_HARRAY_SET_LENGTH(h, v) \
	do { \
		(h)->length = (v); \
	} while (0)

#define DUK_HARRAY_ITEMS_COVERED(h)  (DUK_HARRAY_GET_ITEMS_LENGTH((h)) >= DUK_HARRAY_GET_LENGTH((h)))
#define DUK_HARRAY_ALLOW_FASTPATH(h) (DUK_HOBJECT_HAS_ARRAY_ITEMS((duk_hobject *) (h)) && DUK_HARRAY_ITEMS_COVERED((h)))

struct duk_harray {
	/* Shared object part. */
	duk_hobject obj;

	/* Linear array part, present if array is not too sparse.  If the
	 * array would become too sparse, or any item uses non-standard
	 * property attributes (writable, enumerable, configurable), the
	 * array part is abandoned (DUK_HOBJECT_FLAG_ARRAY_ITEMS will be
	 * cleared to indicate 'items' won't be used anymore).
	 *
	 * Init policy: all items are initialized, to 'unused' for gaps.
	 */
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t items16;
#else
	duk_tval *items;
#endif
#if defined(DUK_USE_OBJSIZES16)
	duk_uint16_t items_length;
#else
	duk_uint32_t items_length;
#endif

	/* Array .length.  Unused for Arguments object.
	 *
	 * At present Array .length may be smaller, equal, or even larger
	 * than the allocated underlying array part.  Fast path code must
	 * always take this into account carefully.
	 */
	duk_uint32_t length;

	/* Array .length property attributes.  The property is always
	 * non-enumerable and non-configurable.  It's initially writable
	 * but per Object.defineProperty() rules it can be made non-writable
	 * even if it is non-configurable.  Thus we need to track the
	 * writability explicitly.
	 */
	duk_bool_t length_nonwritable;
};

DUK_INTERNAL_DECL duk_uint32_t duk_harray_get_active_items_length(duk_harray *a);
DUK_INTERNAL_DECL duk_tval *duk_harray_get_items(duk_heap *heap, duk_harray *a);

#endif /* DUK_HARRAY_H_INCLUDED */
