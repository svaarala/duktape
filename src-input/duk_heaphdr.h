/*
 *  Heap header definition and assorted macros, including ref counting.
 *  Access all fields through the accessor macros.
 */

#if !defined(DUK_HEAPHDR_H_INCLUDED)
#define DUK_HEAPHDR_H_INCLUDED

/*
 *  Common heap header
 *
 *  All heap objects share the same flags and refcount fields.  Objects other
 *  than strings also need to have a single or double linked list pointers
 *  for insertion into the "heap allocated" list.  Strings have single linked
 *  list pointers for string table chaining.
 *
 *  Technically, 'h_refcount' must be wide enough to guarantee that it cannot
 *  wrap (otherwise objects might be freed incorrectly after wrapping).  This
 *  means essentially that the refcount field must be as wide as data pointers.
 *  On 64-bit platforms this means that the refcount needs to be 64 bits even
 *  if an 'int' is 32 bits.  This is a bit unfortunate, and compromising on
 *  this might be reasonable in the future.
 *
 *  Heap header size on 32-bit platforms: 8 bytes without reference counting,
 *  16 bytes with reference counting.
 */

/* XXX: macro for shared header fields (avoids some padding issues) */

struct duk_heaphdr {
	duk_uint32_t h_flags;

#if defined(DUK_USE_REFERENCE_COUNTING)
#if defined(DUK_USE_REFCOUNT16)
	duk_uint16_t h_refcount16;
#else
	duk_size_t h_refcount;
#endif
#endif

#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t h_next16;
#else
	duk_heaphdr *h_next;
#endif

#if defined(DUK_USE_DOUBLE_LINKED_HEAP)
	/* refcounting requires direct heap frees, which in turn requires a dual linked heap */
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t h_prev16;
#else
	duk_heaphdr *h_prev;
#endif
#endif

	/* When DUK_USE_HEAPPTR16 (and DUK_USE_REFCOUNT16) is in use, the
	 * struct won't align nicely to 4 bytes.  This 16-bit extra field
	 * is added to make the alignment clean; the field can be used by
	 * heap objects when 16-bit packing is used.  This field is now
	 * conditional to DUK_USE_HEAPPTR16 only, but it is intended to be
	 * used with DUK_USE_REFCOUNT16 and DUK_USE_DOUBLE_LINKED_HEAP;
	 * this only matter to low memory environments anyway.
	 */
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t h_extra16;
#endif
};

struct duk_heaphdr_string {
	/* 16 bits would be enough for shared heaphdr flags and duk_hstring
	 * flags.  The initial parts of duk_heaphdr_string and duk_heaphdr
	 * must match so changing the flags field size here would be quite
	 * awkward.  However, to minimize struct size, we can pack at least
	 * 16 bits of duk_hstring data into the flags field.
	 */
	duk_uint32_t h_flags;

#if defined(DUK_USE_REFERENCE_COUNTING)
#if defined(DUK_USE_REFCOUNT16)
	duk_uint16_t h_refcount16;
	duk_uint16_t h_strextra16;  /* round out to 8 bytes */
#else
	duk_size_t h_refcount;
#endif
#else
	duk_uint16_t h_strextra16;
#endif

	duk_hstring *h_next;
	/* No 'h_prev' pointer for strings. */
};

#define DUK_HEAPHDR_FLAGS_TYPE_MASK      0x00000003UL
#define DUK_HEAPHDR_FLAGS_FLAG_MASK      (~DUK_HEAPHDR_FLAGS_TYPE_MASK)

                                             /* 2 bits for heap type */
#define DUK_HEAPHDR_FLAGS_HEAP_START     2   /* 5 heap flags */
#define DUK_HEAPHDR_FLAGS_USER_START     7   /* 25 user flags */

#define DUK_HEAPHDR_HEAP_FLAG_NUMBER(n)  (DUK_HEAPHDR_FLAGS_HEAP_START + (n))
#define DUK_HEAPHDR_USER_FLAG_NUMBER(n)  (DUK_HEAPHDR_FLAGS_USER_START + (n))
#define DUK_HEAPHDR_HEAP_FLAG(n)         (1UL << (DUK_HEAPHDR_FLAGS_HEAP_START + (n)))
#define DUK_HEAPHDR_USER_FLAG(n)         (1UL << (DUK_HEAPHDR_FLAGS_USER_START + (n)))

#define DUK_HEAPHDR_FLAG_REACHABLE       DUK_HEAPHDR_HEAP_FLAG(0)  /* mark-and-sweep: reachable */
#define DUK_HEAPHDR_FLAG_TEMPROOT        DUK_HEAPHDR_HEAP_FLAG(1)  /* mark-and-sweep: children not processed */
#define DUK_HEAPHDR_FLAG_FINALIZABLE     DUK_HEAPHDR_HEAP_FLAG(2)  /* mark-and-sweep: finalizable (on current pass) */
#define DUK_HEAPHDR_FLAG_FINALIZED       DUK_HEAPHDR_HEAP_FLAG(3)  /* mark-and-sweep: finalized (on previous pass) */
#define DUK_HEAPHDR_FLAG_READONLY        DUK_HEAPHDR_HEAP_FLAG(4)  /* read-only object, in code section */

#define DUK_HTYPE_MIN                    0
#define DUK_HTYPE_STRING                 0
#define DUK_HTYPE_OBJECT                 1
#define DUK_HTYPE_BUFFER                 2
#define DUK_HTYPE_MAX                    2

#if defined(DUK_USE_HEAPPTR16)
#define DUK_HEAPHDR_GET_NEXT(heap,h) \
	((duk_heaphdr *) DUK_USE_HEAPPTR_DEC16((heap)->heap_udata, (h)->h_next16))
#define DUK_HEAPHDR_SET_NEXT(heap,h,val)   do { \
		(h)->h_next16 = DUK_USE_HEAPPTR_ENC16((heap)->heap_udata, (void *) val); \
	} while (0)
#else
#define DUK_HEAPHDR_GET_NEXT(heap,h)  ((h)->h_next)
#define DUK_HEAPHDR_SET_NEXT(heap,h,val)   do { \
		(h)->h_next = (val); \
	} while (0)
#endif

#if defined(DUK_USE_DOUBLE_LINKED_HEAP)
#if defined(DUK_USE_HEAPPTR16)
#define DUK_HEAPHDR_GET_PREV(heap,h) \
	((duk_heaphdr *) DUK_USE_HEAPPTR_DEC16((heap)->heap_udata, (h)->h_prev16))
#define DUK_HEAPHDR_SET_PREV(heap,h,val)   do { \
		(h)->h_prev16 = DUK_USE_HEAPPTR_ENC16((heap)->heap_udata, (void *) (val)); \
	} while (0)
#else
#define DUK_HEAPHDR_GET_PREV(heap,h)       ((h)->h_prev)
#define DUK_HEAPHDR_SET_PREV(heap,h,val)   do { \
		(h)->h_prev = (val); \
	} while (0)
#endif
#endif

#if defined(DUK_USE_REFERENCE_COUNTING)
#if defined(DUK_USE_REFCOUNT16)
#define DUK_HEAPHDR_GET_REFCOUNT(h)   ((h)->h_refcount16)
#define DUK_HEAPHDR_SET_REFCOUNT(h,val)  do { \
		(h)->h_refcount16 = (val); \
	} while (0)
#define DUK_HEAPHDR_PREINC_REFCOUNT(h)  (++(h)->h_refcount16)  /* result: updated refcount */
#define DUK_HEAPHDR_PREDEC_REFCOUNT(h)  (--(h)->h_refcount16)  /* result: updated refcount */
#else
#define DUK_HEAPHDR_GET_REFCOUNT(h)   ((h)->h_refcount)
#define DUK_HEAPHDR_SET_REFCOUNT(h,val)  do { \
		(h)->h_refcount = (val); \
	} while (0)
#define DUK_HEAPHDR_PREINC_REFCOUNT(h)  (++(h)->h_refcount)  /* result: updated refcount */
#define DUK_HEAPHDR_PREDEC_REFCOUNT(h)  (--(h)->h_refcount)  /* result: updated refcount */
#endif
#else
/* refcount macros not defined without refcounting, caller must #if defined() now */
#endif  /* DUK_USE_REFERENCE_COUNTING */

/*
 *  Note: type is treated as a field separate from flags, so some masking is
 *  involved in the macros below.
 */

#define DUK_HEAPHDR_GET_FLAGS_RAW(h)  ((h)->h_flags)
#define DUK_HEAPHDR_SET_FLAGS_RAW(h,val)  do { \
		(h)->h_flags = (val); } \
	}
#define DUK_HEAPHDR_GET_FLAGS(h)      ((h)->h_flags & DUK_HEAPHDR_FLAGS_FLAG_MASK)
#define DUK_HEAPHDR_SET_FLAGS(h,val)  do { \
		(h)->h_flags = ((h)->h_flags & ~(DUK_HEAPHDR_FLAGS_FLAG_MASK)) | (val); \
	} while (0)
#define DUK_HEAPHDR_GET_TYPE(h)       ((h)->h_flags & DUK_HEAPHDR_FLAGS_TYPE_MASK)
#define DUK_HEAPHDR_SET_TYPE(h,val)   do { \
		(h)->h_flags = ((h)->h_flags & ~(DUK_HEAPHDR_FLAGS_TYPE_MASK)) | (val); \
	} while (0)

/* Comparison for type >= DUK_HTYPE_MIN skipped; because DUK_HTYPE_MIN is zero
 * and the comparison is unsigned, it's always true and generates warnings.
 */
#define DUK_HEAPHDR_HTYPE_VALID(h)    ( \
	DUK_HEAPHDR_GET_TYPE((h)) <= DUK_HTYPE_MAX \
	)

#define DUK_HEAPHDR_SET_TYPE_AND_FLAGS(h,tval,fval)  do { \
		(h)->h_flags = ((tval) & DUK_HEAPHDR_FLAGS_TYPE_MASK) | \
		               ((fval) & DUK_HEAPHDR_FLAGS_FLAG_MASK); \
	} while (0)

#define DUK_HEAPHDR_SET_FLAG_BITS(h,bits)  do { \
		DUK_ASSERT(((bits) & ~(DUK_HEAPHDR_FLAGS_FLAG_MASK)) == 0); \
		(h)->h_flags |= (bits); \
	} while (0)

#define DUK_HEAPHDR_CLEAR_FLAG_BITS(h,bits)  do { \
		DUK_ASSERT(((bits) & ~(DUK_HEAPHDR_FLAGS_FLAG_MASK)) == 0); \
		(h)->h_flags &= ~((bits)); \
	} while (0)

#define DUK_HEAPHDR_CHECK_FLAG_BITS(h,bits)  (((h)->h_flags & (bits)) != 0)

#define DUK_HEAPHDR_SET_REACHABLE(h)      DUK_HEAPHDR_SET_FLAG_BITS((h),DUK_HEAPHDR_FLAG_REACHABLE)
#define DUK_HEAPHDR_CLEAR_REACHABLE(h)    DUK_HEAPHDR_CLEAR_FLAG_BITS((h),DUK_HEAPHDR_FLAG_REACHABLE)
#define DUK_HEAPHDR_HAS_REACHABLE(h)      DUK_HEAPHDR_CHECK_FLAG_BITS((h),DUK_HEAPHDR_FLAG_REACHABLE)

#define DUK_HEAPHDR_SET_TEMPROOT(h)       DUK_HEAPHDR_SET_FLAG_BITS((h),DUK_HEAPHDR_FLAG_TEMPROOT)
#define DUK_HEAPHDR_CLEAR_TEMPROOT(h)     DUK_HEAPHDR_CLEAR_FLAG_BITS((h),DUK_HEAPHDR_FLAG_TEMPROOT)
#define DUK_HEAPHDR_HAS_TEMPROOT(h)       DUK_HEAPHDR_CHECK_FLAG_BITS((h),DUK_HEAPHDR_FLAG_TEMPROOT)

#define DUK_HEAPHDR_SET_FINALIZABLE(h)    DUK_HEAPHDR_SET_FLAG_BITS((h),DUK_HEAPHDR_FLAG_FINALIZABLE)
#define DUK_HEAPHDR_CLEAR_FINALIZABLE(h)  DUK_HEAPHDR_CLEAR_FLAG_BITS((h),DUK_HEAPHDR_FLAG_FINALIZABLE)
#define DUK_HEAPHDR_HAS_FINALIZABLE(h)    DUK_HEAPHDR_CHECK_FLAG_BITS((h),DUK_HEAPHDR_FLAG_FINALIZABLE)

#define DUK_HEAPHDR_SET_FINALIZED(h)      DUK_HEAPHDR_SET_FLAG_BITS((h),DUK_HEAPHDR_FLAG_FINALIZED)
#define DUK_HEAPHDR_CLEAR_FINALIZED(h)    DUK_HEAPHDR_CLEAR_FLAG_BITS((h),DUK_HEAPHDR_FLAG_FINALIZED)
#define DUK_HEAPHDR_HAS_FINALIZED(h)      DUK_HEAPHDR_CHECK_FLAG_BITS((h),DUK_HEAPHDR_FLAG_FINALIZED)

#define DUK_HEAPHDR_SET_READONLY(h)       DUK_HEAPHDR_SET_FLAG_BITS((h),DUK_HEAPHDR_FLAG_READONLY)
#define DUK_HEAPHDR_CLEAR_READONLY(h)     DUK_HEAPHDR_CLEAR_FLAG_BITS((h),DUK_HEAPHDR_FLAG_READONLY)
#define DUK_HEAPHDR_HAS_READONLY(h)       DUK_HEAPHDR_CHECK_FLAG_BITS((h),DUK_HEAPHDR_FLAG_READONLY)

/* get or set a range of flags; m=first bit number, n=number of bits */
#define DUK_HEAPHDR_GET_FLAG_RANGE(h,m,n)  (((h)->h_flags >> (m)) & ((1UL << (n)) - 1UL))

#define DUK_HEAPHDR_SET_FLAG_RANGE(h,m,n,v)  do { \
		(h)->h_flags = \
			((h)->h_flags & (~(((1UL << (n)) - 1UL) << (m)))) \
			| ((v) << (m)); \
	} while (0)

/* init pointer fields to null */
#if defined(DUK_USE_DOUBLE_LINKED_HEAP)
#define DUK_HEAPHDR_INIT_NULLS(h)       do { \
		DUK_HEAPHDR_SET_NEXT((h), (void *) NULL); \
		DUK_HEAPHDR_SET_PREV((h), (void *) NULL); \
	} while (0)
#else
#define DUK_HEAPHDR_INIT_NULLS(h)       do { \
		DUK_HEAPHDR_SET_NEXT((h), (void *) NULL); \
	} while (0)
#endif

#define DUK_HEAPHDR_STRING_INIT_NULLS(h)  do { \
		(h)->h_next = NULL; \
	} while (0)

/*
 *  Type tests
 */

#define DUK_HEAPHDR_IS_OBJECT(h) \
	(DUK_HEAPHDR_GET_TYPE((h)) == DUK_HTYPE_OBJECT)
#define DUK_HEAPHDR_IS_STRING(h) \
	(DUK_HEAPHDR_GET_TYPE((h)) == DUK_HTYPE_STRING)
#define DUK_HEAPHDR_IS_BUFFER(h) \
	(DUK_HEAPHDR_GET_TYPE((h)) == DUK_HTYPE_BUFFER)

/*
 *  Assert helpers
 */

/* Check that prev/next links are consistent: if e.g. h->prev is != NULL,
 * h->prev->next should point back to h.
 */
#if defined(DUK_USE_DOUBLE_LINKED_HEAP) && defined(DUK_USE_ASSERTIONS)
#define DUK_ASSERT_HEAPHDR_LINKS(heap,h) do { \
		if ((h) != NULL) { \
			duk_heaphdr *h__prev, *h__next; \
			h__prev = DUK_HEAPHDR_GET_PREV((heap), (h)); \
			h__next = DUK_HEAPHDR_GET_NEXT((heap), (h)); \
			DUK_ASSERT(h__prev == NULL || (DUK_HEAPHDR_GET_NEXT((heap), h__prev) == (h))); \
			DUK_ASSERT(h__next == NULL || (DUK_HEAPHDR_GET_PREV((heap), h__next) == (h))); \
		} \
	} while (0)
#else
#define DUK_ASSERT_HEAPHDR_LINKS(heap,h) do {} while (0)
#endif

/*
 *  Reference counting helper macros.  The macros take a thread argument
 *  and must thus always be executed in a specific thread context.  The
 *  thread argument is needed for features like finalization.  Currently
 *  it is not required for INCREF, but it is included just in case.
 *
 *  Note that 'raw' macros such as DUK_HEAPHDR_GET_REFCOUNT() are not
 *  defined without DUK_USE_REFERENCE_COUNTING, so caller must #if defined()
 *  around them.
 *
 *  NORZ variants perform refcount operations and may queue an object to be
 *  refcount finalized (heap->refzero_list), but won't actually process the
 *  queue.  Calling code must always call DUK_REFZERO_CHECK_xxx() after a
 *  sequence of NORZ macro calls, otherwise the queue may be left unprocessed
 *  until the next such call.  If an error thrown, DUK_REFZERO_CHECK_xxx()
 *  is always called so that NORZ/DUK_REFZERO_CHECK_xxx() can be used without
 *  a catch point.
 */

#if defined(DUK_USE_REFERENCE_COUNTING)

#if defined(DUK_USE_ROM_OBJECTS)
/* With ROM objects "needs refcount update" is true when the value is
 * heap allocated and is not a ROM object.
 */
/* XXX: double evaluation for 'tv' argument. */
#define DUK_TVAL_NEEDS_REFCOUNT_UPDATE(tv) \
	(DUK_TVAL_IS_HEAP_ALLOCATED((tv)) && !DUK_HEAPHDR_HAS_READONLY(DUK_TVAL_GET_HEAPHDR((tv))))
#define DUK_HEAPHDR_NEEDS_REFCOUNT_UPDATE(h)  (!DUK_HEAPHDR_HAS_READONLY((h)))
#else  /* DUK_USE_ROM_OBJECTS */
/* Without ROM objects "needs refcount update" == is heap allocated. */
#define DUK_TVAL_NEEDS_REFCOUNT_UPDATE(tv)    DUK_TVAL_IS_HEAP_ALLOCATED((tv))
#define DUK_HEAPHDR_NEEDS_REFCOUNT_UPDATE(h)  1
#endif  /* DUK_USE_ROM_OBJECTS */

/* Fast variants, inline refcount operations except for refzero handling.
 * Can be used explicitly when speed is always more important than size.
 * For a good compiler and a single file build, these are basically the
 * same as a forced inline.
 */
#define DUK_TVAL_INCREF_FAST(thr,tv) do { \
		duk_tval *duk__tv = (tv); \
		DUK_ASSERT(duk__tv != NULL); \
		if (DUK_TVAL_NEEDS_REFCOUNT_UPDATE(duk__tv)) { \
			duk_heaphdr *duk__h = DUK_TVAL_GET_HEAPHDR(duk__tv); \
			DUK_ASSERT(duk__h != NULL); \
			DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(duk__h)); \
			DUK_HEAPHDR_PREINC_REFCOUNT(duk__h); \
		} \
	} while (0)
#define DUK_TVAL_DECREF_FAST(thr,tv) do { \
		duk_tval *duk__tv = (tv); \
		DUK_ASSERT(duk__tv != NULL); \
		if (DUK_TVAL_NEEDS_REFCOUNT_UPDATE(duk__tv)) { \
			duk_heaphdr *duk__h = DUK_TVAL_GET_HEAPHDR(duk__tv); \
			DUK_ASSERT(duk__h != NULL); \
			DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(duk__h)); \
			DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT(duk__h) > 0); \
			if (DUK_HEAPHDR_PREDEC_REFCOUNT(duk__h) == 0) { \
				duk_heaphdr_refzero((thr), duk__h); \
			} \
		} \
	} while (0)
#define DUK_TVAL_DECREF_NORZ_FAST(thr,tv) do { \
		duk_tval *duk__tv = (tv); \
		DUK_ASSERT(duk__tv != NULL); \
		if (DUK_TVAL_NEEDS_REFCOUNT_UPDATE(duk__tv)) { \
			duk_heaphdr *duk__h = DUK_TVAL_GET_HEAPHDR(duk__tv); \
			DUK_ASSERT(duk__h != NULL); \
			DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(duk__h)); \
			DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT(duk__h) > 0); \
			if (DUK_HEAPHDR_PREDEC_REFCOUNT(duk__h) == 0) { \
				duk_heaphdr_refzero_norz((thr), duk__h); \
			} \
		} \
	} while (0)
#define DUK_HEAPHDR_INCREF_FAST(thr,h) do { \
		duk_heaphdr *duk__h = (duk_heaphdr *) (h); \
		DUK_ASSERT(duk__h != NULL); \
		DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(duk__h)); \
		if (DUK_HEAPHDR_NEEDS_REFCOUNT_UPDATE(duk__h)) { \
			DUK_HEAPHDR_PREINC_REFCOUNT(duk__h); \
		} \
	} while (0)
#define DUK_HEAPHDR_DECREF_FAST_RAW(thr,h,rzcall,rzcast) do { \
		duk_heaphdr *duk__h = (duk_heaphdr *) (h); \
		DUK_ASSERT(duk__h != NULL); \
		DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(duk__h)); \
		DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT(duk__h) > 0); \
		if (DUK_HEAPHDR_NEEDS_REFCOUNT_UPDATE(duk__h)) { \
			if (DUK_HEAPHDR_PREDEC_REFCOUNT(duk__h) == 0) { \
				(rzcall)((thr), (rzcast) duk__h); \
			} \
		} \
	} while (0)
#define DUK_HEAPHDR_DECREF_FAST(thr,h) \
	DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_heaphdr_refzero,duk_heaphdr *)
#define DUK_HEAPHDR_DECREF_NORZ_FAST(thr,h) \
	DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_heaphdr_refzero_norz,duk_heaphdr *)

/* Slow variants, call to a helper to reduce code size.
 * Can be used explicitly when size is always more important than speed.
 */
#define DUK_TVAL_INCREF_SLOW(thr,tv)         do { duk_tval_incref((tv)); } while (0)
#define DUK_TVAL_DECREF_SLOW(thr,tv)         do { duk_tval_decref((thr), (tv)); } while (0)
#define DUK_TVAL_DECREF_NORZ_SLOW(thr,tv)    do { duk_tval_decref_norz((thr), (tv)); } while (0)
#define DUK_HEAPHDR_INCREF_SLOW(thr,h)       do { duk_heaphdr_incref((duk_heaphdr *) (h)); } while (0)
#define DUK_HEAPHDR_DECREF_SLOW(thr,h)       do { duk_heaphdr_decref((thr), (duk_heaphdr *) (h)); } while (0)
#define DUK_HEAPHDR_DECREF_NORZ_SLOW(thr,h)  do { duk_heaphdr_decref_norz((thr), (duk_heaphdr *) (h)); } while (0)
#define DUK_HSTRING_INCREF_SLOW(thr,h)       do { duk_heaphdr_incref((duk_heaphdr *) (h)); } while (0)
#define DUK_HSTRING_DECREF_SLOW(thr,h)       do { duk_heaphdr_decref((thr), (duk_heaphdr *) (h)); } while (0)
#define DUK_HSTRING_DECREF_NORZ_SLOW(thr,h)  do { duk_heaphdr_decref_norz((thr), (duk_heaphdr *) (h)); } while (0)
#define DUK_HBUFFER_INCREF_SLOW(thr,h)       do { duk_heaphdr_incref((duk_heaphdr *) (h)); } while (0)
#define DUK_HBUFFER_DECREF_SLOW(thr,h)       do { duk_heaphdr_decref((thr), (duk_heaphdr *) (h)); } while (0)
#define DUK_HBUFFER_DECREF_NORZ_SLOW(thr,h)  do { duk_heaphdr_decref_norz((thr), (duk_heaphdr *) (h)); } while (0)
#define DUK_HOBJECT_INCREF_SLOW(thr,h)       do { duk_heaphdr_incref((duk_heaphdr *) (h)); } while (0)
#define DUK_HOBJECT_DECREF_SLOW(thr,h)       do { duk_heaphdr_decref((thr), (duk_heaphdr *) (h)); } while (0)
#define DUK_HOBJECT_DECREF_NORZ_SLOW(thr,h)  do { duk_heaphdr_decref_norz((thr), (duk_heaphdr *) (h)); } while (0)

/* Default variants.  Selection depends on speed/size preference.
 * Concretely: with gcc 4.8.1 -Os x64 the difference in final binary
 * is about +1kB for _FAST variants.
 */
#if defined(DUK_USE_FAST_REFCOUNT_DEFAULT)
/* XXX: It would be nice to specialize for specific duk_hobject subtypes
 * but current refzero queue handling prevents that.
 */
#define DUK_TVAL_INCREF(thr,tv)                DUK_TVAL_INCREF_FAST((thr),(tv))
#define DUK_TVAL_DECREF(thr,tv)                DUK_TVAL_DECREF_FAST((thr),(tv))
#define DUK_TVAL_DECREF_NORZ(thr,tv)           DUK_TVAL_DECREF_NORZ_FAST((thr),(tv))
#define DUK_HEAPHDR_INCREF(thr,h)              DUK_HEAPHDR_INCREF_FAST((thr),(h))
#define DUK_HEAPHDR_DECREF(thr,h)              DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_heaphdr_refzero,duk_heaphdr *)
#define DUK_HEAPHDR_DECREF_NORZ(thr,h)         DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_heaphdr_refzero_norz,duk_heaphdr *)
#define DUK_HSTRING_INCREF(thr,h)              DUK_HEAPHDR_INCREF((thr),(duk_heaphdr *) (h))
#define DUK_HSTRING_DECREF(thr,h)              DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_hstring_refzero,duk_hstring *)
#define DUK_HSTRING_DECREF_NORZ(thr,h)         DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_hstring_refzero,duk_hstring *)  /* no 'norz' variant */
#define DUK_HOBJECT_INCREF(thr,h)              DUK_HEAPHDR_INCREF((thr),(duk_heaphdr *) (h))
#define DUK_HOBJECT_DECREF(thr,h)              DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_hobject_refzero,duk_hobject *)
#define DUK_HOBJECT_DECREF_NORZ(thr,h)         DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_hobject_refzero_norz,duk_hobject *)
#define DUK_HBUFFER_INCREF(thr,h)              DUK_HEAPHDR_INCREF((thr),(duk_heaphdr *) (h))
#define DUK_HBUFFER_DECREF(thr,h)              DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_hbuffer_refzero,duk_hbuffer *)
#define DUK_HBUFFER_DECREF_NORZ(thr,h)         DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_hbuffer_refzero,duk_hbuffer *)  /* no 'norz' variant */
#define DUK_HCOMPFUNC_INCREF(thr,h)            DUK_HEAPHDR_INCREF((thr),(duk_heaphdr *) &(h)->obj)
#define DUK_HCOMPFUNC_DECREF(thr,h)            DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_hobject_refzero,duk_hobject *)
#define DUK_HCOMPFUNC_DECREF_NORZ(thr,h)       DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_hobject_refzero_norz,duk_hobject *)
#define DUK_HNATFUNC_INCREF(thr,h)             DUK_HEAPHDR_INCREF((thr),(duk_heaphdr *) &(h)->obj)
#define DUK_HNATFUNC_DECREF(thr,h)             DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_hobject_refzero,duk_hobject *)
#define DUK_HNATFUNC_DECREF_NORZ(thr,h)        DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_hobject_refzero_norz,duk_hobject *)
#define DUK_HBUFOBJ_INCREF(thr,h)              DUK_HEAPHDR_INCREF((thr),(duk_heaphdr *) &(h)->obj)
#define DUK_HBUFOBJ_DECREF(thr,h)              DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_hobject_refzero,duk_hobject *)
#define DUK_HBUFOBJ_DECREF_NORZ(thr,h)         DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_hobject_refzero_norz,duk_hobject *)
#define DUK_HTHREAD_INCREF(thr,h)              DUK_HEAPHDR_INCREF((thr),(duk_heaphdr *) &(h)->obj)
#define DUK_HTHREAD_DECREF(thr,h)              DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_hobject_refzero,duk_hobject *)
#define DUK_HTHREAD_DECREF_NORZ(thr,h)         DUK_HEAPHDR_DECREF_FAST_RAW((thr),(h),duk_hobject_refzero_norz,duk_hobject *)
#else
#define DUK_TVAL_INCREF(thr,tv)                DUK_TVAL_INCREF_SLOW((thr),(tv))
#define DUK_TVAL_DECREF(thr,tv)                DUK_TVAL_DECREF_SLOW((thr),(tv))
#define DUK_TVAL_DECREF_NORZ(thr,tv)           DUK_TVAL_DECREF_NORZ_SLOW((thr),(tv))
#define DUK_HEAPHDR_INCREF(thr,h)              DUK_HEAPHDR_INCREF_SLOW((thr),(h))
#define DUK_HEAPHDR_DECREF(thr,h)              DUK_HEAPHDR_DECREF_SLOW((thr),(h))
#define DUK_HEAPHDR_DECREF_NORZ(thr,h)         DUK_HEAPHDR_DECREF_NORZ_SLOW((thr),(h))
#define DUK_HSTRING_INCREF(thr,h)              DUK_HEAPHDR_INCREF((thr),(duk_heaphdr *) (h))
#define DUK_HSTRING_DECREF(thr,h)              DUK_HSTRING_DECREF_SLOW((thr),(h))
#define DUK_HSTRING_DECREF_NORZ(thr,h)         DUK_HSTRING_DECREF_NORZ_SLOW((thr),(h))
#define DUK_HOBJECT_INCREF(thr,h)              DUK_HEAPHDR_INCREF((thr),(duk_heaphdr *) (h))
#define DUK_HOBJECT_DECREF(thr,h)              DUK_HOBJECT_DECREF_SLOW((thr),(h))
#define DUK_HOBJECT_DECREF_NORZ(thr,h)         DUK_HOBJECT_DECREF_NORZ_SLOW((thr),(h))
#define DUK_HBUFFER_INCREF(thr,h)              DUK_HEAPHDR_INCREF((thr),(duk_heaphdr *) (h))
#define DUK_HBUFFER_DECREF(thr,h)              DUK_HBUFFER_DECREF_SLOW((thr),(h))
#define DUK_HBUFFER_DECREF_NORZ(thr,h)         DUK_HBUFFER_DECREF_NORZ_SLOW((thr),(h))
#define DUK_HCOMPFUNC_INCREF(thr,h)            DUK_HEAPHDR_INCREF((thr),(duk_heaphdr *) &(h)->obj)
#define DUK_HCOMPFUNC_DECREF(thr,h)            DUK_HOBJECT_DECREF_SLOW((thr),(duk_hobject *) &(h)->obj)
#define DUK_HCOMPFUNC_DECREF_NORZ(thr,h)       DUK_HOBJECT_DECREF_NORZ_SLOW((thr),(duk_hobject *) &(h)->obj)
#define DUK_HNATFUNC_INCREF(thr,h)             DUK_HEAPHDR_INCREF((thr),(duk_heaphdr *) &(h)->obj)
#define DUK_HNATFUNC_DECREF(thr,h)             DUK_HOBJECT_DECREF_SLOW((thr),(duk_hobject *) &(h)->obj)
#define DUK_HNATFUNC_DECREF_NORZ(thr,h)        DUK_HOBJECT_DECREF_NORZ_SLOW((thr),(duk_hobject *) &(h)->obj)
#define DUK_HBUFOBJ_INCREF(thr,h)              DUK_HEAPHDR_INCREF((thr),(duk_heaphdr *) &(h)->obj)
#define DUK_HBUFOBJ_DECREF(thr,h)              DUK_HOBJECT_DECREF_SLOW((thr),(duk_hobject *) &(h)->obj)
#define DUK_HBUFOB_DECREF_NORZ(thr,h)          DUK_HOBJECT_DECREF_NORZ_SLOW((thr),(duk_hobject *) &(h)->obj)
#define DUK_HTHREAD_INCREF(thr,h)              DUK_HEAPHDR_INCREF((thr),(duk_heaphdr *) &(h)->obj)
#define DUK_HTHREAD_DECREF(thr,h)              DUK_HOBJECT_DECREF_SLOW((thr),(duk_hobject *) &(h)->obj)
#define DUK_HTHREAD_DECREF_NORZ(thr,h)         DUK_HOBJECT_DECREF_NORZ_SLOW((thr),(duk_hobject *) &(h)->obj)
#endif

/* Convenience for some situations; the above macros don't allow NULLs
 * for performance reasons.  Macros cover only actually needed cases.
 */
#define DUK_HEAPHDR_INCREF_ALLOWNULL(thr,h) do { \
		if ((h) != NULL) { \
			DUK_HEAPHDR_INCREF((thr), (duk_heaphdr *) (h)); \
		} \
	} while (0)
#define DUK_HEAPHDR_DECREF_ALLOWNULL(thr,h) do { \
		if ((h) != NULL) { \
			DUK_HEAPHDR_DECREF((thr), (duk_heaphdr *) (h)); \
		} \
	} while (0)
#define DUK_HEAPHDR_DECREF_NORZ_ALLOWNULL(thr,h) do { \
		if ((h) != NULL) { \
			DUK_HEAPHDR_DECREF_NORZ((thr), (duk_heaphdr *) (h)); \
		} \
	} while (0)
#define DUK_HOBJECT_INCREF_ALLOWNULL(thr,h) do { \
		if ((h) != NULL) { \
			DUK_HOBJECT_INCREF((thr), (h)); \
		} \
	} while (0)
#define DUK_HOBJECT_DECREF_ALLOWNULL(thr,h) do { \
		if ((h) != NULL) { \
			DUK_HOBJECT_DECREF((thr), (h)); \
		} \
	} while (0)
#define DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr,h) do { \
		if ((h) != NULL) { \
			DUK_HOBJECT_DECREF_NORZ((thr), (h)); \
		} \
	} while (0)
#define DUK_HBUFFER_INCREF_ALLOWNULL(thr,h) do { \
		if ((h) != NULL) { \
			DUK_HBUFFER_INCREF((thr), (h)); \
		} \
	} while (0)
#define DUK_HBUFFER_DECREF_ALLOWNULL(thr,h) do { \
		if ((h) != NULL) { \
			DUK_HBUFFER_DECREF((thr), (h)); \
		} \
	} while (0)
#define DUK_HBUFFER_DECREF_NORZ_ALLOWNULL(thr,h) do { \
		if ((h) != NULL) { \
			DUK_HBUFFER_DECREF_NORZ((thr), (h)); \
		} \
	} while (0)
#define DUK_HTHREAD_INCREF_ALLOWNULL(thr,h) do { \
		if ((h) != NULL) { \
			DUK_HTHREAD_INCREF((thr), (h)); \
		} \
	} while (0)
#define DUK_HTHREAD_DECREF_ALLOWNULL(thr,h) do { \
		if ((h) != NULL) { \
			DUK_HTHREAD_DECREF((thr), (h)); \
		} \
	} while (0)
#define DUK_HTHREAD_DECREF_NORZ_ALLOWNULL(thr,h) do { \
		if ((h) != NULL) { \
			DUK_HTHREAD_DECREF_NORZ((thr), (h)); \
		} \
	} while (0)

/* Free pending refzero entries; quick check to avoid call because often
 * the queue is empty.
 */
#define DUK_REFZERO_CHECK_FAST(thr) do { \
		if ((thr)->heap->refzero_list != NULL) { \
			duk_refzero_free_pending((thr)); \
		} \
	} while (0)
#define DUK_REFZERO_CHECK_SLOW(thr) do { \
		duk_refzero_free_pending((thr)); \
	} while (0)

/*
 *  Macros to set a duk_tval and update refcount of the target (decref the
 *  old value and incref the new value if necessary).  This is both performance
 *  and footprint critical; any changes made should be measured for size/speed.
 */

#define DUK_TVAL_SET_UNDEFINED_UPDREF_ALT0(thr,tvptr_dst) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_UNDEFINED(tv__dst); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)

#define DUK_TVAL_SET_UNDEFINED_UPDREF_NORZ_ALT0(thr,tvptr_dst) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_UNDEFINED(tv__dst); \
		DUK_TVAL_DECREF_NORZ((thr), &tv__tmp); \
	} while (0)

#define DUK_TVAL_SET_UNUSED_UPDREF_ALT0(thr,tvptr_dst) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_UNUSED(tv__dst); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)

#define DUK_TVAL_SET_NULL_UPDREF_ALT0(thr,tvptr_dst) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_NULL(tv__dst); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)

#define DUK_TVAL_SET_BOOLEAN_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_BOOLEAN(tv__dst, (newval)); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)

#define DUK_TVAL_SET_NUMBER_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_NUMBER(tv__dst, (newval)); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)
#define DUK_TVAL_SET_NUMBER_CHKFAST_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_NUMBER_CHKFAST_FAST(tv__dst, (newval)); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)
#define DUK_TVAL_SET_DOUBLE_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_DOUBLE(tv__dst, (newval)); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)
#define DUK_TVAL_SET_NAN_UPDREF_ALT0(thr,tvptr_dst) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_NAN(tv__dst); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)
#if defined(DUK_USE_FASTINT)
#define DUK_TVAL_SET_I48_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_I48(tv__dst, (newval)); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)
#define DUK_TVAL_SET_I32_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_I32(tv__dst, (newval)); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)
#define DUK_TVAL_SET_U32_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_U32(tv__dst, (newval)); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)
#else
#define DUK_TVAL_SET_DOUBLE_CAST_UPDREF(thr,tvptr_dst,newval) \
	DUK_TVAL_SET_DOUBLE_UPDREF((thr), (tvptr_dst), (duk_double_t) (newval))
#endif  /* DUK_USE_FASTINT */

#define DUK_TVAL_SET_LIGHTFUNC_UPDREF_ALT0(thr,tvptr_dst,lf_v,lf_fp,lf_flags) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_LIGHTFUNC(tv__dst, (lf_v), (lf_fp), (lf_flags)); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)

#define DUK_TVAL_SET_STRING_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_STRING(tv__dst, (newval)); \
		DUK_HSTRING_INCREF((thr), (newval)); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)

#define DUK_TVAL_SET_OBJECT_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_OBJECT(tv__dst, (newval)); \
		DUK_HOBJECT_INCREF((thr), (newval)); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)

#define DUK_TVAL_SET_BUFFER_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_BUFFER(tv__dst, (newval)); \
		DUK_HBUFFER_INCREF((thr), (newval)); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)

#define DUK_TVAL_SET_POINTER_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; duk_tval tv__tmp; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_POINTER(tv__dst, (newval)); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)

/* DUK_TVAL_SET_TVAL_UPDREF() is used a lot in executor, property lookups,
 * etc, so it's very important for performance.  Measure when changing.
 *
 * NOTE: the source and destination duk_tval pointers may be the same, and
 * the macros MUST deal with that correctly.
 */

/* Original idiom used, minimal code size. */
#define DUK_TVAL_SET_TVAL_UPDREF_ALT0(thr,tvptr_dst,tvptr_src) do { \
		duk_tval *tv__dst, *tv__src; duk_tval tv__tmp; \
		tv__dst = (tvptr_dst); tv__src = (tvptr_src); \
		DUK_TVAL_SET_TVAL(&tv__tmp, tv__dst); \
		DUK_TVAL_SET_TVAL(tv__dst, tv__src); \
		DUK_TVAL_INCREF((thr), tv__src); \
		DUK_TVAL_DECREF((thr), &tv__tmp);  /* side effects */ \
	} while (0)

/* Faster alternative: avoid making a temporary copy of tvptr_dst and use
 * fast incref/decref macros.
 */
#define DUK_TVAL_SET_TVAL_UPDREF_ALT1(thr,tvptr_dst,tvptr_src) do { \
		duk_tval *tv__dst, *tv__src; duk_heaphdr *h__obj; \
		tv__dst = (tvptr_dst); tv__src = (tvptr_src); \
		DUK_TVAL_INCREF_FAST((thr), tv__src); \
		if (DUK_TVAL_NEEDS_REFCOUNT_UPDATE(tv__dst)) { \
			h__obj = DUK_TVAL_GET_HEAPHDR(tv__dst); \
			DUK_ASSERT(h__obj != NULL); \
			DUK_TVAL_SET_TVAL(tv__dst, tv__src); \
			DUK_HEAPHDR_DECREF_FAST((thr), h__obj);  /* side effects */ \
		} else { \
			DUK_TVAL_SET_TVAL(tv__dst, tv__src); \
		} \
	} while (0)

/* XXX: no optimized variants yet */
#define DUK_TVAL_SET_UNDEFINED_UPDREF         DUK_TVAL_SET_UNDEFINED_UPDREF_ALT0
#define DUK_TVAL_SET_UNDEFINED_UPDREF_NORZ    DUK_TVAL_SET_UNDEFINED_UPDREF_NORZ_ALT0
#define DUK_TVAL_SET_UNUSED_UPDREF            DUK_TVAL_SET_UNUSED_UPDREF_ALT0
#define DUK_TVAL_SET_NULL_UPDREF              DUK_TVAL_SET_NULL_UPDREF_ALT0
#define DUK_TVAL_SET_BOOLEAN_UPDREF           DUK_TVAL_SET_BOOLEAN_UPDREF_ALT0
#define DUK_TVAL_SET_NUMBER_UPDREF            DUK_TVAL_SET_NUMBER_UPDREF_ALT0
#define DUK_TVAL_SET_NUMBER_CHKFAST_UPDREF    DUK_TVAL_SET_NUMBER_CHKFAST_UPDREF_ALT0
#define DUK_TVAL_SET_DOUBLE_UPDREF            DUK_TVAL_SET_DOUBLE_UPDREF_ALT0
#define DUK_TVAL_SET_NAN_UPDREF               DUK_TVAL_SET_NAN_UPDREF_ALT0
#if defined(DUK_USE_FASTINT)
#define DUK_TVAL_SET_I48_UPDREF               DUK_TVAL_SET_I48_UPDREF_ALT0
#define DUK_TVAL_SET_I32_UPDREF               DUK_TVAL_SET_I32_UPDREF_ALT0
#define DUK_TVAL_SET_U32_UPDREF               DUK_TVAL_SET_U32_UPDREF_ALT0
#else
#define DUK_TVAL_SET_I48_UPDREF               DUK_TVAL_SET_DOUBLE_CAST_UPDREF  /* XXX: fast int-to-double */
#define DUK_TVAL_SET_I32_UPDREF               DUK_TVAL_SET_DOUBLE_CAST_UPDREF
#define DUK_TVAL_SET_U32_UPDREF               DUK_TVAL_SET_DOUBLE_CAST_UPDREF
#endif  /* DUK_USE_FASTINT */
#define DUK_TVAL_SET_FASTINT_UPDREF           DUK_TVAL_SET_I48_UPDREF  /* convenience */
#define DUK_TVAL_SET_LIGHTFUNC_UPDREF         DUK_TVAL_SET_LIGHTFUNC_UPDREF_ALT0
#define DUK_TVAL_SET_STRING_UPDREF            DUK_TVAL_SET_STRING_UPDREF_ALT0
#define DUK_TVAL_SET_OBJECT_UPDREF            DUK_TVAL_SET_OBJECT_UPDREF_ALT0
#define DUK_TVAL_SET_BUFFER_UPDREF            DUK_TVAL_SET_BUFFER_UPDREF_ALT0
#define DUK_TVAL_SET_POINTER_UPDREF           DUK_TVAL_SET_POINTER_UPDREF_ALT0

#if defined(DUK_USE_FAST_REFCOUNT_DEFAULT)
/* Optimized for speed. */
#define DUK_TVAL_SET_TVAL_UPDREF              DUK_TVAL_SET_TVAL_UPDREF_ALT1
#define DUK_TVAL_SET_TVAL_UPDREF_FAST         DUK_TVAL_SET_TVAL_UPDREF_ALT1
#define DUK_TVAL_SET_TVAL_UPDREF_SLOW         DUK_TVAL_SET_TVAL_UPDREF_ALT0
#else
/* Optimized for size. */
#define DUK_TVAL_SET_TVAL_UPDREF              DUK_TVAL_SET_TVAL_UPDREF_ALT0
#define DUK_TVAL_SET_TVAL_UPDREF_FAST         DUK_TVAL_SET_TVAL_UPDREF_ALT0
#define DUK_TVAL_SET_TVAL_UPDREF_SLOW         DUK_TVAL_SET_TVAL_UPDREF_ALT0
#endif

#else  /* DUK_USE_REFERENCE_COUNTING */

#define DUK_TVAL_NEEDS_REFCOUNT_UPDATE(tv)     0
#define DUK_HEAPHDR_NEEDS_REFCOUNT_UPDATE(h)   0

#define DUK_TVAL_INCREF_FAST(thr,v)            do {} while (0) /* nop */
#define DUK_TVAL_DECREF_FAST(thr,v)            do {} while (0) /* nop */
#define DUK_TVAL_DECREF_NORZ_FAST(thr,v)       do {} while (0) /* nop */
#define DUK_TVAL_INCREF_SLOW(thr,v)            do {} while (0) /* nop */
#define DUK_TVAL_DECREF_SLOW(thr,v)            do {} while (0) /* nop */
#define DUK_TVAL_DECREF_NORZ_SLOW(thr,v)       do {} while (0) /* nop */
#define DUK_TVAL_INCREF(thr,v)                 do {} while (0) /* nop */
#define DUK_TVAL_DECREF(thr,v)                 do {} while (0) /* nop */
#define DUK_TVAL_DECREF_NORZ(thr,v)            do {} while (0) /* nop */
#define DUK_HEAPHDR_INCREF_FAST(thr,h)         do {} while (0) /* nop */
#define DUK_HEAPHDR_DECREF_FAST(thr,h)         do {} while (0) /* nop */
#define DUK_HEAPHDR_DECREF_NORZ_FAST(thr,h)    do {} while (0) /* nop */
#define DUK_HEAPHDR_INCREF_SLOW(thr,h)         do {} while (0) /* nop */
#define DUK_HEAPHDR_DECREF_SLOW(thr,h)         do {} while (0) /* nop */
#define DUK_HEAPHDR_DECREF_NORZ_SLOW(thr,h)    do {} while (0) /* nop */
#define DUK_HEAPHDR_INCREF(thr,h)              do {} while (0) /* nop */
#define DUK_HEAPHDR_DECREF(thr,h)              do {} while (0) /* nop */
#define DUK_HEAPHDR_DECREF_NORZ(thr,h)         do {} while (0) /* nop */
#define DUK_HSTRING_INCREF_FAST(thr,h)         do {} while (0) /* nop */
#define DUK_HSTRING_DECREF_FAST(thr,h)         do {} while (0) /* nop */
#define DUK_HSTRING_DECREF_NORZ_FAST(thr,h)    do {} while (0) /* nop */
#define DUK_HSTRING_INCREF_SLOW(thr,h)         do {} while (0) /* nop */
#define DUK_HSTRING_DECREF_SLOW(thr,h)         do {} while (0) /* nop */
#define DUK_HSTRING_DECREF_NORZ_SLOW(thr,h)    do {} while (0) /* nop */
#define DUK_HSTRING_INCREF(thr,h)              do {} while (0) /* nop */
#define DUK_HSTRING_DECREF(thr,h)              do {} while (0) /* nop */
#define DUK_HSTRING_DECREF_NORZ(thr,h)         do {} while (0) /* nop */
#define DUK_HOBJECT_INCREF_FAST(thr,h)         do {} while (0) /* nop */
#define DUK_HOBJECT_DECREF_FAST(thr,h)         do {} while (0) /* nop */
#define DUK_HOBJECT_DECREF_NORZ_FAST(thr,h)    do {} while (0) /* nop */
#define DUK_HOBJECT_INCREF_SLOW(thr,h)         do {} while (0) /* nop */
#define DUK_HOBJECT_DECREF_SLOW(thr,h)         do {} while (0) /* nop */
#define DUK_HOBJECT_DECREF_NORZ_SLOW(thr,h)    do {} while (0) /* nop */
#define DUK_HOBJECT_INCREF(thr,h)              do {} while (0) /* nop */
#define DUK_HOBJECT_DECREF(thr,h)              do {} while (0) /* nop */
#define DUK_HOBJECT_DECREF_NORZ(thr,h)         do {} while (0) /* nop */
#define DUK_HBUFFER_INCREF_FAST(thr,h)         do {} while (0) /* nop */
#define DUK_HBUFFER_DECREF_FAST(thr,h)         do {} while (0) /* nop */
#define DUK_HBUFFER_DECREF_NORZ_FAST(thr,h)    do {} while (0) /* nop */
#define DUK_HBUFFER_INCREF_SLOW(thr,h)         do {} while (0) /* nop */
#define DUK_HBUFFER_DECREF_SLOW(thr,h)         do {} while (0) /* nop */
#define DUK_HBUFFER_DECREF_NORZ_SLOW(thr,h)    do {} while (0) /* nop */
#define DUK_HBUFFER_INCREF(thr,h)              do {} while (0) /* nop */
#define DUK_HBUFFER_DECREF(thr,h)              do {} while (0) /* nop */
#define DUK_HBUFFER_DECREF_NORZ(thr,h)         do {} while (0) /* nop */

#define DUK_HCOMPFUNC_INCREF(thr,h)            do {} while (0) /* nop */
#define DUK_HCOMPFUNC_DECREF(thr,h)            do {} while (0) /* nop */
#define DUK_HCOMPFUNC_DECREF_NORZ(thr,h)       do {} while (0) /* nop */
#define DUK_HNATFUNC_INCREF(thr,h)             do {} while (0) /* nop */
#define DUK_HNATFUNC_DECREF(thr,h)             do {} while (0) /* nop */
#define DUK_HNATFUNC_DECREF_NORZ(thr,h)        do {} while (0) /* nop */
#define DUK_HBUFOBJ_INCREF(thr,h)              do {} while (0) /* nop */
#define DUK_HBUFOBJ_DECREF(thr,h)              do {} while (0) /* nop */
#define DUK_HBUFOBJ_DECREF_NORZ(thr,h)         do {} while (0) /* nop */
#define DUK_HTHREAD_INCREF(thr,h)              do {} while (0) /* nop */
#define DUK_HTHREAD_DECREF(thr,h)              do {} while (0) /* nop */
#define DUK_HTHREAD_DECREF_NORZ(thr,h)         do {} while (0) /* nop */
#define DUK_HOBJECT_INCREF_ALLOWNULL(thr,h)    do {} while (0) /* nop */
#define DUK_HOBJECT_DECREF_ALLOWNULL(thr,h)    do {} while (0) /* nop */
#define DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr,h)  do {} while (0) /* nop */
#define DUK_HBUFFER_INCREF_ALLOWNULL(thr,h)    do {} while (0) /* nop */
#define DUK_HBUFFER_DECREF_ALLOWNULL(thr,h)    do {} while (0) /* nop */
#define DUK_HBUFFER_DECREF_NORZ_ALLOWNULL(thr,h)  do {} while (0) /* nop */

#define DUK_REFZERO_CHECK_FAST(thr)            do {} while (0) /* nop */
#define DUK_REFZERO_CHECK_SLOW(thr)            do {} while (0) /* nop */

#define DUK_TVAL_SET_UNDEFINED_UPDREF_ALT0(thr,tvptr_dst) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_UNDEFINED(tv__dst); \
		DUK_UNREF((thr)); \
	} while (0)

#define DUK_TVAL_SET_UNUSED_UPDREF_ALT0(thr,tvptr_dst) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_UNUSED(tv__dst); \
		DUK_UNREF((thr)); \
	} while (0)

#define DUK_TVAL_SET_NULL_UPDREF_ALT0(thr,tvptr_dst) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_NULL(tv__dst); \
		DUK_UNREF((thr)); \
	} while (0)

#define DUK_TVAL_SET_BOOLEAN_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_BOOLEAN(tv__dst, (newval)); \
		DUK_UNREF((thr)); \
	} while (0)

#define DUK_TVAL_SET_NUMBER_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_NUMBER(tv__dst, (newval)); \
		DUK_UNREF((thr)); \
	} while (0)
#define DUK_TVAL_SET_NUMBER_CHKFAST_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_NUMBER_CHKFAST_FAST(tv__dst, (newval)); \
		DUK_UNREF((thr)); \
	} while (0)
#define DUK_TVAL_SET_DOUBLE_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_DOUBLE(tv__dst, (newval)); \
		DUK_UNREF((thr)); \
	} while (0)
#define DUK_TVAL_SET_NAN_UPDREF_ALT0(thr,tvptr_dst) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_NAN(tv__dst); \
		DUK_UNREF((thr)); \
	} while (0)
#if defined(DUK_USE_FASTINT)
#define DUK_TVAL_SET_I48_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_I48(tv__dst, (newval)); \
		DUK_UNREF((thr)); \
	} while (0)
#define DUK_TVAL_SET_I32_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_I32(tv__dst, (newval)); \
		DUK_UNREF((thr)); \
	} while (0)
#define DUK_TVAL_SET_U32_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_U32(tv__dst, (newval)); \
		DUK_UNREF((thr)); \
	} while (0)
#else
#define DUK_TVAL_SET_DOUBLE_CAST_UPDREF(thr,tvptr_dst,newval) \
	DUK_TVAL_SET_DOUBLE_UPDREF((thr), (tvptr_dst), (duk_double_t) (newval))
#endif  /* DUK_USE_FASTINT */

#define DUK_TVAL_SET_LIGHTFUNC_UPDREF_ALT0(thr,tvptr_dst,lf_v,lf_fp,lf_flags) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_LIGHTFUNC(tv__dst, (lf_v), (lf_fp), (lf_flags)); \
		DUK_UNREF((thr)); \
	} while (0)

#define DUK_TVAL_SET_STRING_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_STRING(tv__dst, (newval)); \
		DUK_UNREF((thr)); \
	} while (0)

#define DUK_TVAL_SET_OBJECT_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_OBJECT(tv__dst, (newval)); \
		DUK_UNREF((thr)); \
	} while (0)

#define DUK_TVAL_SET_BUFFER_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_BUFFER(tv__dst, (newval)); \
		DUK_UNREF((thr)); \
	} while (0)

#define DUK_TVAL_SET_POINTER_UPDREF_ALT0(thr,tvptr_dst,newval) do { \
		duk_tval *tv__dst; tv__dst = (tvptr_dst); \
		DUK_TVAL_SET_POINTER(tv__dst, (newval)); \
		DUK_UNREF((thr)); \
	} while (0)

#define DUK_TVAL_SET_TVAL_UPDREF_ALT0(thr,tvptr_dst,tvptr_src) do { \
		duk_tval *tv__dst, *tv__src; \
		tv__dst = (tvptr_dst); tv__src = (tvptr_src); \
		DUK_TVAL_SET_TVAL(tv__dst, tv__src); \
		DUK_UNREF((thr)); \
	} while (0)

#define DUK_TVAL_SET_UNDEFINED_UPDREF         DUK_TVAL_SET_UNDEFINED_UPDREF_ALT0
#define DUK_TVAL_SET_UNDEFINED_UPDREF_NORZ    DUK_TVAL_SET_UNDEFINED_UPDREF_ALT0
#define DUK_TVAL_SET_UNUSED_UPDREF            DUK_TVAL_SET_UNUSED_UPDREF_ALT0
#define DUK_TVAL_SET_NULL_UPDREF              DUK_TVAL_SET_NULL_UPDREF_ALT0
#define DUK_TVAL_SET_BOOLEAN_UPDREF           DUK_TVAL_SET_BOOLEAN_UPDREF_ALT0
#define DUK_TVAL_SET_NUMBER_UPDREF            DUK_TVAL_SET_NUMBER_UPDREF_ALT0
#define DUK_TVAL_SET_NUMBER_CHKFAST_UPDREF    DUK_TVAL_SET_NUMBER_CHKFAST_UPDREF_ALT0
#define DUK_TVAL_SET_DOUBLE_UPDREF            DUK_TVAL_SET_DOUBLE_UPDREF_ALT0
#define DUK_TVAL_SET_NAN_UPDREF               DUK_TVAL_SET_NAN_UPDREF_ALT0
#if defined(DUK_USE_FASTINT)
#define DUK_TVAL_SET_I48_UPDREF               DUK_TVAL_SET_I48_UPDREF_ALT0
#define DUK_TVAL_SET_I32_UPDREF               DUK_TVAL_SET_I32_UPDREF_ALT0
#define DUK_TVAL_SET_U32_UPDREF               DUK_TVAL_SET_U32_UPDREF_ALT0
#else
#define DUK_TVAL_SET_I48_UPDREF               DUK_TVAL_SET_DOUBLE_CAST_UPDREF  /* XXX: fast-int-to-double */
#define DUK_TVAL_SET_I32_UPDREF               DUK_TVAL_SET_DOUBLE_CAST_UPDREF
#define DUK_TVAL_SET_U32_UPDREF               DUK_TVAL_SET_DOUBLE_CAST_UPDREF
#endif  /* DUK_USE_FASTINT */
#define DUK_TVAL_SET_FASTINT_UPDREF           DUK_TVAL_SET_I48_UPDREF  /* convenience */
#define DUK_TVAL_SET_LIGHTFUNC_UPDREF         DUK_TVAL_SET_LIGHTFUNC_UPDREF_ALT0
#define DUK_TVAL_SET_STRING_UPDREF            DUK_TVAL_SET_STRING_UPDREF_ALT0
#define DUK_TVAL_SET_OBJECT_UPDREF            DUK_TVAL_SET_OBJECT_UPDREF_ALT0
#define DUK_TVAL_SET_BUFFER_UPDREF            DUK_TVAL_SET_BUFFER_UPDREF_ALT0
#define DUK_TVAL_SET_POINTER_UPDREF           DUK_TVAL_SET_POINTER_UPDREF_ALT0

#define DUK_TVAL_SET_TVAL_UPDREF              DUK_TVAL_SET_TVAL_UPDREF_ALT0
#define DUK_TVAL_SET_TVAL_UPDREF_FAST         DUK_TVAL_SET_TVAL_UPDREF_ALT0
#define DUK_TVAL_SET_TVAL_UPDREF_SLOW         DUK_TVAL_SET_TVAL_UPDREF_ALT0

#endif  /* DUK_USE_REFERENCE_COUNTING */

#endif  /* DUK_HEAPHDR_H_INCLUDED */
