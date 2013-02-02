/*
 *  Heap stringtable handling, string interning.
 */

#include "duk_internal.h"

/* FIXME: most of these constants should be in duk_heap.h;
 * similar constants for objects are in duk_hobject.h.
 */

#define  MIN_FREE_DIVISOR           4                /* load factor max 75% */
#define  MIN_USED_DIVISOR           4                /* load factor min 25% */

#define  GROW_ST_SIZE(n)            ((n) + (n))      /* used entries + approx 100% -> reset load to 50% */

#define  U32_MAX_STRLEN             10               /* 4'294'967'295 */

#define  HASH_INITIAL(hash,h_size)  ((hash) % (h_size))
#define  HASH_PROBE_STEP(hash)      DUK_UTIL_GET_HASH_PROBE_STEP((hash))

#define  HIGHEST_32BIT_PRIME        0xfffffffbU

#define  DELETED_MARKER(heap)       DUK_HEAP_STRINGTABLE_DELETED_MARKER((heap))

/*
 *  Create a hstring and insert into the heap.  The created object
 *  is directly garbage collectable with reference count zero.
 *
 *  The caller must place the interned string into the stringtable
 *  immediately (without chance of a longjmp); otherwise the string
 *  is lost.
 */

static duk_hstring *alloc_init_hstring(duk_heap *heap,
                                       duk_u8 *str,
                                       duk_u32 blen,
                                       duk_u32 strhash) {
	duk_hstring *res = NULL;
	duk_u8 *data;
	duk_u32 alloc_size;

	/* NUL terminate for convenient C access */

	alloc_size = sizeof(duk_hstring) + blen + 1;
	res = DUK_ALLOC(heap, alloc_size);
	if (!res) {
		goto error;
	}

	memset(res, 0, sizeof(duk_hstring));
#ifdef DUK_USE_EXPLICIT_NULL_INIT
	DUK_HEAPHDR_STRING_INIT_NULLS(&res->hdr);
#endif
	DUK_HEAPHDR_SET_TYPE_AND_FLAGS(&res->hdr, DUK_HTYPE_STRING, 0);

	if (duk_js_is_arrayindex_raw_string(str, blen)) {
		DUK_HSTRING_SET_ARRIDX(res);
	}

	res->hash = strhash;
	res->blen = blen;
	res->clen = duk_unicode_unvalidated_utf8_length(str, blen);

	data = (duk_u8 *) (res + 1);
	memcpy(data, str, blen);
	data[blen] = (duk_u8) 0;

	DUK_DDDPRINT("interned string, hash=0x%08x, blen=%d, clen=%d, arridx=%d",
	             DUK_HSTRING_GET_HASH(res),
	             DUK_HSTRING_GET_BYTELEN(res),
	             DUK_HSTRING_GET_CHARLEN(res),
	             DUK_HSTRING_HAS_ARRIDX(res) ? 1 : 0);

	return res;

 error:
	DUK_FREE(heap, res);
	return NULL;
}

/*
 *  Count actually used (non-NULL, non-DELETED) entries
 */

static int count_used(duk_heap *heap) {
	int i;
	int res = 0;

	for (i = 0; i < heap->st_size; i++) {
		if (heap->st[i] != NULL && heap->st[i] != DELETED_MARKER(heap)) {
			res++;
		}
	}
	return res;
}

/*
 *  Hashtable lookup and insert helpers
 */

static void insert_hstring(duk_heap *heap, duk_hstring **entries, duk_u32 size, duk_u32 *p_used, duk_hstring *h) {
	duk_u32 i;
	duk_u32 step;

	DUK_ASSERT(size > 0);

	i = HASH_INITIAL(DUK_HSTRING_GET_HASH(h), size);
	step = HASH_PROBE_STEP(DUK_HSTRING_GET_HASH(h)); 
	for (;;) {
		duk_hstring *e;
		
		e = entries[i];
		if (e == NULL) {
			DUK_DDDPRINT("insert hit (null): %d", i);
			entries[i] = h;
			(*p_used)++;
			break;
		} else if (e == DELETED_MARKER(heap)) {
			/* st_used remains the same, DELETED is counted as used */
			DUK_DDDPRINT("insert hit (deleted): %d", i);
			entries[i] = h;
			break;
		}
		DUK_DDDPRINT("insert miss: %d", i);
		i = (i + step) % size;

		/* looping should never happen */
		DUK_ASSERT(i != HASH_INITIAL(DUK_HSTRING_GET_HASH(h), size));
	}
}

static duk_hstring *find_matching_string(duk_heap *heap, duk_hstring **entries, duk_u32 size, duk_u8 *str, duk_u32 blen, duk_u32 strhash) {
	duk_u32 i;
	duk_u32 step;

	DUK_ASSERT(size > 0);

	i = HASH_INITIAL(strhash, size);
	step = HASH_PROBE_STEP(strhash);
	for (;;) {
		duk_hstring *e;

		e = entries[i];
		if (!e) {
			return NULL;
		}
		if (e != DELETED_MARKER(heap) && DUK_HSTRING_GET_BYTELEN(e) == blen) {
			if (memcmp(str, DUK_HSTRING_GET_DATA(e), blen) == 0) {
				DUK_DDDPRINT("find matching hit: %d (step %d, size %d)", i, step, size);
				return e;
			}
		}
		DUK_DDDPRINT("find matching miss: %d (step %d, size %d)", i, step, size);
		i = (i + step) % size;

		/* looping should never happen */
		DUK_ASSERT(i != HASH_INITIAL(strhash, size));
	}
	DUK_NEVER_HERE();
}

static void remove_matching_hstring(duk_heap *heap, duk_hstring **entries, duk_u32 size, duk_hstring *h) {
	duk_u32 i;
	duk_u32 step;

	DUK_ASSERT(size > 0);

	i = HASH_INITIAL(h->hash, size);
	step = HASH_PROBE_STEP(h->hash);
	for (;;) {
		duk_hstring *e;

		e = entries[i];
		if (!e) {
			DUK_NEVER_HERE();
			break;
		}
		if (e == h) {
			/* st_used remains the same, DELETED is counted as used */
			DUK_DDDPRINT("free matching hit: %d", i);
			entries[i] = DELETED_MARKER(heap);
			break;
		}

		DUK_DDDPRINT("free matching miss: %d", i);
		i = (i + step) % size;

		/* looping should never happen */
		DUK_ASSERT(i != HASH_INITIAL(h->hash, size));
	}
}

/*
 *  Hash resizing and resizing policy
 */

static int resize_hash_raw(duk_heap *heap, duk_u32 new_size) {
#ifdef DUK_USE_MARK_AND_SWEEP
	int prev_mark_and_sweep_base_flags;
#endif
#ifdef DUK_USE_DEBUG
	duk_u32 old_used = heap->st_used;
#endif
	duk_u32 old_size = heap->st_size;
	duk_hstring **old_entries = heap->st;
	duk_hstring **new_entries = NULL;
	duk_u32 new_used = 0;
	duk_u32 i;

#ifdef DUK_USE_DEBUG
	DUK_DDDPRINT("attempt to resize stringtable: %d entries, %d bytes, %d used, %d%% load -> %d entries, %d bytes, %d used, %d%% load",
	             (int) old_size, (int) (sizeof(duk_hstring *) * old_size), (int) old_used,
	             (int) (((double) old_used) / ((double) old_size) * 100.0),
	             (int) new_size, (int) (sizeof(duk_hstring *) * new_size), count_used(heap),
	             (int) (((double) count_used(heap)) / ((double) new_size) * 100.0));
#endif

	DUK_ASSERT(new_size > count_used(heap));  /* required for rehash to succeed, equality not that useful */
	DUK_ASSERT(old_entries);
	DUK_ASSERT((heap->mark_and_sweep_base_flags & DUK_MS_FLAG_NO_STRINGTABLE_RESIZE) == 0);

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

	new_entries = (duk_hstring **) DUK_ALLOC(heap, sizeof(duk_hstring *) * new_size);

#ifdef DUK_USE_MARK_AND_SWEEP
	heap->mark_and_sweep_base_flags = prev_mark_and_sweep_base_flags;
#endif

	if (!new_entries) {
		goto error;
	}

#ifdef DUK_USE_EXPLICIT_NULL_INIT
	for (i = 0; i < new_size; i++) {
		new_entries[i] = NULL;
	}
#else
	memset(new_entries, 0, sizeof(duk_hstring *) * new_size);
#endif

	/* Because new_size > count_used(heap), guaranteed to work */
	for (i = 0; i < old_size; i++) {
		duk_hstring *e;

		e = old_entries[i];
		if (e == NULL || e == DELETED_MARKER(heap)) {
			continue;
		}
		/* checking for DELETED_MARKER is not necessary here, but helper does it now */
		insert_hstring(heap, new_entries, new_size, &new_used, e);
	}

#ifdef DUK_USE_DEBUG
	DUK_DPRINT("resized stringtable: %d entries, %d bytes, %d used, %d%% load -> %d entries, %d bytes, %d used, %d%% load",
	           (int) old_size, (int) (sizeof(duk_hstring *) * old_size), (int) old_used,
	           (int) (((double) old_used) / ((double) old_size) * 100.0),
	           (int) new_size, (int) (sizeof(duk_hstring *) * new_size), (int) new_used,
	           (int) (((double) new_used) / ((double) new_size) * 100.0));
#endif

	DUK_FREE(heap, heap->st);
	heap->st = new_entries;
	heap->st_size = new_size;
	heap->st_used = new_used;  /* may be less, since DELETED entries are NULLed by rehash */

	return DUK_ERR_OK;

 error:
	DUK_FREE(heap, new_entries);
	return DUK_ERR_FAIL;
}

static int resize_hash(duk_heap *heap) {
	duk_u32 new_size;
	int ret;

	new_size = count_used(heap);
	if (new_size >= 0x80000000U) {
		new_size = HIGHEST_32BIT_PRIME;
	} else {
		new_size = duk_util_get_hash_prime(GROW_ST_SIZE(new_size));
		new_size = duk_util_get_hash_prime(new_size);
	}
	DUK_ASSERT(new_size > 0);

	/* rehash even if old and new sizes are the same to get rid of
	 * DELETED entries.
	*/ 

	ret = resize_hash_raw(heap, new_size);

	return ret;
}

static int recheck_hash_size(duk_heap *heap, duk_u32 new_used) {
	duk_u32 new_free;
	duk_u32 tmp1;
	duk_u32 tmp2;

	DUK_ASSERT(new_used <= heap->st_size);  /* grow by at most one */
	new_free = heap->st_size - new_used;    /* unsigned intentionally */

	/* new_free / size <= 1 / DIV  <=>  new_free <= size / DIV */
	/* new_used / size <= 1 / DIV  <=>  new_used <= size / DIV */

	tmp1 = heap->st_size / MIN_FREE_DIVISOR;
	tmp2 = heap->st_size / MIN_USED_DIVISOR;

	if (new_free <= tmp1 || new_used <= tmp2) {
		/* load factor too low or high, count actually used entries and resize */
		return resize_hash(heap);
	} else {
		return DUK_ERR_OK;
	}
}

/*
 *  Raw intern and lookup
 */

static duk_hstring *do_intern(duk_heap *heap, duk_u8 *str, duk_u32 blen, duk_u32 strhash) {
	duk_hstring *res;

	if (recheck_hash_size(heap, heap->st_used + 1)) {
		return NULL;
	}

	res = alloc_init_hstring(heap, str, blen, strhash);
	if (!res) {
		return NULL;
	}

	insert_hstring(heap, heap->st, heap->st_size, &heap->st_used, res);  /* guaranteed to succeed */

	/* Note: hstring is in heap but has refcount zero and is not strongly reachable.
	 * Caller should increase refcount and make the hstring reachable before any
	 * operations which require allocation (and possible gc).
	 */

	return res;
}

static duk_hstring *do_lookup(duk_heap *heap, duk_u8 *str, duk_u32 blen, duk_u32 *out_strhash) {
	duk_hstring *res;

	DUK_ASSERT(out_strhash);

	*out_strhash = duk_heap_hashstring(heap, str, blen);
	res = find_matching_string(heap, heap->st, heap->st_size, str, blen, *out_strhash);
	return res;
}

/*
 *  Exposed calls
 */

duk_hstring *duk_heap_string_lookup(duk_heap *heap, duk_u8 *str, duk_u32 blen) {
	duk_u32 strhash;  /* dummy */
	return do_lookup(heap, str, blen, &strhash);
}

duk_hstring *duk_heap_string_intern(duk_heap *heap, duk_u8 *str, duk_u32 blen) {
	duk_hstring *res;
	duk_u32 strhash;

	res = do_lookup(heap, str, blen, &strhash);
	if (res) {
		return res;
	}

	res = do_intern(heap, str, blen, strhash);
	return res;  /* may be NULL */
}

duk_hstring *duk_heap_string_intern_checked(duk_hthread *thr, duk_u8 *str, duk_u32 blen) {
	duk_hstring *res = duk_heap_string_intern(thr->heap, str, blen);
	if (!res) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, "failed to intern string");
	}
	return res;
}

duk_hstring *duk_heap_string_lookup_u32(duk_heap *heap, duk_u32 val) {
	char buf[U32_MAX_STRLEN+1];
	sprintf(buf, "%u", (unsigned int) val);
	return duk_heap_string_lookup(heap, (duk_u8 *) buf, strlen(buf));
}

duk_hstring *duk_heap_string_intern_u32(duk_heap *heap, duk_u32 val) {
	char buf[U32_MAX_STRLEN+1];
	sprintf(buf, "%u", (unsigned int) val);
	return duk_heap_string_intern(heap, (duk_u8 *) buf, strlen(buf));
}

duk_hstring *duk_heap_string_intern_u32_checked(duk_hthread *thr, duk_u32 val) {
	duk_hstring *res = duk_heap_string_intern_u32(thr->heap, val);
	if (!res) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, "failed to intern string");
	}
	return res;
}

/* find and remove string from stringtable; caller must free the string itself */
void duk_heap_string_remove(duk_heap *heap, duk_hstring *h) {
	DUK_DDDPRINT("remove string from stringtable: %!O", h);
	remove_matching_hstring(heap, heap->st, heap->st_size, h);
}

/* essentially shrink check after gc */
void duk_heap_force_stringtable_resize(duk_heap *heap) {
	/* force resize so that DELETED entries are eliminated */
	resize_hash(heap);
}

