/*
 *  Heap stringtable handling, string interning.
 */

#include "duk_internal.h"

#if defined(DUK_USE_STRTAB_PROBE)
#define DUK__HASH_INITIAL(hash,h_size)        DUK_STRTAB_HASH_INITIAL((hash),(h_size))
#define DUK__HASH_PROBE_STEP(hash)            DUK_STRTAB_HASH_PROBE_STEP((hash))
#define DUK__DELETED_MARKER(heap)             DUK_STRTAB_DELETED_MARKER((heap))
#endif

/*
 *  Create a hstring and insert into the heap.  The created object
 *  is directly garbage collectable with reference count zero.
 *
 *  The caller must place the interned string into the stringtable
 *  immediately (without chance of a longjmp); otherwise the string
 *  is lost.
 */

DUK_LOCAL
duk_hstring *duk__alloc_init_hstring(duk_heap *heap,
                                     const duk_uint8_t *str,
                                     duk_uint32_t blen,
                                     duk_uint32_t strhash,
                                     const duk_uint8_t *extdata) {
	duk_hstring *res = NULL;
	duk_uint8_t *data;
	duk_size_t alloc_size;
	duk_uarridx_t dummy;
	duk_uint32_t clen;

#if defined(DUK_USE_STRLEN16)
	/* If blen <= 0xffffUL, clen is also guaranteed to be <= 0xffffUL. */
	if (blen > 0xffffUL) {
		DUK_D(DUK_DPRINT("16-bit string blen/clen active and blen over 16 bits, reject intern"));
		return NULL;
	}
#endif

	if (extdata) {
		alloc_size = (duk_size_t) sizeof(duk_hstring_external);
		res = (duk_hstring *) DUK_ALLOC(heap, alloc_size);
		if (!res) {
			goto alloc_error;
		}
		DUK_MEMZERO(res, sizeof(duk_hstring_external));
#ifdef DUK_USE_EXPLICIT_NULL_INIT
		DUK_HEAPHDR_STRING_INIT_NULLS(&res->hdr);
#endif
		DUK_HEAPHDR_SET_TYPE_AND_FLAGS(&res->hdr, DUK_HTYPE_STRING, DUK_HSTRING_FLAG_EXTDATA);

		((duk_hstring_external *) res)->extdata = extdata;
	} else {
		/* NUL terminate for convenient C access */
		alloc_size = (duk_size_t) (sizeof(duk_hstring) + blen + 1);
		res = (duk_hstring *) DUK_ALLOC(heap, alloc_size);
		if (!res) {
			goto alloc_error;
		}
		DUK_MEMZERO(res, sizeof(duk_hstring));
#ifdef DUK_USE_EXPLICIT_NULL_INIT
		DUK_HEAPHDR_STRING_INIT_NULLS(&res->hdr);
#endif
		DUK_HEAPHDR_SET_TYPE_AND_FLAGS(&res->hdr, DUK_HTYPE_STRING, 0);

		data = (duk_uint8_t *) (res + 1);
		DUK_MEMCPY(data, str, blen);
		data[blen] = (duk_uint8_t) 0;
	}

	if (duk_js_to_arrayindex_raw_string(str, blen, &dummy)) {
		DUK_HSTRING_SET_ARRIDX(res);
	}

	/* All strings beginning with 0xff are treated as "internal",
	 * even strings interned by the user.  This allows user code to
	 * create internal properties too, and makes behavior consistent
	 * in case user code happens to use a string also used by Duktape
	 * (such as string has already been interned and has the 'internal'
	 * flag set).
	 */
	if (blen > 0 && str[0] == (duk_uint8_t) 0xff) {
		DUK_HSTRING_SET_INTERNAL(res);
	}

	DUK_HSTRING_SET_HASH(res, strhash);
	DUK_HSTRING_SET_BYTELEN(res, blen);
	clen = (duk_uint32_t) duk_unicode_unvalidated_utf8_length(str, (duk_size_t) blen);
	DUK_ASSERT(clen <= blen);
	DUK_HSTRING_SET_CHARLEN(res, clen);

	DUK_DDD(DUK_DDDPRINT("interned string, hash=0x%08lx, blen=%ld, clen=%ld, has_arridx=%ld, has_extdata=%ld",
	                     (unsigned long) DUK_HSTRING_GET_HASH(res),
	                     (long) DUK_HSTRING_GET_BYTELEN(res),
	                     (long) DUK_HSTRING_GET_CHARLEN(res),
	                     (long) DUK_HSTRING_HAS_ARRIDX(res) ? 1 : 0,
	                     (long) DUK_HSTRING_HAS_EXTDATA(res) ? 1 : 0));

	return res;

 alloc_error:
	DUK_FREE(heap, res);
	return NULL;
}

/*
 *  String table algorithm: fixed size string table with array chaining
 *
 *  The top level string table has a fixed size, with each slot holding
 *  either NULL, string pointer, or pointer to a separately allocated
 *  string pointer list.
 *
 *  This is good for low memory environments using a pool allocator: the
 *  top level allocation has a fixed size and the pointer lists have quite
 *  small allocation size, which further matches the typical pool sizes
 *  needed by objects, strings, property tables, etc.
 */

#if defined(DUK_USE_STRTAB_CHAIN)

#if defined(DUK_USE_HEAPPTR16)
DUK_LOCAL duk_bool_t duk__insert_hstring_chain(duk_heap *heap, duk_hstring *h) {
	duk_small_uint_t slotidx;
	duk_strtab_entry *e;
	duk_uint16_t *lst;
	duk_uint16_t *new_lst;
	duk_size_t i, n;
	duk_uint16_t null16 = heap->heapptr_null16;
	duk_uint16_t h16 = DUK_USE_HEAPPTR_ENC16(heap->heap_udata, (void *) h);

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(h != NULL);

	slotidx = DUK_HSTRING_GET_HASH(h) % DUK_STRTAB_CHAIN_SIZE;
	DUK_ASSERT(slotidx < DUK_STRTAB_CHAIN_SIZE);

	e = heap->strtable + slotidx;
	if (e->listlen == 0) {
		if (e->u.str16 == null16) {
			e->u.str16 = h16;
		} else {
			/* Now two entries in the same slot, alloc list */
			lst = (duk_uint16_t *) DUK_ALLOC(heap, sizeof(duk_uint16_t) * 2);
			if (lst == NULL) {
				return 1;  /* fail */
			}
			lst[0] = e->u.str16;
			lst[1] = h16;
			e->u.strlist16 = DUK_USE_HEAPPTR_ENC16(heap->heap_udata, (void *) lst);
			e->listlen = 2;
		}
	} else {
		DUK_ASSERT(e->u.strlist16 != null16);
		lst = (duk_uint16_t *) DUK_USE_HEAPPTR_DEC16(heap->heap_udata, e->u.strlist16);
		DUK_ASSERT(lst != NULL);
		for (i = 0, n = e->listlen; i < n; i++) {
			if (lst[i] == null16) {
				lst[i] = h16;
				return 0;
			}
		}

		if (e->listlen + 1 == 0) {
			/* Overflow, relevant mainly when listlen is 16 bits. */
			return 1;  /* fail */
		}

		new_lst = (duk_uint16_t *) DUK_REALLOC(heap, lst, sizeof(duk_uint16_t) * (e->listlen + 1));
		if (new_lst == NULL) {
			return 1;  /* fail */
		}
		new_lst[e->listlen++] = h16;
		e->u.strlist16 = DUK_USE_HEAPPTR_ENC16(heap->heap_udata, (void *) new_lst);
	}
	return 0;
}
#else  /* DUK_USE_HEAPPTR16 */
DUK_LOCAL duk_bool_t duk__insert_hstring_chain(duk_heap *heap, duk_hstring *h) {
	duk_small_uint_t slotidx;
	duk_strtab_entry *e;
	duk_hstring **lst;
	duk_hstring **new_lst;
	duk_size_t i, n;

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(h != NULL);

	slotidx = DUK_HSTRING_GET_HASH(h) % DUK_STRTAB_CHAIN_SIZE;
	DUK_ASSERT(slotidx < DUK_STRTAB_CHAIN_SIZE);

	e = heap->strtable + slotidx;
	if (e->listlen == 0) {
		if (e->u.str == NULL) {
			e->u.str = h;
		} else {
			/* Now two entries in the same slot, alloc list */
			lst = (duk_hstring **) DUK_ALLOC(heap, sizeof(duk_hstring *) * 2);
			if (lst == NULL) {
				return 1;  /* fail */
			}
			lst[0] = e->u.str;
			lst[1] = h;
			e->u.strlist = lst;
			e->listlen = 2;
		}
	} else {
		DUK_ASSERT(e->u.strlist != NULL);
		lst = e->u.strlist;
		for (i = 0, n = e->listlen; i < n; i++) {
			if (lst[i] == NULL) {
				lst[i] = h;
				return 0;
			}
		}

		if (e->listlen + 1 == 0) {
			/* Overflow, relevant mainly when listlen is 16 bits. */
			return 1;  /* fail */
		}

		new_lst = (duk_hstring **) DUK_REALLOC(heap, e->u.strlist, sizeof(duk_hstring *) * (e->listlen + 1));
		if (new_lst == NULL) {
			return 1;  /* fail */
		}
		new_lst[e->listlen++] = h;
		e->u.strlist = new_lst;
	}
	return 0;
}
#endif  /* DUK_USE_HEAPPTR16 */

#if defined(DUK_USE_HEAPPTR16)
DUK_LOCAL duk_hstring *duk__find_matching_string_chain(duk_heap *heap, const duk_uint8_t *str, duk_uint32_t blen, duk_uint32_t strhash) {
	duk_small_uint_t slotidx;
	duk_strtab_entry *e;
	duk_uint16_t *lst;
	duk_size_t i, n;
	duk_uint16_t null16 = heap->heapptr_null16;

	DUK_ASSERT(heap != NULL);

	slotidx = strhash % DUK_STRTAB_CHAIN_SIZE;
	DUK_ASSERT(slotidx < DUK_STRTAB_CHAIN_SIZE);

	e = heap->strtable + slotidx;
	if (e->listlen == 0) {
		if (e->u.str16 != null16) {
			duk_hstring *h = (duk_hstring *) DUK_USE_HEAPPTR_DEC16(heap->heap_udata, e->u.str16);
			DUK_ASSERT(h != NULL);
			if (DUK_HSTRING_GET_BYTELEN(h) == blen &&
			    DUK_MEMCMP(str, DUK_HSTRING_GET_DATA(h), blen) == 0) {
				return h;
			}
		}
	} else {
		DUK_ASSERT(e->u.strlist16 != null16);
		lst = (duk_uint16_t *) DUK_USE_HEAPPTR_DEC16(heap->heap_udata, e->u.strlist16);
		DUK_ASSERT(lst != NULL);
		for (i = 0, n = e->listlen; i < n; i++) {
			if (lst[i] != null16) {
				duk_hstring *h = (duk_hstring *) DUK_USE_HEAPPTR_DEC16(heap->heap_udata, lst[i]);
				DUK_ASSERT(h != NULL);
				if (DUK_HSTRING_GET_BYTELEN(h) == blen &&
				    DUK_MEMCMP(str, DUK_HSTRING_GET_DATA(h), blen) == 0) {
					return h;
				}
			}
		}
	}

	return NULL;
}
#else  /* DUK_USE_HEAPPTR16 */
DUK_LOCAL duk_hstring *duk__find_matching_string_chain(duk_heap *heap, const duk_uint8_t *str, duk_uint32_t blen, duk_uint32_t strhash) {
	duk_small_uint_t slotidx;
	duk_strtab_entry *e;
	duk_hstring **lst;
	duk_size_t i, n;

	DUK_ASSERT(heap != NULL);

	slotidx = strhash % DUK_STRTAB_CHAIN_SIZE;
	DUK_ASSERT(slotidx < DUK_STRTAB_CHAIN_SIZE);

	e = heap->strtable + slotidx;
	if (e->listlen == 0) {
		if (e->u.str != NULL &&
	           DUK_HSTRING_GET_BYTELEN(e->u.str) == blen &&
	           DUK_MEMCMP(str, DUK_HSTRING_GET_DATA(e->u.str), blen) == 0) {
			return e->u.str;
		}
	} else {
		DUK_ASSERT(e->u.strlist != NULL);
		lst = e->u.strlist;
		for (i = 0, n = e->listlen; i < n; i++) {
			if (lst[i] != NULL &&
		           DUK_HSTRING_GET_BYTELEN(lst[i]) == blen &&
		           DUK_MEMCMP(str, DUK_HSTRING_GET_DATA(lst[i]), blen) == 0) {
				return lst[i];
			}
		}
	}

	return NULL;
}
#endif  /* DUK_USE_HEAPPTR16 */

#if defined(DUK_USE_HEAPPTR16)
DUK_LOCAL void duk__remove_matching_hstring_chain(duk_heap *heap, duk_hstring *h) {
	duk_small_uint_t slotidx;
	duk_strtab_entry *e;
	duk_uint16_t *lst;
	duk_size_t i, n;
	duk_uint16_t h16;
	duk_uint16_t null16 = heap->heapptr_null16;

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(h != NULL);

	slotidx = DUK_HSTRING_GET_HASH(h) % DUK_STRTAB_CHAIN_SIZE;
	DUK_ASSERT(slotidx < DUK_STRTAB_CHAIN_SIZE);

	DUK_ASSERT(h != NULL);
	h16 = DUK_USE_HEAPPTR_ENC16(heap->heap_udata, (void *) h);

	e = heap->strtable + slotidx;
	if (e->listlen == 0) {
		if (e->u.str16 == h16) {
			e->u.str16 = null16;
			return;
		}
	} else {
		DUK_ASSERT(e->u.strlist16 != null16);
		lst = (duk_uint16_t *) DUK_USE_HEAPPTR_DEC16(heap->heap_udata, e->u.strlist16);
		DUK_ASSERT(lst != NULL);
		for (i = 0, n = e->listlen; i < n; i++) {
			if (lst[i] == h16) {
				lst[i] = null16;
				return;
			}
		}
	}

	DUK_D(DUK_DPRINT("failed to find string that should be in stringtable"));
	DUK_UNREACHABLE();
	return;
}
#else  /* DUK_USE_HEAPPTR16 */
DUK_LOCAL void duk__remove_matching_hstring_chain(duk_heap *heap, duk_hstring *h) {
	duk_small_uint_t slotidx;
	duk_strtab_entry *e;
	duk_hstring **lst;
	duk_size_t i, n;

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(h != NULL);

	slotidx = DUK_HSTRING_GET_HASH(h) % DUK_STRTAB_CHAIN_SIZE;
	DUK_ASSERT(slotidx < DUK_STRTAB_CHAIN_SIZE);

	e = heap->strtable + slotidx;
	if (e->listlen == 0) {
		DUK_ASSERT(h != NULL);
		if (e->u.str == h) {
			e->u.str = NULL;
			return;
		}
	} else {
		DUK_ASSERT(e->u.strlist != NULL);
		lst = e->u.strlist;
		for (i = 0, n = e->listlen; i < n; i++) {
			DUK_ASSERT(h != NULL);
			if (lst[i] == h) {
				lst[i] = NULL;
				return;
			}
		}
	}

	DUK_D(DUK_DPRINT("failed to find string that should be in stringtable"));
	DUK_UNREACHABLE();
	return;
}
#endif  /* DUK_USE_HEAPPTR16 */

#if defined(DUK_USE_DEBUG)
DUK_INTERNAL void duk_heap_dump_strtab(duk_heap *heap) {
	duk_strtab_entry *e;
	duk_small_uint_t i;
	duk_size_t j, n, used;
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t *lst;
	duk_uint16_t null16 = heap->heapptr_null16;
#else
	duk_hstring **lst;
#endif

	DUK_ASSERT(heap != NULL);

	for (i = 0; i < DUK_STRTAB_CHAIN_SIZE; i++) {
		e = heap->strtable + i;

		if (e->listlen == 0) {
#if defined(DUK_USE_HEAPPTR16)
			DUK_DD(DUK_DDPRINT("[%03d] -> plain %d", (int) i, (int) (e->u.str16 != null16 ? 1 : 0)));
#else
			DUK_DD(DUK_DDPRINT("[%03d] -> plain %d", (int) i, (int) (e->u.str ? 1 : 0)));
#endif
		} else {
			used = 0;
#if defined(DUK_USE_HEAPPTR16)
			lst = (duk_uint16_t *) DUK_USE_HEAPPTR_DEC16(heap->heap_udata, e->u.strlist16);
#else
			lst = e->u.strlist;
#endif
			DUK_ASSERT(lst != NULL);
			for (j = 0, n = e->listlen; j < n; j++) {
#if defined(DUK_USE_HEAPPTR16)
				if (lst[j] != null16) {
#else
				if (lst[j] != NULL) {
#endif
					used++;
				}
			}
			DUK_DD(DUK_DDPRINT("[%03d] -> array %d/%d", (int) i, (int) used, (int) e->listlen));
		}
	}
}
#endif  /* DUK_USE_DEBUG */

#endif  /* DUK_USE_STRTAB_CHAIN */

/*
 *  String table algorithm: closed hashing with a probe sequence
 *
 *  This is the default algorithm and works fine for environments with
 *  minimal memory constraints.
 */

#if defined(DUK_USE_STRTAB_PROBE)

/* Count actually used (non-NULL, non-DELETED) entries. */
DUK_LOCAL duk_int_t duk__count_used_probe(duk_heap *heap) {
	duk_int_t res = 0;
	duk_uint_fast32_t i, n;
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t null16 = heap->heapptr_null16;
	duk_uint16_t deleted16 = heap->heapptr_deleted16;
#endif

	n = (duk_uint_fast32_t) heap->st_size;
	for (i = 0; i < n; i++) {
#if defined(DUK_USE_HEAPPTR16)
		if (heap->strtable16[i] != null16 && heap->strtable16[i] != deleted16) {
#else
		if (heap->strtable[i] != NULL && heap->strtable[i] != DUK__DELETED_MARKER(heap)) {
#endif
			res++;
		}
	}
	return res;
}

#if defined(DUK_USE_HEAPPTR16)
DUK_LOCAL void duk__insert_hstring_probe(duk_heap *heap, duk_uint16_t *entries16, duk_uint32_t size, duk_uint32_t *p_used, duk_hstring *h) {
#else
DUK_LOCAL void duk__insert_hstring_probe(duk_heap *heap, duk_hstring **entries, duk_uint32_t size, duk_uint32_t *p_used, duk_hstring *h) {
#endif
	duk_uint32_t i;
	duk_uint32_t step;
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t null16 = heap->heapptr_null16;
	duk_uint16_t deleted16 = heap->heapptr_deleted16;
#endif

	DUK_ASSERT(size > 0);

	i = DUK__HASH_INITIAL(DUK_HSTRING_GET_HASH(h), size);
	step = DUK__HASH_PROBE_STEP(DUK_HSTRING_GET_HASH(h));
	for (;;) {
#if defined(DUK_USE_HEAPPTR16)
		duk_uint16_t e16 = entries16[i];
#else
		duk_hstring *e = entries[i];
#endif

#if defined(DUK_USE_HEAPPTR16)
		/* XXX: could check for e16 == 0 because NULL is guaranteed to
		 * encode to zero.
		 */
		if (e16 == null16) {
#else
		if (e == NULL) {
#endif
			DUK_DDD(DUK_DDDPRINT("insert hit (null): %ld", (long) i));
#if defined(DUK_USE_HEAPPTR16)
			entries16[i] = DUK_USE_HEAPPTR_ENC16(heap->heap_udata, (void *) h);
#else
			entries[i] = h;
#endif
			(*p_used)++;
			break;
#if defined(DUK_USE_HEAPPTR16)
		} else if (e16 == deleted16) {
#else
		} else if (e == DUK__DELETED_MARKER(heap)) {
#endif
			/* st_used remains the same, DELETED is counted as used */
			DUK_DDD(DUK_DDDPRINT("insert hit (deleted): %ld", (long) i));
#if defined(DUK_USE_HEAPPTR16)
			entries16[i] = DUK_USE_HEAPPTR_ENC16(heap->heap_udata, (void *) h);
#else
			entries[i] = h;
#endif
			break;
		}
		DUK_DDD(DUK_DDDPRINT("insert miss: %ld", (long) i));
		i = (i + step) % size;

		/* looping should never happen */
		DUK_ASSERT(i != DUK__HASH_INITIAL(DUK_HSTRING_GET_HASH(h), size));
	}
}

#if defined(DUK_USE_HEAPPTR16)
DUK_LOCAL duk_hstring *duk__find_matching_string_probe(duk_heap *heap, duk_uint16_t *entries16, duk_uint32_t size, const duk_uint8_t *str, duk_uint32_t blen, duk_uint32_t strhash) {
#else
DUK_LOCAL duk_hstring *duk__find_matching_string_probe(duk_heap *heap, duk_hstring **entries, duk_uint32_t size, const duk_uint8_t *str, duk_uint32_t blen, duk_uint32_t strhash) {
#endif
	duk_uint32_t i;
	duk_uint32_t step;

	DUK_ASSERT(size > 0);

	i = DUK__HASH_INITIAL(strhash, size);
	step = DUK__HASH_PROBE_STEP(strhash);
	for (;;) {
		duk_hstring *e;
#if defined(DUK_USE_HEAPPTR16)
		e = (duk_hstring *) DUK_USE_HEAPPTR_DEC16(heap->heap_udata, entries16[i]);
#else
		e = entries[i];
#endif

		if (!e) {
			return NULL;
		}
		if (e != DUK__DELETED_MARKER(heap) && DUK_HSTRING_GET_BYTELEN(e) == blen) {
			if (DUK_MEMCMP(str, DUK_HSTRING_GET_DATA(e), blen) == 0) {
				DUK_DDD(DUK_DDDPRINT("find matching hit: %ld (step %ld, size %ld)",
				                     (long) i, (long) step, (long) size));
				return e;
			}
		}
		DUK_DDD(DUK_DDDPRINT("find matching miss: %ld (step %ld, size %ld)",
		                     (long) i, (long) step, (long) size));
		i = (i + step) % size;

		/* looping should never happen */
		DUK_ASSERT(i != DUK__HASH_INITIAL(strhash, size));
	}
	DUK_UNREACHABLE();
}

#if defined(DUK_USE_HEAPPTR16)
DUK_LOCAL void duk__remove_matching_hstring_probe(duk_heap *heap, duk_uint16_t *entries16, duk_uint32_t size, duk_hstring *h) {
#else
DUK_LOCAL void duk__remove_matching_hstring_probe(duk_heap *heap, duk_hstring **entries, duk_uint32_t size, duk_hstring *h) {
#endif
	duk_uint32_t i;
	duk_uint32_t step;
	duk_uint32_t hash;
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t null16 = heap->heapptr_null16;
	duk_uint16_t h16 = DUK_USE_HEAPPTR_ENC16(heap->heap_udata, (void *) h);
#endif

	DUK_ASSERT(size > 0);

	hash = DUK_HSTRING_GET_HASH(h);
	i = DUK__HASH_INITIAL(hash, size);
	step = DUK__HASH_PROBE_STEP(hash);
	for (;;) {
#if defined(DUK_USE_HEAPPTR16)
		duk_uint16_t e16 = entries16[i];
#else
		duk_hstring *e = entries[i];
#endif

#if defined(DUK_USE_HEAPPTR16)
		if (e16 == null16) {
#else
		if (!e) {
#endif
			DUK_UNREACHABLE();
			break;
		}
#if defined(DUK_USE_HEAPPTR16)
		if (e16 == h16) {
#else
		if (e == h) {
#endif
			/* st_used remains the same, DELETED is counted as used */
			DUK_DDD(DUK_DDDPRINT("free matching hit: %ld", (long) i));
#if defined(DUK_USE_HEAPPTR16)
			entries16[i] = heap->heapptr_deleted16;
#else
			entries[i] = DUK__DELETED_MARKER(heap);
#endif
			break;
		}

		DUK_DDD(DUK_DDDPRINT("free matching miss: %ld", (long) i));
		i = (i + step) % size;

		/* looping should never happen */
		DUK_ASSERT(i != DUK__HASH_INITIAL(hash, size));
	}
}

DUK_LOCAL duk_bool_t duk__resize_strtab_raw_probe(duk_heap *heap, duk_uint32_t new_size) {
#ifdef DUK_USE_MARK_AND_SWEEP
	duk_small_uint_t prev_mark_and_sweep_base_flags;
#endif
#ifdef DUK_USE_DEBUG
	duk_uint32_t old_used = heap->st_used;
#endif
	duk_uint32_t old_size = heap->st_size;
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t *old_entries = heap->strtable16;
	duk_uint16_t *new_entries = NULL;
#else
	duk_hstring **old_entries = heap->strtable;
	duk_hstring **new_entries = NULL;
#endif
	duk_uint32_t new_used = 0;
	duk_uint32_t i;

#ifdef DUK_USE_DEBUG
	DUK_UNREF(old_used);  /* unused with some debug level combinations */
#endif

#ifdef DUK_USE_DDDPRINT
	DUK_DDD(DUK_DDDPRINT("attempt to resize stringtable: %ld entries, %ld bytes, %ld used, %ld%% load -> %ld entries, %ld bytes, %ld used, %ld%% load",
	                     (long) old_size, (long) (sizeof(duk_hstring *) * old_size), (long) old_used,
	                     (long) (((double) old_used) / ((double) old_size) * 100.0),
	                     (long) new_size, (long) (sizeof(duk_hstring *) * new_size), (long) duk__count_used_probe(heap),
	                     (long) (((double) duk__count_used_probe(heap)) / ((double) new_size) * 100.0)));
#endif

	DUK_ASSERT(new_size > (duk_uint32_t) duk__count_used_probe(heap));  /* required for rehash to succeed, equality not that useful */
	DUK_ASSERT(old_entries);
#ifdef DUK_USE_MARK_AND_SWEEP
	DUK_ASSERT((heap->mark_and_sweep_base_flags & DUK_MS_FLAG_NO_STRINGTABLE_RESIZE) == 0);
#endif

	/*
	 *  The attempt to allocate may cause a GC.  Such a GC must not attempt to resize
	 *  the stringtable (though it can be swept); finalizer execution and object
	 *  compaction must also be postponed to avoid the pressure to add strings to the
	 *  string table.
	 */

#ifdef DUK_USE_MARK_AND_SWEEP
	prev_mark_and_sweep_base_flags = heap->mark_and_sweep_base_flags;
	heap->mark_and_sweep_base_flags |= \
	        DUK_MS_FLAG_NO_STRINGTABLE_RESIZE |  /* avoid recursive call here */
	        DUK_MS_FLAG_NO_FINALIZERS |          /* avoid pressure to add/remove strings */
	        DUK_MS_FLAG_NO_OBJECT_COMPACTION;    /* avoid array abandoning which interns strings */
#endif

#if defined(DUK_USE_HEAPPTR16)
	new_entries = (duk_uint16_t *) DUK_ALLOC(heap, sizeof(duk_uint16_t) * new_size);
#else
	new_entries = (duk_hstring **) DUK_ALLOC(heap, sizeof(duk_hstring *) * new_size);
#endif

#ifdef DUK_USE_MARK_AND_SWEEP
	heap->mark_and_sweep_base_flags = prev_mark_and_sweep_base_flags;
#endif

	if (!new_entries) {
		goto resize_error;
	}

#ifdef DUK_USE_EXPLICIT_NULL_INIT
	for (i = 0; i < new_size; i++) {
#if defined(DUK_USE_HEAPPTR16)
		new_entries[i] = heap->heapptr_null16;
#else
		new_entries[i] = NULL;
#endif
	}
#else
#if defined(DUK_USE_HEAPPTR16)
	/* Relies on NULL encoding to zero. */
	DUK_MEMZERO(new_entries, sizeof(duk_uint16_t) * new_size);
#else
	DUK_MEMZERO(new_entries, sizeof(duk_hstring *) * new_size);
#endif
#endif

	/* Because new_size > duk__count_used_probe(heap), guaranteed to work */
	for (i = 0; i < old_size; i++) {
		duk_hstring *e;

#if defined(DUK_USE_HEAPPTR16)
		e = (duk_hstring *) DUK_USE_HEAPPTR_DEC16(heap->heap_udata, old_entries[i]);
#else
		e = old_entries[i];
#endif
		if (e == NULL || e == DUK__DELETED_MARKER(heap)) {
			continue;
		}
		/* checking for DUK__DELETED_MARKER is not necessary here, but helper does it now */
		duk__insert_hstring_probe(heap, new_entries, new_size, &new_used, e);
	}

#ifdef DUK_USE_DDPRINT
	DUK_DD(DUK_DDPRINT("resized stringtable: %ld entries, %ld bytes, %ld used, %ld%% load -> %ld entries, %ld bytes, %ld used, %ld%% load",
	                   (long) old_size, (long) (sizeof(duk_hstring *) * old_size), (long) old_used,
	                   (long) (((double) old_used) / ((double) old_size) * 100.0),
	                   (long) new_size, (long) (sizeof(duk_hstring *) * new_size), (long) new_used,
	                   (long) (((double) new_used) / ((double) new_size) * 100.0)));
#endif

#if defined(DUK_USE_HEAPPTR16)
	DUK_FREE(heap, heap->strtable16);
	heap->strtable16 = new_entries;
#else
	DUK_FREE(heap, heap->strtable);
	heap->strtable = new_entries;
#endif
	heap->st_size = new_size;
	heap->st_used = new_used;  /* may be less, since DELETED entries are NULLed by rehash */

	return 0;  /* OK */

 resize_error:
	DUK_FREE(heap, new_entries);
	return 1;  /* FAIL */
}

DUK_LOCAL duk_bool_t duk__resize_strtab_probe(duk_heap *heap) {
	duk_uint32_t new_size;
	duk_bool_t ret;

	new_size = (duk_uint32_t) duk__count_used_probe(heap);
	if (new_size >= 0x80000000UL) {
		new_size = DUK_STRTAB_HIGHEST_32BIT_PRIME;
	} else {
		new_size = duk_util_get_hash_prime(DUK_STRTAB_GROW_ST_SIZE(new_size));
		new_size = duk_util_get_hash_prime(new_size);
	}
	DUK_ASSERT(new_size > 0);

	/* rehash even if old and new sizes are the same to get rid of
	 * DELETED entries.
	*/

	ret = duk__resize_strtab_raw_probe(heap, new_size);

	return ret;
}

DUK_LOCAL duk_bool_t duk__recheck_strtab_size_probe(duk_heap *heap, duk_uint32_t new_used) {
	duk_uint32_t new_free;
	duk_uint32_t tmp1;
	duk_uint32_t tmp2;

	DUK_ASSERT(new_used <= heap->st_size);  /* grow by at most one */
	new_free = heap->st_size - new_used;    /* unsigned intentionally */

	/* new_free / size <= 1 / DIV  <=>  new_free <= size / DIV */
	/* new_used / size <= 1 / DIV  <=>  new_used <= size / DIV */

	tmp1 = heap->st_size / DUK_STRTAB_MIN_FREE_DIVISOR;
	tmp2 = heap->st_size / DUK_STRTAB_MIN_USED_DIVISOR;

	if (new_free <= tmp1 || new_used <= tmp2) {
		/* load factor too low or high, count actually used entries and resize */
		return duk__resize_strtab_probe(heap);
	} else {
		return 0;  /* OK */
	}
}

#if defined(DUK_USE_DEBUG)
DUK_INTERNAL void duk_heap_dump_strtab(duk_heap *heap) {
	duk_uint32_t i;
	duk_hstring *h;

	DUK_ASSERT(heap != NULL);
#if defined(DUK_USE_HEAPPTR16)
	DUK_ASSERT(heap->strtable16 != NULL);
#else
	DUK_ASSERT(heap->strtable != NULL);
#endif
	DUK_UNREF(h);

	for (i = 0; i < heap->st_size; i++) {
#if defined(DUK_USE_HEAPPTR16)
		h = (duk_hstring *) DUK_USE_HEAPPTR_DEC16(heap->strtable16[i]);
#else
		h = heap->strtable[i];
#endif

		DUK_DD(DUK_DDPRINT("[%03d] -> %p", (int) i, (void *) h));
	}
}
#endif  /* DUK_USE_DEBUG */

#endif  /* DUK_USE_STRTAB_PROBE */

/*
 *  Raw intern and lookup
 */

DUK_LOCAL duk_hstring *duk__do_intern(duk_heap *heap, const duk_uint8_t *str, duk_uint32_t blen, duk_uint32_t strhash) {
	duk_hstring *res;
	const duk_uint8_t *extdata;

#if defined(DUK_USE_STRTAB_PROBE)
	if (duk__recheck_strtab_size_probe(heap, heap->st_used + 1)) {
		return NULL;
	}
#endif

	/* For manual testing only. */
#if 0
	{
		duk_size_t i;
		DUK_PRINTF("INTERN: \"");
		for (i = 0; i < blen; i++) {
			duk_uint8_t x = str[i];
			if (x >= 0x20 && x <= 0x7e && x != '"' && x != '\\') {
				DUK_PRINTF("%c", (int) x);  /* char: use int cast */
			} else {
				DUK_PRINTF("\\x%02lx", (long) x);
			}
		}
		DUK_PRINTF("\"\n");
	}
#endif

#if defined(DUK_USE_HSTRING_EXTDATA) && defined(DUK_USE_EXTSTR_INTERN_CHECK)
	extdata = (const duk_uint8_t *) DUK_USE_EXTSTR_INTERN_CHECK(heap->heap_udata, (void *) str, (duk_size_t) blen);
#else
	extdata = (const duk_uint8_t *) NULL;
#endif
	res = duk__alloc_init_hstring(heap, str, blen, strhash, extdata);
	if (!res) {
		return NULL;
	}

#if defined(DUK_USE_STRTAB_CHAIN)
	if (duk__insert_hstring_chain(heap, res)) {
		/* failed */
		DUK_FREE(heap, res);
		return NULL;
	}
#elif defined(DUK_USE_STRTAB_PROBE)
	/* guaranteed to succeed */
	duk__insert_hstring_probe(heap,
#if defined(DUK_USE_HEAPPTR16)
	                          heap->strtable16,
#else
	                          heap->strtable,
#endif
	                          heap->st_size,
	                          &heap->st_used,
	                          res);
#else
#error internal error, invalid strtab options
#endif

	/* Note: hstring is in heap but has refcount zero and is not strongly reachable.
	 * Caller should increase refcount and make the hstring reachable before any
	 * operations which require allocation (and possible gc).
	 */

	return res;
}

DUK_LOCAL duk_hstring *duk__do_lookup(duk_heap *heap, const duk_uint8_t *str, duk_uint32_t blen, duk_uint32_t *out_strhash) {
	duk_hstring *res;

	DUK_ASSERT(out_strhash);

	*out_strhash = duk_heap_hashstring(heap, str, (duk_size_t) blen);

#if defined(DUK_USE_STRTAB_CHAIN)
	res = duk__find_matching_string_chain(heap, str, blen, *out_strhash);
#elif defined(DUK_USE_STRTAB_PROBE)
	res = duk__find_matching_string_probe(heap,
#if defined(DUK_USE_HEAPPTR16)
	                                      heap->strtable16,
#else
	                                      heap->strtable,
#endif
	                                      heap->st_size,
	                                      str,
	                                      blen,
	                                      *out_strhash);
#else
#error internal error, invalid strtab options
#endif

	return res;
}

/*
 *  Exposed calls
 */

#if 0  /*unused*/
DUK_INTERNAL duk_hstring *duk_heap_string_lookup(duk_heap *heap, const duk_uint8_t *str, duk_uint32_t blen) {
	duk_uint32_t strhash;  /* dummy */
	return duk__do_lookup(heap, str, blen, &strhash);
}
#endif

DUK_INTERNAL duk_hstring *duk_heap_string_intern(duk_heap *heap, const duk_uint8_t *str, duk_uint32_t blen) {
	duk_hstring *res;
	duk_uint32_t strhash;

	/* caller is responsible for ensuring this */
	DUK_ASSERT(blen <= DUK_HSTRING_MAX_BYTELEN);

	res = duk__do_lookup(heap, str, blen, &strhash);
	if (res) {
		return res;
	}

	res = duk__do_intern(heap, str, blen, strhash);
	return res;  /* may be NULL */
}

DUK_INTERNAL duk_hstring *duk_heap_string_intern_checked(duk_hthread *thr, const duk_uint8_t *str, duk_uint32_t blen) {
	duk_hstring *res = duk_heap_string_intern(thr->heap, str, blen);
	if (!res) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, "failed to intern string");
	}
	return res;
}

#if 0  /*unused*/
DUK_INTERNAL duk_hstring *duk_heap_string_lookup_u32(duk_heap *heap, duk_uint32_t val) {
	char buf[DUK_STRTAB_U32_MAX_STRLEN+1];
	DUK_SNPRINTF(buf, sizeof(buf), "%lu", (unsigned long) val);
	buf[sizeof(buf) - 1] = (char) 0;
	DUK_ASSERT(DUK_STRLEN(buf) <= DUK_UINT32_MAX);  /* formatted result limited */
	return duk_heap_string_lookup(heap, (const duk_uint8_t *) buf, (duk_uint32_t) DUK_STRLEN(buf));
}
#endif

DUK_INTERNAL duk_hstring *duk_heap_string_intern_u32(duk_heap *heap, duk_uint32_t val) {
	char buf[DUK_STRTAB_U32_MAX_STRLEN+1];
	DUK_SNPRINTF(buf, sizeof(buf), "%lu", (unsigned long) val);
	buf[sizeof(buf) - 1] = (char) 0;
	DUK_ASSERT(DUK_STRLEN(buf) <= DUK_UINT32_MAX);  /* formatted result limited */
	return duk_heap_string_intern(heap, (const duk_uint8_t *) buf, (duk_uint32_t) DUK_STRLEN(buf));
}

DUK_INTERNAL duk_hstring *duk_heap_string_intern_u32_checked(duk_hthread *thr, duk_uint32_t val) {
	duk_hstring *res = duk_heap_string_intern_u32(thr->heap, val);
	if (!res) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, "failed to intern string");
	}
	return res;
}

/* find and remove string from stringtable; caller must free the string itself */
DUK_INTERNAL void duk_heap_string_remove(duk_heap *heap, duk_hstring *h) {
	DUK_DDD(DUK_DDDPRINT("remove string from stringtable: %!O", (duk_heaphdr *) h));

#if defined(DUK_USE_STRTAB_CHAIN)
	duk__remove_matching_hstring_chain(heap, h);
#elif defined(DUK_USE_STRTAB_PROBE)
	duk__remove_matching_hstring_probe(heap,
#if defined(DUK_USE_HEAPPTR16)
	                                   heap->strtable16,
#else
	                                   heap->strtable,
#endif
	                                   heap->st_size,
	                                   h);
#else
#error internal error, invalid strtab options
#endif
}

#if defined(DUK_USE_MARK_AND_SWEEP) && defined(DUK_USE_MS_STRINGTABLE_RESIZE)
DUK_INTERNAL void duk_heap_force_strtab_resize(duk_heap *heap) {
	/* Force a resize so that DELETED entries are eliminated.
	 * Another option would be duk__recheck_strtab_size_probe();
	 * but since that happens on every intern anyway, this whole
	 * check can now be disabled.
	 */
#if defined(DUK_USE_STRTAB_CHAIN)
	DUK_UNREF(heap);
#elif defined(DUK_USE_STRTAB_PROBE)
	duk__resize_strtab_probe(heap);
#endif
}
#endif

#if defined(DUK_USE_STRTAB_CHAIN)
DUK_INTERNAL void duk_heap_free_strtab(duk_heap *heap) {
	/* Free strings in the stringtable and any allocations needed
	 * by the stringtable itself.
	 */
	duk_uint_fast32_t i, j;
	duk_strtab_entry *e;
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t *lst;
	duk_uint16_t null16 = heap->heapptr_null16;
#else
	duk_hstring **lst;
#endif
	duk_hstring *h;

	for (i = 0; i < DUK_STRTAB_CHAIN_SIZE; i++) {
		e = heap->strtable + i;
		if (e->listlen > 0) {
#if defined(DUK_USE_HEAPPTR16)
			lst = (duk_uint16_t *) DUK_USE_HEAPPTR_DEC16(heap->heap_udata, e->u.strlist16);
#else
			lst = e->u.strlist;
#endif
			DUK_ASSERT(lst != NULL);

			for (j = 0; j < e->listlen; j++) {
#if defined(DUK_USE_HEAPPTR16)
				h = DUK_USE_HEAPPTR_DEC16(heap->heap_udata, lst[j]);
				lst[j] = null16;
#else
				h = lst[j];
				lst[j] = NULL;
#endif
				/* strings may have inner refs (extdata) in some cases */
				if (h != NULL) {
					duk_free_hstring_inner(heap, h);
					DUK_FREE(heap, h);
				}
			}
#if defined(DUK_USE_HEAPPTR16)
			e->u.strlist16 = null16;
#else
			e->u.strlist = NULL;
#endif
			DUK_FREE(heap, lst);
		} else {
#if defined(DUK_USE_HEAPPTR16)
			h = DUK_USE_HEAPPTR_DEC16(heap->heap_udata, e->u.str16);
			e->u.str16 = null16;
#else
			h = e->u.str;
			e->u.str = NULL;
#endif
			if (h != NULL) {
				duk_free_hstring_inner(heap, h);
				DUK_FREE(heap, h);
			}
		}
		e->listlen = 0;
	}
}
#endif  /* DUK_USE_STRTAB_CHAIN */

#if defined(DUK_USE_STRTAB_PROBE)
DUK_INTERNAL void duk_heap_free_strtab(duk_heap *heap) {
	duk_uint_fast32_t i;
	duk_hstring *h;

#if defined(DUK_USE_HEAPPTR16)
	if (heap->strtable16) {
#else
	if (heap->strtable) {
#endif
		for (i = 0; i < (duk_uint_fast32_t) heap->st_size; i++) {
#if defined(DUK_USE_HEAPPTR16)
			h = (duk_hstring *) DUK_USE_HEAPPTR_DEC16(heap->heap_udata, heap->strtable16[i]);
#else
			h = heap->strtable[i];
#endif
			if (h == NULL || h == DUK_STRTAB_DELETED_MARKER(heap)) {
				continue;
			}
			DUK_ASSERT(h != NULL);

			/* strings may have inner refs (extdata) in some cases */
			duk_free_hstring_inner(heap, h);
			DUK_FREE(heap, h);
#if 0  /* not strictly necessary */
			heap->strtable[i] = NULL;
#endif
		}
#if defined(DUK_USE_HEAPPTR16)
		DUK_FREE(heap, heap->strtable16);
#else
		DUK_FREE(heap, heap->strtable);
#endif
#if 0  /* not strictly necessary */
		heap->strtable = NULL;
#endif
	}
}
#endif  /* DUK_USE_STRTAB_PROBE */

/* Undefine local defines */
#undef DUK__HASH_INITIAL
#undef DUK__HASH_PROBE_STEP
#undef DUK__DELETED_MARKER
