/*
 *  Heap string table handling, string interning.
 */

#include "duk_internal.h"

/* Avoid finalizers during string table resize because a finalizer may e.g.
 * resize a dynamic buffer whose data area is used as the source for a string
 * intern operation.  String table traffic (interning strings) is fine.
 */
#define DUK__PREVENT_MS_SIDE_EFFECTS(heap) do { \
		(heap)->mark_and_sweep_base_flags |= \
		        DUK_MS_FLAG_NO_FINALIZERS; \
	} while (0)

/* Resize checks not needed if minsize == maxsize, typical for low memory
 * targets.
 */
#define DUK__STRTAB_RESIZE_CHECK
#if (DUK_USE_STRTAB_MINSIZE == DUK_USE_STRTAB_MAXSIZE)
#undef DUK__STRTAB_RESIZE_CHECK
#endif

#if defined(DUK_USE_STRTAB_PTRCOMP)
#define DUK__HEAPPTR_ENC16(heap,ptr)    DUK_USE_HEAPPTR_ENC16((heap)->heap_udata, (ptr))
#define DUK__HEAPPTR_DEC16(heap,val)    DUK_USE_HEAPPTR_DEC16((heap)->heap_udata, (val))
#define DUK__GET_STRTABLE(heap)         ((heap)->strtable16)
#else
#define DUK__HEAPPTR_ENC16(heap,ptr)    (ptr)
#define DUK__HEAPPTR_DEC16(heap,val)    (val)
#define DUK__GET_STRTABLE(heap)         ((heap)->strtable)
#endif

#define DUK__STRTAB_U32_MAX_STRLEN      10               /* 4'294'967'295 */

/*
 *  Debug dump stringtable.
 */

#if defined(DUK_USE_DEBUG)
DUK_INTERNAL void duk_heap_strtable_dump(duk_heap *heap) {
#if defined(DUK_USE_STRTAB_PTRCOMP)
	duk_uint16_t *strtable;
#else
	duk_hstring **strtable;
#endif
	duk_uint32_t i;
	duk_hstring *h;
	duk_size_t count_total = 0;
	duk_size_t count_chain;
	duk_size_t count_chain_min = DUK_SIZE_MAX;
	duk_size_t count_chain_max = 0;
	duk_size_t count_len[8];  /* chain lengths from 0 to 7 */

	if (heap == NULL) {
		DUK_D(DUK_DPRINT("string table, heap=NULL"));
		return;
	}

	strtable = DUK__GET_STRTABLE(heap);
	if (strtable == NULL) {
		DUK_D(DUK_DPRINT("string table, strtab=NULL"));
		return;
	}

	DUK_MEMZERO((void *) count_len, sizeof(count_len));
	for (i = 0; i < heap->st_size; i++) {
		h = DUK__HEAPPTR_DEC16(heap, strtable[i]);
		count_chain = 0;
		while (h != NULL) {
			count_chain++;
			h = h->hdr.h_next;
		}
		if (count_chain < sizeof(count_len) / sizeof(duk_size_t)) {
			count_len[count_chain]++;
		}
		count_chain_max = (count_chain > count_chain_max ? count_chain : count_chain_max);
		count_chain_min = (count_chain < count_chain_min ? count_chain : count_chain_min);
		count_total += count_chain;
	}

	DUK_D(DUK_DPRINT("string table, strtab=%p, count=%lu, chain min=%lu max=%lu avg=%lf: "
	                 "counts: %lu %lu %lu %lu %lu %lu %lu %lu ...",
	                 (void *) heap->strtable, (unsigned long) count_total,
	                 (unsigned long) count_chain_min, (unsigned long) count_chain_max,
	                 (double) count_total / (double) heap->st_size,
	                 (unsigned long) count_len[0], (unsigned long) count_len[1],
	                 (unsigned long) count_len[2], (unsigned long) count_len[3],
	                 (unsigned long) count_len[4], (unsigned long) count_len[5],
	                 (unsigned long) count_len[6], (unsigned long) count_len[7]));
}
#endif  /* DUK_USE_DEBUG */

/*
 *  Assertion helper to ensure strtable is populated correctly.
 */

#if defined(DUK_USE_ASSERTIONS)
DUK_LOCAL void duk__strtable_assert_checks(duk_heap *heap) {
#if defined(DUK_USE_STRTAB_PTRCOMP)
	duk_uint16_t *strtable;
#else
	duk_hstring **strtable;
#endif
	duk_uint32_t i;
	duk_hstring *h;
	duk_size_t count = 0;

	DUK_ASSERT(heap != NULL);

	strtable = DUK__GET_STRTABLE(heap);
	if (strtable != NULL) {
		DUK_ASSERT(heap->st_size != 0);
		DUK_ASSERT(heap->st_mask == heap->st_size - 1);

		for (i = 0; i < heap->st_size; i++) {
			h = DUK__HEAPPTR_DEC16(heap, strtable[i]);
			while (h != NULL) {
				DUK_ASSERT((DUK_HSTRING_GET_HASH(h) & heap->st_mask) == i);
				count++;
				h = h->hdr.h_next;
			}
		}
	} else {
		DUK_ASSERT(heap->st_size == 0);
		DUK_ASSERT(heap->st_mask == 0);
	}

#if defined(DUK__STRTAB_RESIZE_CHECK)
	DUK_ASSERT(count == (duk_size_t) heap->st_count);
#endif
}
#endif  /* DUK_USE_ASSERTIONS */

/*
 *  Allocate and initialize a duk_hstring.
 *
 *  Returns a NULL if allocation or initialization fails for some reason.
 *
 *  The string won't be inserted into the string table and isn't tracked in
 *  any way (link pointers will be NULL).  The caller must place the string
 *  into the string table without any risk of a longjmp, otherwise the string
 *  is leaked.
 */

DUK_LOCAL duk_hstring *duk__strtable_alloc_hstring(duk_heap *heap,
                                                   const duk_uint8_t *str,
                                                   duk_uint32_t blen,
                                                   duk_uint32_t strhash,
                                                   const duk_uint8_t *extdata) {
	duk_hstring *res;
	const duk_uint8_t *data;
	duk_uarridx_t arridx;
	duk_uint32_t clen;

	DUK_ASSERT(heap != NULL);
	DUK_UNREF(extdata);

#if defined(DUK_USE_STRLEN16)
	/* If blen <= 0xffffUL, clen is also guaranteed to be <= 0xffffUL. */
	if (blen > 0xffffUL) {
		DUK_D(DUK_DPRINT("16-bit string blen/clen active and blen over 16 bits, reject intern"));
		goto alloc_error;
	}
#endif

	/* XXX: Memzeroing the allocated structure is not really necessary
	 * because we could just initialize all fields explicitly (almost
	 * all fields are initialized explicitly anyway).
	 */
#if defined(DUK_USE_HSTRING_EXTDATA) && defined(DUK_USE_EXTSTR_INTERN_CHECK)
	if (extdata) {
		res = (duk_hstring *) DUK_ALLOC(heap, sizeof(duk_hstring_external));
		if (DUK_UNLIKELY(res == NULL)) {
			goto alloc_error;
		}
		DUK_MEMZERO(res, sizeof(duk_hstring_external));
#if defined(DUK_USE_EXPLICIT_NULL_INIT)
		DUK_HEAPHDR_STRING_INIT_NULLS(&res->hdr);
#endif
		DUK_HEAPHDR_SET_TYPE_AND_FLAGS(&res->hdr, DUK_HTYPE_STRING, DUK_HSTRING_FLAG_EXTDATA);

		DUK_ASSERT(extdata[blen] == 0);  /* Application responsibility. */
		data = extdata;
		((duk_hstring_external *) res)->extdata = extdata;
	} else
#endif  /* DUK_USE_HSTRING_EXTDATA && DUK_USE_EXTSTR_INTERN_CHECK */
	{
		duk_uint8_t *data_tmp;

		/* NUL terminate for convenient C access */
		DUK_ASSERT(sizeof(duk_hstring) + blen + 1 > blen);  /* No wrap, limits ensure. */
		res = (duk_hstring *) DUK_ALLOC(heap, sizeof(duk_hstring) + blen + 1);
		if (DUK_UNLIKELY(res == NULL)) {
			goto alloc_error;
		}
		DUK_MEMZERO(res, sizeof(duk_hstring));
#if defined(DUK_USE_EXPLICIT_NULL_INIT)
		DUK_HEAPHDR_STRING_INIT_NULLS(&res->hdr);
#endif
		DUK_HEAPHDR_SET_TYPE_AND_FLAGS(&res->hdr, DUK_HTYPE_STRING, 0);

		data_tmp = (duk_uint8_t *) (res + 1);
		DUK_MEMCPY(data_tmp, str, blen);
		data_tmp[blen] = (duk_uint8_t) 0;
		data = (const duk_uint8_t *) data_tmp;
	}

	/* Because 'data' is NUL-terminated, we don't need a
	 * blen > 0 check here.  For NUL (0x00) the symbol
	 * checks will be false.  Note that 0x80-0xBF is not
	 * counted towards clen, but 0xFF is.
	 */
	if (DUK_UNLIKELY(data[0] >= 0x80U)) {
		if (data[0] == 0xffU) {
			DUK_HSTRING_SET_SYMBOL(res);
			DUK_HSTRING_SET_HIDDEN(res);
		} else if (data[0] <= 0xbf) {
			/* Check equivalent to: (data[0] & 0xc0U) == 0x80U. */
			DUK_HSTRING_SET_SYMBOL(res);
		}
	}

	DUK_HSTRING_SET_BYTELEN(res, blen);
	DUK_HSTRING_SET_HASH(res, strhash);

	clen = (duk_uint32_t) duk_unicode_unvalidated_utf8_length(data, (duk_size_t) blen);
	DUK_ASSERT(clen <= blen);

#if defined(DUK_USE_HSTRING_CLEN)
	DUK_HSTRING_SET_CHARLEN(res, clen);
#endif

	DUK_ASSERT(!DUK_HSTRING_HAS_ASCII(res));
	if (DUK_LIKELY(clen == blen)) {
		/* Only pure ASCII strings can be array index strings.
		 *
		 * Using an explicit 'ASCII' flag has larger footprint (one
		 * call site only) but is quite useful for the case when
		 * there's no explicit 'clen' in duk_hstring.
		 */
		DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(res));
		arridx = duk_js_to_arrayindex_string(data, blen);
		if (arridx != DUK_HSTRING_NO_ARRAY_INDEX) {
			DUK_HSTRING_SET_ARRIDX(res);
		}
		DUK_HSTRING_SET_ASCII(res);
#if defined(DUK_USE_HSTRING_ARRIDX)
		res->arridx = arridx;
#endif
	}

	DUK_DDD(DUK_DDDPRINT("interned string, hash=0x%08lx, blen=%ld, clen=%ld, has_arridx=%ld, has_extdata=%ld",
	                     (unsigned long) DUK_HSTRING_GET_HASH(res),
	                     (long) DUK_HSTRING_GET_BYTELEN(res),
	                     (long) DUK_HSTRING_GET_CHARLEN(res),
	                     (long) (DUK_HSTRING_HAS_ARRIDX(res) ? 1 : 0),
	                     (long) (DUK_HSTRING_HAS_EXTDATA(res) ? 1 : 0)));

	DUK_ASSERT(res != NULL);
	return res;

 alloc_error:
	return NULL;
}

/*
 *  Grow strtable allocation in-place.
 */

#if defined(DUK__STRTAB_RESIZE_CHECK)
DUK_LOCAL void duk__strtable_grow_inplace(duk_heap *heap) {
	duk_uint32_t new_st_size;
	duk_uint32_t old_st_size;
	duk_uint32_t i;
	duk_hstring *h;
	duk_hstring *next;
	duk_hstring *prev;
#if defined(DUK_USE_STRTAB_PTRCOMP)
	duk_uint16_t *new_ptr;
	duk_uint16_t *new_ptr_high;
#else
	duk_hstring **new_ptr;
	duk_hstring **new_ptr_high;
#endif

	DUK_DD(DUK_DDPRINT("grow in-place: %lu -> %lu", (unsigned long) heap->st_size, (unsigned long) heap->st_size * 2));

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(heap->st_resizing == 1);
	DUK_ASSERT(heap->st_size >= 2);
	DUK_ASSERT((heap->st_size & (heap->st_size - 1)) == 0);  /* 2^N */
	DUK_ASSERT(DUK__GET_STRTABLE(heap) != NULL);

	new_st_size = heap->st_size << 1U;
	DUK_ASSERT(new_st_size > heap->st_size);  /* No overflow. */

	/* Reallocate the strtable first and then work in-place to rehash
	 * strings.  We don't need an indirect allocation here: even if GC
	 * is triggered to satisfy the allocation, recursive strtable resize
	 * is prevented by flags.  This is also why we don't need to use
	 * DUK_REALLOC_INDIRECT().
	 */

#if defined(DUK_USE_STRTAB_PTRCOMP)
	new_ptr = (duk_uint16_t *) DUK_REALLOC(heap, heap->strtable16, sizeof(duk_uint16_t) * new_st_size);
#else
	new_ptr = (duk_hstring **) DUK_REALLOC(heap, heap->strtable, sizeof(duk_hstring *) * new_st_size);
#endif
	if (DUK_UNLIKELY(new_ptr == NULL)) {
		/* If realloc fails we can continue normally: the string table
		 * won't "fill up" although chains will gradually get longer.
		 * When string insertions continue, we'll quite soon try again
		 * with no special handling.
		 */
		DUK_D(DUK_DPRINT("string table grow failed, ignoring"));
		return;
	}
#if defined(DUK_USE_STRTAB_PTRCOMP)
	heap->strtable16 = new_ptr;
#else
	heap->strtable = new_ptr;
#endif

	/* Rehash a single bucket into two separate ones.  When we grow
	 * by x2 the highest 'new' bit determines whether a string remains
	 * in its old position (bit is 0) or goes to a new one (bit is 1).
	 */

	old_st_size = heap->st_size;
	new_ptr_high = new_ptr + old_st_size;
	for (i = 0; i < old_st_size; i++) {
		duk_hstring *new_root;
		duk_hstring *new_root_high;

		h = DUK__HEAPPTR_DEC16(heap, new_ptr[i]);
		new_root = h;
		new_root_high = NULL;

		prev = NULL;
		while (h != NULL) {
			duk_uint32_t mask;

			DUK_ASSERT((DUK_HSTRING_GET_HASH(h) & heap->st_mask) == i);
			next = h->hdr.h_next;

			/* Example: if previous size was 256, previous mask is 0xFF
			 * and size is 0x100 which corresponds to the new bit that
			 * comes into play.
			 */
			DUK_ASSERT(heap->st_mask == old_st_size - 1);
			mask = old_st_size;
			if (DUK_HSTRING_GET_HASH(h) & mask) {
				if (prev != NULL) {
					prev->hdr.h_next = h->hdr.h_next;
				} else {
					DUK_ASSERT(h == new_root);
					new_root = h->hdr.h_next;
				}

				h->hdr.h_next = new_root_high;
				new_root_high = h;
			} else {
				prev = h;
			}
			h = next;
		}

		new_ptr[i] = DUK__HEAPPTR_ENC16(heap, new_root);
		new_ptr_high[i] = DUK__HEAPPTR_ENC16(heap, new_root_high);
	}

	heap->st_size = new_st_size;
	heap->st_mask = new_st_size - 1;

#if defined(DUK_USE_ASSERTIONS)
	duk__strtable_assert_checks(heap);
#endif
}
#endif  /* DUK__STRTAB_RESIZE_CHECK */

/*
 *  Shrink strtable allocation in-place.
 */

#if defined(DUK__STRTAB_RESIZE_CHECK)
DUK_LOCAL void duk__strtable_shrink_inplace(duk_heap *heap) {
	duk_uint32_t new_st_size;
	duk_uint32_t i;
	duk_hstring *h;
	duk_hstring *other;
	duk_hstring *root;
#if defined(DUK_USE_STRTAB_PTRCOMP)
	duk_uint16_t *old_ptr;
	duk_uint16_t *old_ptr_high;
	duk_uint16_t *new_ptr;
#else
	duk_hstring **old_ptr;
	duk_hstring **old_ptr_high;
	duk_hstring **new_ptr;
#endif

	DUK_DD(DUK_DDPRINT("shrink in-place: %lu -> %lu", (unsigned long) heap->st_size, (unsigned long) heap->st_size / 2));

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(heap->st_resizing == 1);
	DUK_ASSERT(heap->st_size >= 2);
	DUK_ASSERT((heap->st_size & (heap->st_size - 1)) == 0);  /* 2^N */
	DUK_ASSERT(DUK__GET_STRTABLE(heap) != NULL);

	new_st_size = heap->st_size >> 1U;

	/* Combine two buckets into a single one.  When we shrink, one hash
	 * bit (highest) disappears.
	 */
	old_ptr = DUK__GET_STRTABLE(heap);
	old_ptr_high = old_ptr + new_st_size;
	for (i = 0; i < new_st_size; i++) {
		h = DUK__HEAPPTR_DEC16(heap, old_ptr[i]);
		other = DUK__HEAPPTR_DEC16(heap, old_ptr_high[i]);

		if (h == NULL) {
			/* First chain is empty, so use second one as is. */
			root = other;
		} else {
			/* Find end of first chain, and link in the second. */
			root = h;
			while (h->hdr.h_next != NULL) {
				h = h->hdr.h_next;
			}
			h->hdr.h_next = other;
		}

		old_ptr[i] = DUK__HEAPPTR_ENC16(heap, root);
	}

	heap->st_size = new_st_size;
	heap->st_mask = new_st_size - 1;

	/* The strtable is now consistent and we can realloc safely.  Even
	 * if side effects cause string interning or removal the strtable
	 * updates are safe.  Recursive resize has been prevented by caller.
	 * This is also why we don't need to use DUK_REALLOC_INDIRECT().
	 *
	 * We assume a realloc() to a smaller size is guaranteed to succeed.
	 * It would be relatively straightforward to handle the error by
	 * essentially performing a "grow" step to recover.
	 */

#if defined(DUK_USE_STRTAB_PTRCOMP)
	new_ptr = (duk_uint16_t *) DUK_REALLOC(heap, heap->strtable16, sizeof(duk_uint16_t) * new_st_size);
	DUK_ASSERT(new_ptr != NULL);
	heap->strtable16 = new_ptr;
#else
	new_ptr = (duk_hstring **) DUK_REALLOC(heap, heap->strtable, sizeof(duk_hstring *) * new_st_size);
	DUK_ASSERT(new_ptr != NULL);
	heap->strtable = new_ptr;
#endif

#if defined(DUK_USE_ASSERTIONS)
	duk__strtable_assert_checks(heap);
#endif
}
#endif  /* DUK__STRTAB_RESIZE_CHECK */

/*
 *  Grow/shrink check.
 */

#if defined(DUK__STRTAB_RESIZE_CHECK)
DUK_LOCAL DUK_COLD DUK_NOINLINE void duk__strtable_resize_check(duk_heap *heap) {
	duk_uint32_t load_factor;  /* fixed point */

	DUK_ASSERT(heap != NULL);
#if defined(DUK_USE_STRTAB_PTRCOMP)
	DUK_ASSERT(heap->strtable16 != NULL);
#else
	DUK_ASSERT(heap->strtable != NULL);
#endif

	/* Prevent recursive resizing. */
	if (DUK_UNLIKELY(heap->st_resizing)) {
		DUK_D(DUK_DPRINT("prevent recursive strtable resize"));
		return;
	}

	heap->st_resizing = 1;

	DUK_ASSERT(heap->st_size >= 16U);
	DUK_ASSERT((heap->st_size >> 4U) >= 1);
	load_factor = heap->st_count / (heap->st_size >> 4U);

	DUK_DD(DUK_DDPRINT("resize check string table: size=%lu, count=%lu, load_factor=%lu (fixed point .4; float %lf)",
	                   (unsigned long) heap->st_size, (unsigned long) heap->st_count,
	                   (unsigned long) load_factor,
	                   (double) heap->st_count / (double) heap->st_size));

	if (load_factor >= DUK_USE_STRTAB_GROW_LIMIT) {
		if (heap->st_size >= DUK_USE_STRTAB_MAXSIZE) {
			DUK_DD(DUK_DDPRINT("want to grow strtable (based on load factor) but already maximum size"));
		} else {
			DUK_D(DUK_DPRINT("grow string table: %lu -> %lu", (unsigned long) heap->st_size, (unsigned long) heap->st_size * 2));
#if defined(DUK_USE_DEBUG)
			duk_heap_strtable_dump(heap);
#endif
			duk__strtable_grow_inplace(heap);
		}
	} else if (load_factor <= DUK_USE_STRTAB_SHRINK_LIMIT) {
		if (heap->st_size <= DUK_USE_STRTAB_MINSIZE) {
			DUK_DD(DUK_DDPRINT("want to shrink strtable (based on load factor) but already minimum size"));
		} else {
			DUK_D(DUK_DPRINT("shrink string table: %lu -> %lu", (unsigned long) heap->st_size, (unsigned long) heap->st_size / 2));
#if defined(DUK_USE_DEBUG)
			duk_heap_strtable_dump(heap);
#endif
			duk__strtable_shrink_inplace(heap);
		}
	} else {
		DUK_DD(DUK_DDPRINT("no need for strtable resize"));
	}

	heap->st_resizing = 0;
}
#endif  /* DUK__STRTAB_RESIZE_CHECK */

/*
 *  Torture grow/shrink: unconditionally grow and shrink back.
 */

#if defined(DUK_USE_STRTAB_TORTURE) && defined(DUK__STRTAB_RESIZE_CHECK)
DUK_LOCAL void duk__strtable_resize_torture(duk_heap *heap) {
	duk_uint32_t old_st_size;

	DUK_ASSERT(heap != NULL);

	old_st_size = heap->st_size;
	if (old_st_size >= DUK_USE_STRTAB_MAXSIZE) {
		return;
	}

	heap->st_resizing = 1;
	duk__strtable_grow_inplace(heap);
	if (heap->st_size > old_st_size) {
		duk__strtable_shrink_inplace(heap);
	}
	heap->st_resizing = 0;
}
#endif  /* DUK_USE_STRTAB_TORTURE && DUK__STRTAB_RESIZE_CHECK */

/*
 *  Raw intern; string already checked not to be present.
 */

DUK_LOCAL duk_hstring *duk__strtable_do_intern(duk_heap *heap, const duk_uint8_t *str, duk_uint32_t blen, duk_uint32_t strhash) {
	duk_hstring *res;
	const duk_uint8_t *extdata;
#if defined(DUK_USE_STRTAB_PTRCOMP)
	duk_uint16_t *slot;
#else
	duk_hstring **slot;
#endif
	duk_small_uint_t prev_mark_and_sweep_base_flags;

	DUK_DDD(DUK_DDDPRINT("do_intern: heap=%p, str=%p, blen=%lu, strhash=%lx, st_size=%lu, st_count=%lu, load=%lf",
	                     (void *) heap, (const void *) str, (unsigned long) blen, (unsigned long) strhash,
	                     (unsigned long) heap->st_size, (unsigned long) heap->st_count,
	                     (double) heap->st_count / (double) heap->st_size));

	DUK_ASSERT(heap != NULL);

	/* Prevent any side effects on the string table and the caller provided
	 * str/blen arguments while interning is in progress.  For example, if
	 * the caller provided str/blen from a dynamic buffer, a finalizer
	 * might resize or modify that dynamic buffer, invalidating the call
	 * arguments.
	 */

	prev_mark_and_sweep_base_flags = heap->mark_and_sweep_base_flags;
	DUK__PREVENT_MS_SIDE_EFFECTS(heap);

#if defined(DUK_USE_STRTAB_TORTURE) && defined(DUK__STRTAB_RESIZE_CHECK)
	duk__strtable_resize_torture(heap);
#endif

	/* String table grow/shrink check.  Because of chaining (and no
	 * accumulation issues as with hash probe chains and DELETED
	 * markers) there's never a mandatory need to resize right now.
	 * Check for the resize only periodically, based on st_count
	 * bit pattern.  Because string table removal doesn't do a shrink
	 * check, we do that also here.
	 *
	 * Do the resize and possible grow/shrink before the new duk_hstring
	 * has been allocated.  Otherwise we may trigger a GC when the result
	 * duk_hstring is not yet strongly referenced.
	 */

#if defined(DUK__STRTAB_RESIZE_CHECK)
	if (DUK_UNLIKELY((heap->st_count & DUK_USE_STRTAB_RESIZE_CHECK_MASK) == 0)) {
		duk__strtable_resize_check(heap);
	}
#endif

	/* External string check (low memory optimization). */

#if defined(DUK_USE_HSTRING_EXTDATA) && defined(DUK_USE_EXTSTR_INTERN_CHECK)
	extdata = (const duk_uint8_t *) DUK_USE_EXTSTR_INTERN_CHECK(heap->heap_udata, (void *) DUK_LOSE_CONST(str), (duk_size_t) blen);
#else
	extdata = (const duk_uint8_t *) NULL;
#endif

	/* Allocate and initialize string, not yet linked.  This may cause a
	 * GC which may cause other strings to be interned and inserted into
	 * the string table before we insert our string.  Finalizer execution
	 * is disabled intentionally to avoid a finalizer from e.g. resizing
	 * a buffer used as a data area for 'str'.
	 */

	res = duk__strtable_alloc_hstring(heap, str, blen, strhash, extdata);

	/* Allow side effects again: GC must be avoided until duk_hstring
	 * result (if successful) has been INCREF'd.
	 */

	heap->mark_and_sweep_base_flags = prev_mark_and_sweep_base_flags;

	/* Alloc error handling. */

	if (DUK_UNLIKELY(res == NULL)) {
#if defined(DUK_USE_HSTRING_EXTDATA) && defined(DUK_USE_EXTSTR_INTERN_CHECK)
		if (extdata != NULL) {
			DUK_USE_EXTSTR_FREE(heap->heap_udata, (const void *) extdata);
		}
#endif
		return NULL;
	}

	/* Insert into string table. */

#if defined(DUK_USE_STRTAB_PTRCOMP)
	slot = heap->strtable16 + (strhash & heap->st_mask);
#else
	slot = heap->strtable + (strhash & heap->st_mask);
#endif
	DUK_ASSERT(res->hdr.h_next == NULL);  /* This is the case now, but unnecessary zeroing/NULLing. */
	res->hdr.h_next = DUK__HEAPPTR_DEC16(heap, *slot);
	*slot = DUK__HEAPPTR_ENC16(heap, res);

	/* Update string count only for successful inserts. */

#if defined(DUK__STRTAB_RESIZE_CHECK)
	heap->st_count++;
#endif

	/* The duk_hstring is in the string table but is not yet strongly
	 * reachable.  Calling code MUST NOT make any allocations or other
	 * side effects before the duk_hstring has been INCREF'd and made
	 * reachable.
	 */

	return res;
}

/*
 *  Intern a string from str/blen, returning either an existing duk_hstring
 *  or adding a new one into the string table.  The input string does -not-
 *  need to be NUL terminated.
 *
 *  The input 'str' argument may point to a Duktape managed data area such as
 *  the data area of a dynamic buffer.  It's crucial to avoid any side effects
 *  that might affect the data area (e.g. resize the dynamic buffer, or write
 *  to the buffer) before the string is fully interned.
 */

#if defined(DUK_USE_ROM_STRINGS)
DUK_LOCAL duk_hstring *duk__strtab_romstring_lookup(duk_heap *heap, const duk_uint8_t *str, duk_size_t blen, duk_uint32_t strhash) {
	duk_small_uint_t i;

	DUK_ASSERT(heap != NULL);

	/* XXX: This is VERY inefficient now, and should be e.g. a
	 * binary search or perfect hash, to be fixed.
	 */
	for (i = 0; i < (duk_small_uint_t) (sizeof(duk_rom_strings) / sizeof(duk_hstring *)); i++) {
		duk_hstring *romstr;

		romstr = (duk_hstring *) DUK_LOSE_CONST(duk_rom_strings[i]);

		if (strhash == DUK_HSTRING_GET_HASH(romstr) &&
		    blen == DUK_HSTRING_GET_BYTELEN(romstr) &&
		    DUK_MEMCMP((const void *) str, (const void *) DUK_HSTRING_GET_DATA(romstr), blen) == 0) {
			DUK_DDD(DUK_DDDPRINT("intern check: rom string: %!O, computed hash 0x%08lx, rom hash 0x%08lx",
			                     romstr, (unsigned long) strhash, (unsigned long) DUK_HSTRING_GET_HASH(romstr)));
			return romstr;
		}
	}
	return NULL;
}
#endif  /* DUK_USE_ROM_STRINGS */

DUK_INTERNAL duk_hstring *duk_heap_strtable_intern(duk_heap *heap, const duk_uint8_t *str, duk_uint32_t blen) {
	duk_uint32_t strhash;
	duk_hstring *h;

	DUK_DDD(DUK_DDDPRINT("intern check: heap=%p, str=%p, blen=%lu", (void *) heap, (const void *) str, (unsigned long) blen));

	/* Preliminaries. */

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(blen == 0 || str != NULL);
	DUK_ASSERT(blen <= DUK_HSTRING_MAX_BYTELEN);  /* Caller is responsible for ensuring this. */
	strhash = duk_heap_hashstring(heap, str, (duk_size_t) blen);

	/* String table lookup. */

	DUK_ASSERT(DUK__GET_STRTABLE(heap) != NULL);
	DUK_ASSERT(heap->st_size > 0);
	DUK_ASSERT(heap->st_size == heap->st_mask + 1);
#if defined(DUK_USE_STRTAB_PTRCOMP)
	h = DUK__HEAPPTR_DEC16(heap, heap->strtable16[strhash & heap->st_mask]);
#else
	h = heap->strtable[strhash & heap->st_mask];
#endif
	while (h != NULL) {
		if (DUK_HSTRING_GET_HASH(h) == strhash &&
		    DUK_HSTRING_GET_BYTELEN(h) == blen &&
		    DUK_MEMCMP((const void *) str, (const void *) DUK_HSTRING_GET_DATA(h), (size_t) blen) == 0) {
			/* Found existing entry. */
			return h;
		}
		h = h->hdr.h_next;
	}

	/* ROM table lookup.  Because this lookup is slower, do it only after
	 * RAM lookup.  This works because no ROM string is ever interned into
	 * the RAM string table.
	 */

#if defined(DUK_USE_ROM_STRINGS)
	h = duk__strtab_romstring_lookup(heap, str, blen, strhash);
	if (h != NULL) {
		return h;
	}
#endif

	/* Not found in string table; insert. */

	h = duk__strtable_do_intern(heap, str, blen, strhash);
	return h;  /* may be NULL */
}

/*
 *  Intern a string from u32.
 */

/* XXX: Could arrange some special handling because we know that the result
 * will have an arridx flag and an ASCII flag, won't need a clen check, etc.
 */

DUK_INTERNAL duk_hstring *duk_heap_strtable_intern_u32(duk_heap *heap, duk_uint32_t val) {
	char buf[DUK__STRTAB_U32_MAX_STRLEN];
	char *p;

	DUK_ASSERT(heap != NULL);

	/* This is smaller and faster than a %lu sprintf. */
	p = buf + sizeof(buf);
	do {
		p--;
		*p = duk_lc_digits[val % 10];
		val = val / 10;
	} while (val != 0);  /* For val == 0, emit exactly one '0'. */
	DUK_ASSERT(p >= buf);

	return duk_heap_strtable_intern(heap, (const duk_uint8_t *) p, (duk_uint32_t) ((buf + sizeof(buf)) - p));
}

/*
 *  Checked convenience variants.
 *
 *  XXX: Because the main use case is for the checked variants, make them the
 *  main functionality and provide a safe variant separately (it is only needed
 *  during heap init).  The problem with that is that longjmp state and error
 *  creation must already be possible to throw.
 */

DUK_INTERNAL duk_hstring *duk_heap_strtable_intern_checked(duk_hthread *thr, const duk_uint8_t *str, duk_uint32_t blen) {
	duk_hstring *res;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);
	DUK_ASSERT(blen == 0 || str != NULL);

	res = duk_heap_strtable_intern(thr->heap, str, blen);
	if (DUK_UNLIKELY(res == NULL)) {
		DUK_ERROR_ALLOC_FAILED(thr);
	}
	return res;
}

DUK_INTERNAL duk_hstring *duk_heap_strtable_intern_u32_checked(duk_hthread *thr, duk_uint32_t val) {
	duk_hstring *res;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);

	res = duk_heap_strtable_intern_u32(thr->heap, val);
	if (DUK_UNLIKELY(res == NULL)) {
		DUK_ERROR_ALLOC_FAILED(thr);
	}
	return res;
}

/*
 *  Remove (unlink) a string from the string table.
 *
 *  Just unlinks the duk_hstring, leaving link pointers as garbage.
 *  Caller must free the string itself.
 */

#if defined(DUK_USE_REFERENCE_COUNTING)
/* Unlink without a 'prev' pointer. */
DUK_INTERNAL void duk_heap_strtable_unlink(duk_heap *heap, duk_hstring *h) {
#if defined(DUK_USE_STRTAB_PTRCOMP)
	duk_uint16_t *slot;
#else
	duk_hstring **slot;
#endif
	duk_hstring *other;
	duk_hstring *prev;

	DUK_DDD(DUK_DDDPRINT("remove: heap=%p, h=%p, blen=%lu, strhash=%lx",
	                     (void *) heap, (void *) h,
	                     (unsigned long) (h != NULL ? DUK_HSTRING_GET_BYTELEN(h) : 0),
	                     (unsigned long) (h != NULL ? DUK_HSTRING_GET_HASH(h) : 0)));

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(h != NULL);

#if defined(DUK__STRTAB_RESIZE_CHECK)
	DUK_ASSERT(heap->st_count > 0);
	heap->st_count--;
#endif

#if defined(DUK_USE_STRTAB_PTRCOMP)
	slot = heap->strtable16 + (DUK_HSTRING_GET_HASH(h) & heap->st_mask);
#else
	slot = heap->strtable + (DUK_HSTRING_GET_HASH(h) & heap->st_mask);
#endif
	other = DUK__HEAPPTR_DEC16(heap, *slot);
	DUK_ASSERT(other != NULL);  /* At least argument string is in the chain. */

	prev = NULL;
	while (other != h) {
		prev = other;
		other = other->hdr.h_next;
		DUK_ASSERT(other != NULL);  /* We'll eventually find 'h'. */
	}
	if (prev != NULL) {
		/* Middle of list. */
		prev->hdr.h_next = h->hdr.h_next;
	} else {
		/* Head of list. */
		*slot = DUK__HEAPPTR_ENC16(heap, h->hdr.h_next);
	}

	/* There's no resize check on a string free.  The next string
	 * intern will do one.
	 */
}
#endif  /* DUK_USE_REFERENCE_COUNTING */

/* Unlink with a 'prev' pointer. */
DUK_INTERNAL void duk_heap_strtable_unlink_prev(duk_heap *heap, duk_hstring *h, duk_hstring *prev) {
#if defined(DUK_USE_STRTAB_PTRCOMP)
	duk_uint16_t *slot;
#else
	duk_hstring **slot;
#endif

	DUK_DDD(DUK_DDDPRINT("remove: heap=%p, prev=%p, h=%p, blen=%lu, strhash=%lx",
	                     (void *) heap, (void *) prev, (void *) h,
	                     (unsigned long) (h != NULL ? DUK_HSTRING_GET_BYTELEN(h) : 0),
	                     (unsigned long) (h != NULL ? DUK_HSTRING_GET_HASH(h) : 0)));

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(prev == NULL || prev->hdr.h_next == h);

#if defined(DUK__STRTAB_RESIZE_CHECK)
	DUK_ASSERT(heap->st_count > 0);
	heap->st_count--;
#endif

	if (prev != NULL) {
		/* Middle of list. */
		prev->hdr.h_next = h->hdr.h_next;
	} else {
		/* Head of list. */
#if defined(DUK_USE_STRTAB_PTRCOMP)
		slot = heap->strtable16 + (DUK_HSTRING_GET_HASH(h) & heap->st_mask);
#else
		slot = heap->strtable + (DUK_HSTRING_GET_HASH(h) & heap->st_mask);
#endif
		DUK_ASSERT(DUK__HEAPPTR_DEC16(heap, *slot) == h);
		*slot = DUK__HEAPPTR_ENC16(heap, h->hdr.h_next);
	}
}

/*
 *  Force string table resize check in mark-and-sweep.
 */

DUK_INTERNAL void duk_heap_strtable_force_resize(duk_heap *heap) {
	/* Does only one grow/shrink step if needed.  The heap->st_resizing
	 * flag protects against recursive resizing.
	 */

	DUK_ASSERT(heap != NULL);
	DUK_UNREF(heap);

#if defined(DUK__STRTAB_RESIZE_CHECK)
#if defined(DUK_USE_STRTAB_PTRCOMP)
	if (heap->strtable16 != NULL) {
#else
	if (heap->strtable != NULL) {
#endif
		duk__strtable_resize_check(heap);
	}
#endif
}

/*
 *  Free strings in the string table and the string table itself.
 */

DUK_INTERNAL void duk_heap_strtable_free(duk_heap *heap) {
#if defined(DUK_USE_STRTAB_PTRCOMP)
	duk_uint16_t *strtable;
	duk_uint16_t *st;
#else
	duk_hstring **strtable;
	duk_hstring **st;
#endif
	duk_hstring *h;

	DUK_ASSERT(heap != NULL);

#if defined(DUK_USE_ASSERTIONS)
	duk__strtable_assert_checks(heap);
#endif

	/* Strtable can be NULL if heap init fails.  However, in that case
	 * heap->st_size is 0, so strtable == strtable_end and we skip the
	 * loop without a special check.
	 */
	strtable = DUK__GET_STRTABLE(heap);
	st = strtable + heap->st_size;
	DUK_ASSERT(strtable != NULL || heap->st_size == 0);

	while (strtable != st) {
		--st;
		h = DUK__HEAPPTR_DEC16(heap, *st);
		while (h) {
			duk_hstring *h_next;
			h_next = h->hdr.h_next;

			/* Strings may have inner refs (extdata) in some cases. */
			duk_free_hstring(heap, h);

			h = h_next;
		}
	}

	DUK_FREE(heap, strtable);
}
