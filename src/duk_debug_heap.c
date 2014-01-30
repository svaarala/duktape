/*
 *  Debug dumping of duk_heap.
 */

#include "duk_internal.h"

#ifdef DUK_USE_DEBUG

static void sanitize_snippet(char *buf, int buf_size, duk_hstring *str) {
	int i;
	int nchars;
	int maxchars;
	duk_uint8_t *data;

	DUK_MEMSET(buf, 0, buf_size);

	maxchars = buf_size - 1;
	data = DUK_HSTRING_GET_DATA(str);
	nchars = (str->blen < maxchars ? str->blen : maxchars);
	for (i = 0; i < nchars; i++) {
		char c = (char) data[i];
		if (c < 0x20 || c > 0x7e) {
			c = '.';
		}
		buf[i] = c;
	}
}

static const char *get_heap_type_string(duk_heaphdr *hdr) {
	switch (DUK_HEAPHDR_GET_TYPE(hdr)) {
	case DUK_HTYPE_STRING:
		return "string";
	case DUK_HTYPE_OBJECT:
		return "object";
	case DUK_HTYPE_BUFFER:
		return "buffer";
	default:
		return "???";
	}
}

static void dump_indented(duk_heaphdr *obj, int index) {
#ifdef DUK_USE_REFERENCE_COUNTING
	DUK_DPRINT("  [%d]: %p %s (flags: 0x%08x, ref: %d) -> %!O",
	           index,
	           (void *) obj,
	           get_heap_type_string(obj),
	           (int) DUK_HEAPHDR_GET_FLAGS(obj),
	           DUK_HEAPHDR_GET_REFCOUNT(obj),
	           obj);
#else
	DUK_DPRINT("  [%d]: %p %s (flags: 0x%08x) -> %!O",
	           index,
	           (void *) obj,
	           get_heap_type_string(obj),
	           (int) DUK_HEAPHDR_GET_FLAGS(obj),
	           obj);
#endif
}

static void dump_heaphdr_list(duk_heap *heap, duk_heaphdr *root, const char *name) {
	int count;
	duk_heaphdr *curr;

	DUK_UNREF(heap);

	count = 0;
	curr = root;
	while (curr) {
		count++;
		curr = DUK_HEAPHDR_GET_NEXT(curr);
	}

	DUK_DPRINT("%s, %d objects", name, count);

	count = 0;
	curr = root;
	while (curr) {
		count++;
		dump_indented(curr, count);
		curr = DUK_HEAPHDR_GET_NEXT(curr);
	}
}

static void dump_stringtable(duk_heap *heap) {
	duk_uint32_t i;
	char buf[64+1];

	DUK_DPRINT("stringtable %p, used %d, size %d, load %d%%",
	           (void *) heap->st,
	           (int) heap->st_used,
	           (int) heap->st_size,
	           (int) (((double) heap->st_used) / ((double) heap->st_size) * 100.0));

	for (i = 0; i < heap->st_size; i++) {
		duk_hstring *e = heap->st[i];

		if (!e) {
			DUK_DPRINT("  [%d]: NULL", i);
		} else if (e == DUK_STRTAB_DELETED_MARKER(heap)) {
			DUK_DPRINT("  [%d]: DELETED", i);
		} else {
			sanitize_snippet(buf, sizeof(buf), e);

			/* FIXME: all string flags not printed now */

#ifdef DUK_USE_REFERENCE_COUNTING
			DUK_DPRINT("  [%d]: %p (flags: 0x%08x, ref: %d) '%s', strhash=0x%08x, blen=%d, clen=%d, arridx=%d, internal=%d",
			           i,
			           (void *) e,
			           (int) DUK_HEAPHDR_GET_FLAGS((duk_heaphdr *) e),
			           (int) DUK_HEAPHDR_GET_REFCOUNT((duk_heaphdr *) e),
			           buf,
			           (int) e->hash,
			           (int) e->blen,
			           (int) e->clen,
			           DUK_HSTRING_HAS_ARRIDX(e) ? 1 : 0,
			           DUK_HSTRING_HAS_INTERNAL(e) ? 1 : 0);
#else
			DUK_DPRINT("  [%d]: %p (flags: 0x%08x) '%s', strhash=0x%08x, blen=%d, clen=%d, arridx=%d, internal=%d",
			           i,
			           (void *) e,
			           (int) DUK_HEAPHDR_GET_FLAGS((duk_heaphdr *) e),
			           buf,
			           (int) e->hash,
			           (int) e->blen,
			           (int) e->clen,
			           DUK_HSTRING_HAS_ARRIDX(e) ? 1 : 0,
			           DUK_HSTRING_HAS_INTERNAL(e) ? 1 : 0);
#endif
		}
	}
}

static void dump_strcache(duk_heap *heap) {
	duk_uint32_t i;
	char buf[64+1];

	DUK_DPRINT("stringcache");

	for (i = 0; i < DUK_HEAP_STRCACHE_SIZE; i++) {
		duk_strcache *c = &heap->strcache[i];
		if (!c->h) {
			DUK_DPRINT("  [%d]: bidx=%d, cidx=%d, str=NULL",
			           i, c->bidx, c->cidx);
		} else {
			sanitize_snippet(buf, sizeof(buf), c->h);
			DUK_DPRINT("  [%d]: bidx=%d cidx=%d str=%s",
			           i, c->bidx, c->cidx, buf);
		}
	} 
}

void duk_debug_dump_heap(duk_heap *heap) {
	char buf[64+1];

	DUK_DPRINT("=== heap %p ===", (void *) heap);
	DUK_DPRINT("  flags: 0x%08x", (int) heap->flags);

	/* Note: there is no standard formatter for function pointers */
#ifdef DUK_USE_GCC_PRAGMAS
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#endif
	duk_debug_format_funcptr(buf, sizeof(buf), (unsigned char *) &heap->alloc_func, sizeof(heap->alloc_func));
	DUK_DPRINT("  alloc_func: %s", buf);
	duk_debug_format_funcptr(buf, sizeof(buf), (unsigned char *) &heap->realloc_func, sizeof(heap->realloc_func));
	DUK_DPRINT("  realloc_func: %s", buf);
	duk_debug_format_funcptr(buf, sizeof(buf), (unsigned char *) &heap->free_func, sizeof(heap->free_func));
	DUK_DPRINT("  free_func: %s", buf);
#ifdef DUK_USE_GCC_PRAGMAS
#pragma GCC diagnostic pop
#endif

	DUK_DPRINT("  alloc_udata: %p", (void *) heap->alloc_udata);

#ifdef DUK_USE_MARK_AND_SWEEP
#ifdef DUK_USE_VOLUNTARY_GC
	DUK_DPRINT("  mark-and-sweep trig counter: %d", heap->mark_and_sweep_trigger_counter);
#endif
	DUK_DPRINT("  mark-and-sweep rec depth: %d", heap->mark_and_sweep_recursion_depth);
	DUK_DPRINT("  mark-and-sweep base flags: 0x%08x", heap->mark_and_sweep_base_flags);
#endif

	/* FIXME: heap->fatal_func */

	DUK_DPRINT("  lj.jmpbuf_ptr: %p", (void *) heap->lj.jmpbuf_ptr);
	DUK_DPRINT("  lj.errhandler: %!@O", (duk_heaphdr *) heap->lj.errhandler);
	DUK_DPRINT("  lj.type: %d", heap->lj.type);
	DUK_DPRINT("  lj.value1: %!T", &heap->lj.value1);
	DUK_DPRINT("  lj.value2: %!T", &heap->lj.value2);
	DUK_DPRINT("  lj.iserror: %d", heap->lj.iserror);

	DUK_DPRINT("  handling_error: %d", heap->handling_error);

	DUK_DPRINT("  heap_thread: %!@O", (duk_heaphdr *) heap->heap_thread);
	DUK_DPRINT("  curr_thread: %!@O", (duk_heaphdr *) heap->curr_thread);
	DUK_DPRINT("  heap_object: %!@O", (duk_heaphdr *) heap->heap_object);

	DUK_DPRINT("  call_recursion_depth: %d", heap->call_recursion_depth);
	DUK_DPRINT("  call_recursion_limit: %d", heap->call_recursion_limit);

	DUK_DPRINT("  hash_seed: 0x%08x", (int) heap->hash_seed);
	DUK_DPRINT("  rnd_state: 0x%08x", (int) heap->rnd_state);

	dump_strcache(heap);

	dump_heaphdr_list(heap, heap->heap_allocated, "heap allocated");

#ifdef DUK_USE_REFERENCE_COUNTING
	dump_heaphdr_list(heap, heap->refzero_list, "refcounting refzero list");
#endif

#ifdef DUK_USE_MARK_AND_SWEEP
	dump_heaphdr_list(heap, heap->finalize_list, "mark-and-sweep finalize list");
#endif

	dump_stringtable(heap);

	/* heap->strs: not worth dumping */
}

#endif  /* DUK_USE_DEBUG */

