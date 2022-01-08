/*
 *  String cache.
 *
 *  Provides a cache to optimize indexed string lookups.  The cache keeps
 *  track of (byte offset, char offset) states for a fixed number of strings.
 *  Otherwise we'd need to scan from either end of the string, as we store
 *  strings in WTF-8.
 */

#include "duk_internal.h"

/*
 *  Delete references to given hstring from the heap string cache.
 *
 *  String cache references are 'weak': they are not counted towards
 *  reference counts, nor serve as roots for mark-and-sweep.  When an
 *  object is about to be freed, such references need to be removed.
 */

DUK_INTERNAL void duk_heap_strcache_string_remove(duk_heap *heap, duk_hstring *h) {
	duk_uint_t i;
	for (i = 0; i < DUK_HEAP_STRCACHE_SIZE; i++) {
		duk_strcache_entry *c = heap->strcache + i;
		if (c->h == h) {
			DUK_DD(
			    DUK_DDPRINT("deleting weak strcache reference to hstring %p from heap %p", (void *) h, (void *) heap));
			c->h = NULL;
			break;
		}
	}

	/* String cannot appear twice, assert for that. */
#if defined(DUK_USE_ASSERTIONS)
	for (i = 0; i < DUK_HEAP_STRCACHE_SIZE; i++) {
		duk_strcache_entry *c = heap->strcache + i;
		DUK_ASSERT(c->h != h);
	}
#endif
}

/*
 *  String cache for WTF-8
 */

#if defined(DUK_USE_DEBUG_LEVEL) && (DUK_USE_DEBUG_LEVEL >= 2)
DUK_LOCAL void duk__strcache_dump_state(duk_heap *heap) {
	duk_uint_t i;

	for (i = 0; i < DUK_HEAP_STRCACHE_SIZE; i++) {
		duk_strcache_entry *c = heap->strcache + i;

		DUK_UNREF(c);
		DUK_DDD(
		    DUK_DDDPRINT("  [%ld] -> h=%p, cidx=%ld, bidx=%ld", (long) i, (void *) c->h, (long) c->cidx, (long) c->bidx));
	}
}
#endif

#if 0
DUK_LOCAL void duk__strcache_scan_char2byte_wtf8_forwards_1(duk_hthread *thr,
                                                            duk_hstring *h,
                                                            duk_uint32_t char_offset,
                                                            duk_uint32_t *out_byteoff,
                                                            duk_uint32_t *out_charoff,
                                                            duk_uint_fast32_t start_byteoff,
                                                            duk_uint_fast32_t start_charoff) {
	const duk_uint8_t *p = duk_hstring_get_data(h) + start_byteoff;
	duk_uint_fast32_t left = char_offset - start_charoff;

	DUK_ASSERT(char_offset >= start_charoff);

	DUK_DD(DUK_DDPRINT("scan forwards %ld codepoints", (long) left));
	while (left > 0) {
		duk_uint8_t t = *p;

		if (t <= 0x7fU) {
			p++;
			left--;
		} else if (t <= 0xdfU) {
			p += 2;
			left--;
		} else if (t <= 0xefU) {
			p += 3;
			left--;
		} else {
			if (left == 1) {
				/* Non-BMP and target char_offset is in the middle.  Caller
				 * must deal with this.
				 */
				*out_byteoff = p - duk_hstring_get_data(h);
				*out_charoff = char_offset - 1;
				return;
			} else {
				p += 4;
				left -= 2;
			}
		}
	}
	*out_byteoff = p - duk_hstring_get_data(h);
	*out_charoff = char_offset;
}
#endif

/* Forward scan lookup. */
const duk_uint_t duk__strcache_wtf8_pstep_lookup[256] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
};
const duk_uint_t duk__strcache_wtf8_leftadj_lookup[256] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

DUK_LOCAL void duk__strcache_scan_char2byte_wtf8_forwards_2(duk_hthread *thr,
                                                            duk_hstring *h,
                                                            duk_uint32_t char_offset,
                                                            duk_uint32_t *out_byteoff,
                                                            duk_uint32_t *out_charoff,
                                                            duk_uint_fast32_t start_byteoff,
                                                            duk_uint_fast32_t start_charoff) {
	const duk_uint8_t *p = duk_hstring_get_data(h) + start_byteoff;
	duk_uint_fast32_t left = char_offset - start_charoff;

	DUK_UNREF(thr);
	DUK_ASSERT(char_offset >= start_charoff);

	DUK_DD(DUK_DDPRINT("scan forwards %ld codepoints", (long) left));
	while (DUK_LIKELY(left >= 4)) {
		duk_uint8_t t;

		/* Maximum leftadj is 2 so with left >= 4 safe to unroll by 2. */

		t = *p;
		p += duk__strcache_wtf8_pstep_lookup[t];
		left -= duk__strcache_wtf8_leftadj_lookup[t];

		t = *p;
		p += duk__strcache_wtf8_pstep_lookup[t];
		left -= duk__strcache_wtf8_leftadj_lookup[t];
	}
	while (left > 0) {
		duk_uint8_t t = *p;
		duk_uint32_t left_adj = duk__strcache_wtf8_leftadj_lookup[t];

		if (DUK_UNLIKELY(left_adj == 2 && left == 1)) {
			/* Non-BMP and target char_offset is in the middle.  Caller
			 * must deal with this.
			 */
			*out_byteoff = p - duk_hstring_get_data(h);
			*out_charoff = char_offset - 1;
			return;
		}

		p += duk__strcache_wtf8_pstep_lookup[t];
		left -= left_adj;
	}
	*out_byteoff = p - duk_hstring_get_data(h);
	*out_charoff = char_offset;
}

DUK_LOCAL void duk__strcache_scan_char2byte_wtf8_forwards(duk_hthread *thr,
                                                          duk_hstring *h,
                                                          duk_uint32_t char_offset,
                                                          duk_uint32_t *out_byteoff,
                                                          duk_uint32_t *out_charoff,
                                                          duk_uint_fast32_t start_byteoff,
                                                          duk_uint_fast32_t start_charoff) {
	duk__strcache_scan_char2byte_wtf8_forwards_2(thr, h, char_offset, out_byteoff, out_charoff, start_byteoff, start_charoff);
}

#if 0
DUK_LOCAL void duk__strcache_scan_char2byte_wtf8_backwards_1(duk_hthread *thr,
                                                             duk_hstring *h,
                                                             duk_uint32_t char_offset,
                                                             duk_uint32_t *out_byteoff,
                                                             duk_uint32_t *out_charoff,
                                                             duk_uint_fast32_t start_byteoff,
                                                             duk_uint_fast32_t start_charoff) {
	const duk_uint8_t *p = duk_hstring_get_data(h) + start_byteoff;
	duk_uint_fast32_t left = start_charoff - char_offset;

	DUK_ASSERT(start_charoff >= char_offset);

	DUK_DD(DUK_DDPRINT("scan backwards %ld codepoints", (long) left));
	while (left > 0) {
		duk_uint8_t t = *(--p);

		if (t <= 0x7fU) {
			left--;
		} else if (t <= 0xbfU) {
			/* Continuation byte. */
		} else if (t <= 0xdfU) {
			left--;
		} else if (t <= 0xefU) {
			left--;
		} else {
			if (left == 1) {
				/* Non-BMP and target char_offset is in the middle.  Caller
				 * must deal with this.
				 */
				*out_byteoff = p - duk_hstring_get_data(h);
				*out_charoff = char_offset - 1;
				return;
			} else {
				left -= 2;
			}
		}
	}
	*out_byteoff = p - duk_hstring_get_data(h);
	*out_charoff = char_offset;
}
#endif

DUK_LOCAL void duk__strcache_scan_char2byte_wtf8_backwards_2(duk_hthread *thr,
                                                             duk_hstring *h,
                                                             duk_uint32_t char_offset,
                                                             duk_uint32_t *out_byteoff,
                                                             duk_uint32_t *out_charoff,
                                                             duk_uint_fast32_t start_byteoff,
                                                             duk_uint_fast32_t start_charoff) {
	const duk_uint8_t *p = duk_hstring_get_data(h) + start_byteoff;
	duk_uint_fast32_t left = start_charoff - char_offset;

	DUK_UNREF(thr);
	DUK_ASSERT(start_charoff >= char_offset);

	DUK_DD(DUK_DDPRINT("scan backwards %ld codepoints", (long) left));
	while (DUK_LIKELY(left >= 4)) {
		/* In backwards direction we scan byte by byte so with left >= 4
		 * it's safe to unroll by 4.
		 */
		duk_uint8_t t;

		t = *(--p);
		left -= duk__strcache_wtf8_leftadj_lookup[t];
		t = *(--p);
		left -= duk__strcache_wtf8_leftadj_lookup[t];
		t = *(--p);
		left -= duk__strcache_wtf8_leftadj_lookup[t];
		t = *(--p);
		left -= duk__strcache_wtf8_leftadj_lookup[t];
	}
	while (left > 0) {
		duk_uint8_t t = *(--p);
		duk_uint32_t left_adj = duk__strcache_wtf8_leftadj_lookup[t];

		if (DUK_UNLIKELY(left_adj == 2 && left == 1)) {
			/* Non-BMP and target char_offset is in the middle.  Caller
			 * must deal with this.
			 */
			*out_byteoff = p - duk_hstring_get_data(h);
			*out_charoff = char_offset - 1;
			return;
		} else {
			left -= left_adj;
		}
	}
	*out_byteoff = p - duk_hstring_get_data(h);
	*out_charoff = char_offset;
}

DUK_LOCAL void duk__strcache_scan_char2byte_wtf8_backwards(duk_hthread *thr,
                                                           duk_hstring *h,
                                                           duk_uint32_t char_offset,
                                                           duk_uint32_t *out_byteoff,
                                                           duk_uint32_t *out_charoff,
                                                           duk_uint_fast32_t start_byteoff,
                                                           duk_uint_fast32_t start_charoff) {
	duk__strcache_scan_char2byte_wtf8_backwards_2(thr, h, char_offset, out_byteoff, out_charoff, start_byteoff, start_charoff);
}

DUK_LOCAL void duk__strcache_scan_char2byte_wtf8_uncached(duk_hthread *thr,
                                                          duk_hstring *h,
                                                          duk_uint32_t char_offset,
                                                          duk_uint32_t *out_byteoff,
                                                          duk_uint32_t *out_charoff) {
	duk_uint_fast32_t char_length;
	duk_uint_fast32_t dist_start;
	duk_uint_fast32_t dist_end;
	duk_bool_t prefer_forwards;

	char_length = (duk_uint_fast32_t) duk_hstring_get_charlen(h);
	DUK_ASSERT(duk_hstring_get_charlen(h) >= char_offset);

	dist_start = char_offset;
	dist_end = char_length - char_offset;

	/* Scanning valid WTF-8 forwards is faster than scanning it backwards
	 * because in the forwards direction we can skip the continuation bytes.
	 * So prefer scanning forwards with a crude quick check.
	 */
	prefer_forwards = (dist_start / 2 <= dist_end);

	if (prefer_forwards) {
		duk_uint_fast32_t start_boff = 0;
		duk_uint_fast32_t start_coff = 0;

		duk__strcache_scan_char2byte_wtf8_forwards(thr, h, char_offset, out_byteoff, out_charoff, start_boff, start_coff);
	} else {
		duk_uint_fast32_t start_boff = duk_hstring_get_bytelen(h);
		duk_uint_fast32_t start_coff = duk_hstring_get_charlen(h);

		duk__strcache_scan_char2byte_wtf8_backwards(thr, h, char_offset, out_byteoff, out_charoff, start_boff, start_coff);
	}
}

DUK_LOCAL void duk__strcache_scan_char2byte_wtf8_cached(duk_hthread *thr,
                                                        duk_hstring *h,
                                                        duk_uint32_t char_offset,
                                                        duk_uint32_t *out_byteoff,
                                                        duk_uint32_t *out_charoff) {
	duk_heap *heap;
	duk_uint_t i;
	duk_strcache_entry *sce = NULL;
	duk_uint_fast32_t dist_start;
	duk_uint_fast32_t dist_end;
	duk_uint_fast32_t dist_sce;
	duk_uint_fast32_t char_length;

	heap = thr->heap;
	char_length = (duk_uint_fast32_t) duk_hstring_get_charlen(h);
	DUK_ASSERT(duk_hstring_get_charlen(h) >= char_offset);

#if defined(DUK_USE_DEBUG_LEVEL) && (DUK_USE_DEBUG_LEVEL >= 2)
	DUK_DDD(DUK_DDDPRINT("stringcache before char2byte (using cache):"));
	duk__strcache_dump_state(thr->heap);
#endif

	for (i = 0; i < DUK_HEAP_STRCACHE_SIZE; i++) {
		duk_strcache_entry *c = heap->strcache + i;

		if (c->h == h) {
			sce = c;
			break;
		}
	}

	dist_start = char_offset;
	dist_end = char_length - char_offset;
	dist_sce = 0;

	if (sce) {
		if (char_offset >= sce->cidx) {
			/* Prefer forward scan from sce to scanning from end. */
			dist_sce = char_offset - sce->cidx;
			DUK_ASSERT(dist_sce <= dist_start);
			if (dist_sce / 2 <= dist_end) {
				DUK_DDD(DUK_DDDPRINT("non-ascii string, sce=%p:%ld:%ld, "
				                     "dist_start=%ld, dist_end=%ld, dist_sce=%ld => "
				                     "scan forwards from sce",
				                     (void *) (sce ? sce->h : NULL),
				                     (sce ? (long) sce->cidx : (long) -1),
				                     (sce ? (long) sce->bidx : (long) -1),
				                     (long) dist_start,
				                     (long) dist_end,
				                     (long) dist_sce));

				duk__strcache_scan_char2byte_wtf8_forwards(thr,
				                                           h,
				                                           char_offset,
				                                           out_byteoff,
				                                           out_charoff,
				                                           sce->bidx,
				                                           sce->cidx);
				goto scan_done;
			}
		} else {
			/* Prefer forward scan from start to scanning from sce. */
			dist_sce = sce->cidx - char_offset;
			DUK_ASSERT(dist_sce <= dist_end);
			if (dist_sce <= dist_start / 2) {
				DUK_DDD(DUK_DDDPRINT("non-ascii string, sce=%p:%ld:%ld, "
				                     "dist_start=%ld, dist_end=%ld, dist_sce=%ld => "
				                     "scan backwards from sce",
				                     (void *) (sce ? sce->h : NULL),
				                     (sce ? (long) sce->cidx : (long) -1),
				                     (sce ? (long) sce->bidx : (long) -1),
				                     (long) dist_start,
				                     (long) dist_end,
				                     (long) dist_sce));

				duk__strcache_scan_char2byte_wtf8_backwards(thr,
				                                            h,
				                                            char_offset,
				                                            out_byteoff,
				                                            out_charoff,
				                                            sce->bidx,
				                                            sce->cidx);
				goto scan_done;
			}
		}
	}

	/* no sce, or sce scan not best */
	duk__strcache_scan_char2byte_wtf8_uncached(thr, h, char_offset, out_byteoff, out_charoff);

scan_done:
	/*
	 *  Update cache entry (allocating if necessary), and move the
	 *  cache entry to the first place (in an "LRU" policy).
	 */

	if (!sce) {
		DUK_DD(DUK_DDPRINT("no stringcache entry, allocate one"));
		sce = heap->strcache + DUK_HEAP_STRCACHE_SIZE - 1; /* take last entry */
		sce->h = h;
	}
	DUK_ASSERT(sce != NULL);
	sce->bidx = (duk_uint32_t) *out_byteoff;
	sce->cidx = (duk_uint32_t) *out_charoff;
	DUK_DD(DUK_DDPRINT("stringcache entry updated to bidx=%ld, cidx=%ld", (long) sce->bidx, (long) sce->cidx));

	/* LRU: move our entry to first */
	if (sce > &heap->strcache[0]) {
		/*
		 *   A                  C
		 *   B                  A
		 *   C <- sce    ==>    B
		 *   D                  D
		 */
		duk_strcache_entry tmp;

		tmp = *sce;
		duk_memmove((void *) (&heap->strcache[1]),
		            (const void *) (&heap->strcache[0]),
		            (size_t) (((char *) sce) - ((char *) &heap->strcache[0])));
		heap->strcache[0] = tmp;

		/* 'sce' points to the wrong entry here, but is no longer used */
	}
#if defined(DUK_USE_DEBUG_LEVEL) && (DUK_USE_DEBUG_LEVEL >= 2)
	DUK_DDD(DUK_DDDPRINT("stringcache after char2byte (using cache):"));
	duk__strcache_dump_state(thr->heap);
#endif
}

/* Scan for codepoint taking advantage of possible string cache entry.  Update
 * string cache entry to point to the scanned byte/char offset position.  For
 * non-BMP codepoints, update the cache to point to the high surrogate.
 */
DUK_INTERNAL void duk_strcache_scan_char2byte_wtf8(duk_hthread *thr,
                                                   duk_hstring *h,
                                                   duk_uint32_t char_offset,
                                                   duk_uint32_t *out_byteoff,
                                                   duk_uint32_t *out_charoff) {
	duk_bool_t use_cache;
	duk_uint_fast32_t char_length;

	DUK_ASSERT(!DUK_HSTRING_HAS_SYMBOL(h));

	/*
	 *  For ASCII strings, the answer is simple.
	 */

	if (DUK_LIKELY(duk_hstring_is_ascii(h) != 0)) {
		*out_byteoff = char_offset;
		*out_charoff = char_offset;
		return;
	}

	char_length = (duk_uint_fast32_t) duk_hstring_get_charlen(h);
	DUK_ASSERT(char_offset <= char_length);

	if (DUK_LIKELY(duk_hstring_is_ascii(h) != 0)) {
		/* Must recheck because the 'is ascii' flag may be set
		 * lazily.  Alternatively, we could just compare charlen
		 * to bytelen.
		 */
		*out_byteoff = char_offset;
		*out_charoff = char_offset;
		return;
	}

	/*
	 *  For non-ASCII strings, we need to scan forwards or backwards
	 *  from some starting point.  The starting point may be the start
	 *  or end of the string, or some cached midpoint in the string
	 *  cache.
	 *
	 *  For "short" strings we simply scan without checking or updating
	 *  the cache.  For longer strings we check and update the cache as
	 *  necessary, inserting a new cache entry if none exists.
	 */

	DUK_DDD(DUK_DDDPRINT("non-ascii string %p, char_offset=%ld, clen=%ld, blen=%ld",
	                     (void *) h,
	                     (long) char_offset,
	                     (long) duk_hstring_get_charlen(h),
	                     (long) duk_hstring_get_bytelen(h)));

	use_cache = (char_length > DUK_HEAP_STRINGCACHE_NOCACHE_LIMIT);

	if (use_cache) {
		duk__strcache_scan_char2byte_wtf8_cached(thr, h, char_offset, out_byteoff, out_charoff);
	} else {
		duk__strcache_scan_char2byte_wtf8_uncached(thr, h, char_offset, out_byteoff, out_charoff);
	}
}
