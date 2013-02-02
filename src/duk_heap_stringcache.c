/*
 *  String cache.
 *
 *  Provides a cache to optimize indexed string lookups.  The cache keeps
 *  track of (byte offset, char offset) states for a fixed number of strings.
 *  Otherwise we'd need to scan from either end of the string, as we store
 *  strings in (extended) UTF-8.
 */

#include "duk_internal.h"

/*
 *  Delete references to given hstring from the heap string cache.
 *
 *  String cache references are 'weak': they are not counted towards
 *  reference counts, nor serve as roots for mark-and-sweep.  When an
 *  object is about to be freed, such references need to be removed.
 */

void duk_heap_strcache_string_remove(duk_heap *heap, duk_hstring *h) {
	int i;
	for (i = 0; i < DUK_HEAP_STRCACHE_SIZE; i++) {
		duk_strcache *c = &heap->strcache[i];
		if (c->h == h) {
			DUK_DDPRINT("deleting weak strcache reference to hstring %p from heap %p",
			            (void *) h, (void *) heap);
			c->h = NULL;

			/* XXX: the string shouldn't appear twice, but we now loop to the
			 * end anyway; if fixed, add a looping assertion to ensure there
			 * is no duplicate.
			 */
		}
	}
}

/*
 *  String scanning helpers
 */

static duk_u8 *scan_forwards(duk_u8 *p, duk_u8 *q, duk_u32 n) {
	while (n > 0) {
		for (;;) {
			p++;
			if (p >= q) {
				return NULL;
			}
			if ((*p & 0xc0) != 0x80) {
				break;
			}
		}
		n--;
	}
	return p;
}

static duk_u8 *scan_backwards(duk_u8 *p, duk_u8 *q, duk_u32 n) {
	while (n > 0) {
		for (;;) {
			p--;
			if (p < q) {
				return NULL;
			}
			if ((*p & 0xc0) != 0x80) {
				break;
			}
		}
		n--;
	}
	return p;
}

/*
 *  Convert char offset to byte offset
 *
 *  Avoid using the string cache if possible: for ASCII strings byte and
 *  char offsets are equal and for short strings direct scanning may be
 *  better than using the string cache (which may evict a more important
 *  entry).
 */

/* FIXME: typing throughout */

duk_u32 duk_heap_strcache_offset_char2byte(duk_hthread *thr, duk_hstring *h, duk_u32 char_offset) {
	duk_heap *heap;
	duk_strcache *sce;
	duk_u32 byte_offset;
	int i;
	int use_cache;
	duk_u32 dist_start, dist_end, dist_sce;
	duk_u8 *p_start;
	duk_u8 *p_end;
	duk_u8 *p_found;

	if (char_offset > DUK_HSTRING_GET_CHARLEN(h)) {
		goto error;
	}

	/*
	 *  For ASCII strings, the answer is simple.
	 */

	if (DUK_HSTRING_IS_ASCII(h)) {
		/* clen == blen -> pure ascii */
		return char_offset;
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

	DUK_DDDPRINT("non-ascii string %p, char_offset=%d, clen=%d, blen=%d",
	             (void *) h, char_offset, DUK_HSTRING_GET_CHARLEN(h),
	             DUK_HSTRING_GET_BYTELEN(h));

	heap = thr->heap;
	sce = NULL;
	use_cache = (DUK_HSTRING_GET_CHARLEN(h) > DUK_HEAP_STRINGCACHE_NOCACHE_LIMIT);

	if (use_cache) {
#ifdef DUK_USE_DDDEBUG
		DUK_DDDPRINT("stringcache before char2byte (using cache):");
		for (i = 0; i < DUK_HEAP_STRCACHE_SIZE; i++) {
			duk_strcache *c = &heap->strcache[i];
			DUK_DDDPRINT("  [%d] -> h=%p, cidx=%d, bidx=%d", i, c->h, c->cidx, c->bidx);
		}
#endif

		for (i = 0; i < DUK_HEAP_STRCACHE_SIZE; i++) {
			duk_strcache *c = &heap->strcache[i];

			if (c->h == h) {
				sce = c;
				break;
			}
		}
	}

	/*
	 *  Scan from shortest distance:
	 *    - start of string
	 *    - end of string
	 *    - cache entry (if exists)
	 */

	DUK_ASSERT(DUK_HSTRING_GET_CHARLEN(h) >= char_offset);
	dist_start = char_offset;
	dist_end = DUK_HSTRING_GET_CHARLEN(h) - char_offset;

	p_start = (duk_u8 *) DUK_HSTRING_GET_DATA(h);
	p_end = (duk_u8 *) (p_start + DUK_HSTRING_GET_BYTELEN(h));
	p_found = NULL;

	if (sce) {
		if (char_offset >= sce->cidx) {
			dist_sce = char_offset - sce->cidx;
			if ((dist_sce <= dist_start) && (dist_sce <= dist_end)) {
				DUK_DDDPRINT("non-ascii string, use_cache=%d, sce=%p:%d:%d, "
				             "dist_start=%d, dist_end=%d, dist_sce=%d => "
				             "scan forwards from sce",
				             use_cache, (sce ? sce->h : NULL), (sce ? sce->cidx : -1),
				             (sce ? sce->bidx : -1), dist_start, dist_end, dist_sce);

				p_found = scan_forwards(p_start + sce->bidx,
				                        p_end,
				                        dist_sce);
				goto scan_done;
			}
		} else {
			dist_sce = sce->cidx - char_offset;
			if ((dist_sce <= dist_start) && (dist_sce <= dist_end)) {
				DUK_DDDPRINT("non-ascii string, use_cache=%d, sce=%p:%d:%d, "
				             "dist_start=%d, dist_end=%d, dist_sce=%d => "
				             "scan backwards from sce",
				             use_cache, (sce ? sce->h : NULL), (sce ? sce->cidx : -1),
				             (sce ? sce->bidx : -1), dist_start, dist_end, dist_sce);

				p_found = scan_backwards(p_start + sce->bidx,
				                         p_start,
				                         dist_sce);
				goto scan_done;
			}
		}
	}

	/* no sce, or sce scan not best */

	if (dist_start <= dist_end) {
		DUK_DDDPRINT("non-ascii string, use_cache=%d, sce=%p:%d:%d, "
		             "dist_start=%d, dist_end=%d, dist_sce=%d => "
		             "scan forwards from string start",
		             use_cache, (sce ? sce->h : NULL), (sce ? sce->cidx : -1),
		             (sce ? sce->bidx : -1), dist_start, dist_end, dist_sce);

		p_found = scan_forwards(p_start,
		                        p_end,
		                        dist_start);
	} else {
		DUK_DDDPRINT("non-ascii string, use_cache=%d, sce=%p:%d:%d, "
		             "dist_start=%d, dist_end=%d, dist_sce=%d => "
		             "scan backwards from string end",
		             use_cache, (sce ? sce->h : NULL), (sce ? sce->cidx : -1),
		             (sce ? sce->bidx : -1), dist_start, dist_end, dist_sce);

		p_found = scan_backwards(p_end,
		                         p_start,
		                         dist_end);
	}

 scan_done:

	if (!p_found) {
		/* Scan error: this shouldn't normally happen; it could happen if
		 * string is not valid UTF-8 data, and clen/blen are not consistent
		 * with the scanning algorithm.
		 */
		goto error;
	}

	DUK_ASSERT(p_found >= p_start);
	DUK_ASSERT(p_found <= p_end);  /* may be equal */
	byte_offset = (duk_u32) (p_found - p_start);

	DUK_DDDPRINT("-> string %p, cidx %d -> bidx %d", (void *) h, char_offset, byte_offset);

	/*
	 *  Update cache entry (allocating if necessary), and move the
	 *  cache entry to the first place (in an "LRU" policy).
	 */
	
	if (use_cache) {
		/* update entry, allocating if necessary */
		if (!sce) {
			sce = heap->strcache + DUK_HEAP_STRCACHE_SIZE - 1;  /* take last entry */
			sce->h = h;
		}
		DUK_ASSERT(sce != NULL);
		sce->bidx = (duk_u32) (p_found - p_start);
		sce->cidx = char_offset;

		/* LRU: move our entry to first */
		if (sce > &heap->strcache[0]) {
			/*
			 *   A                  C
			 *   B                  A
			 *   C <- sce    ==>    B
			 *   D                  D
			 */
			duk_strcache tmp;

			tmp = *sce;
			memmove((void *) (&heap->strcache[1]),
			        (void *) (&heap->strcache[0]),
			        (size_t) (((char *) sce) - ((char *) &heap->strcache[0])));
			heap->strcache[0] = tmp;

			/* 'sce' points to the wrong entry here, but is no longer used */
		}
#ifdef DUK_USE_DDDEBUG
		DUK_DDDPRINT("stringcache after char2byte (using cache):");
		for (i = 0; i < DUK_HEAP_STRCACHE_SIZE; i++) {
			duk_strcache *c = &heap->strcache[i];
			DUK_DDDPRINT("  [%d] -> h=%p, cidx=%d, bidx=%d", i, c->h, c->cidx, c->bidx);
		}
#endif
	}

	return byte_offset;

 error:
	DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, "string scan error");
	return 0;
}


