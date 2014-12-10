/*
 *  Debug dumping of duk_heap.
 */

#include "duk_internal.h"

#ifdef DUK_USE_DEBUG

#if 0  /*unused*/
DUK_LOCAL void duk__sanitize_snippet(char *buf, duk_size_t buf_size, duk_hstring *str) {
	duk_size_t i;
	duk_size_t nchars;
	duk_size_t maxchars;
	duk_uint8_t *data;

	DUK_MEMZERO(buf, buf_size);

	maxchars = (duk_size_t) (buf_size - 1);
	data = DUK_HSTRING_GET_DATA(str);
	nchars = ((duk_size_t) str->blen < maxchars ? (duk_size_t) str->blen : maxchars);
	for (i = 0; i < nchars; i++) {
		duk_small_int_t c = (duk_small_int_t) data[i];
		if (c < 0x20 || c > 0x7e) {
			c = '.';
		}
		buf[i] = (char) c;
	}
}
#endif

#if 0
DUK_LOCAL const char *duk__get_heap_type_string(duk_heaphdr *hdr) {
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
#endif

#if 0
DUK_LOCAL void duk__dump_indented(duk_heaphdr *obj, int index) {
	DUK_UNREF(obj);
	DUK_UNREF(index);
	DUK_UNREF(duk__get_heap_type_string);

#ifdef DUK_USE_REFERENCE_COUNTING
	DUK_D(DUK_DPRINT("  [%ld]: %p %s (flags: 0x%08lx, ref: %ld) -> %!O",
	                 (long) index,
	                 (void *) obj,
	                 (const char *) duk__get_heap_type_string(obj),
	                 (unsigned long) DUK_HEAPHDR_GET_FLAGS(obj),
	                 (long) DUK_HEAPHDR_GET_REFCOUNT(obj),
	                 (duk_heaphdr *) obj));
#else
	DUK_D(DUK_DPRINT("  [%ld]: %p %s (flags: 0x%08lx) -> %!O",
	                 (long) index,
	                 (void *) obj,
	                 (const char *) duk__get_heap_type_string(obj),
	                 (unsigned long) DUK_HEAPHDR_GET_FLAGS(obj),
	                 (duk_heaphdr *) obj));
#endif
}
#endif

#if 0  /*unused*/
DUK_LOCAL void duk__dump_heaphdr_list(duk_heap *heap, duk_heaphdr *root, const char *name) {
	duk_int_t count;
	duk_heaphdr *curr;

	DUK_UNREF(heap);
	DUK_UNREF(name);

	count = 0;
	curr = root;
	while (curr) {
		count++;
		curr = DUK_HEAPHDR_GET_NEXT(curr);
	}

	DUK_D(DUK_DPRINT("%s, %ld objects", (const char *) name, (long) count));

	count = 0;
	curr = root;
	while (curr) {
		count++;
		duk__dump_indented(curr, count);
		curr = DUK_HEAPHDR_GET_NEXT(curr);
	}
}
#endif

#if 0  /*unused*/
DUK_LOCAL void duk__dump_stringtable(duk_heap *heap) {
	duk_uint_fast32_t i;
	char buf[64+1];

	DUK_D(DUK_DPRINT("stringtable %p, used %ld, size %ld, load %ld%%",
	                 (void *) heap->strtable,
	                 (long) heap->st_used,
	                 (long) heap->st_size,
	                 (long) (((double) heap->st_used) / ((double) heap->st_size) * 100.0)));

	for (i = 0; i < (duk_uint_fast32_t) heap->st_size; i++) {
		duk_hstring *e = heap->strtable[i];

		if (!e) {
			DUK_D(DUK_DPRINT("  [%ld]: NULL", (long) i));
		} else if (e == DUK_STRTAB_DELETED_MARKER(heap)) {
			DUK_D(DUK_DPRINT("  [%ld]: DELETED", (long) i));
		} else {
			duk__sanitize_snippet(buf, sizeof(buf), e);

#ifdef DUK_USE_REFERENCE_COUNTING
			DUK_D(DUK_DPRINT("  [%ld]: %p (flags: 0x%08lx, ref: %ld) '%s', strhash=0x%08lx, blen=%ld, clen=%ld, "
			                 "arridx=%ld, internal=%ld, reserved_word=%ld, strict_reserved_word=%ld, eval_or_arguments=%ld",
			                 (long) i,
			                 (void *) e,
			                 (unsigned long) DUK_HEAPHDR_GET_FLAGS((duk_heaphdr *) e),
			                 (long) DUK_HEAPHDR_GET_REFCOUNT((duk_heaphdr *) e),
			                 (const char *) buf,
			                 (unsigned long) e->hash,
			                 (long) e->blen,
			                 (long) e->clen,
			                 (long) (DUK_HSTRING_HAS_ARRIDX(e) ? 1 : 0),
			                 (long) (DUK_HSTRING_HAS_INTERNAL(e) ? 1 : 0),
			                 (long) (DUK_HSTRING_HAS_RESERVED_WORD(e) ? 1 : 0),
			                 (long) (DUK_HSTRING_HAS_STRICT_RESERVED_WORD(e) ? 1 : 0),
			                 (long) (DUK_HSTRING_HAS_EVAL_OR_ARGUMENTS(e) ? 1 : 0)));
#else
			DUK_D(DUK_DPRINT("  [%ld]: %p (flags: 0x%08lx) '%s', strhash=0x%08lx, blen=%ld, clen=%ld, "
			                 "arridx=%ld, internal=%ld, reserved_word=%ld, strict_reserved_word=%ld, eval_or_arguments=%ld",
			                 (long) i,
			                 (void *) e,
			                 (unsigned long) DUK_HEAPHDR_GET_FLAGS((duk_heaphdr *) e),
			                 (const char *) buf,
			                 (long) e->hash,
			                 (long) e->blen,
			                 (long) e->clen,
			                 (long) (DUK_HSTRING_HAS_ARRIDX(e) ? 1 : 0),
			                 (long) (DUK_HSTRING_HAS_INTERNAL(e) ? 1 : 0),
			                 (long) (DUK_HSTRING_HAS_RESERVED_WORD(e) ? 1 : 0),
			                 (long) (DUK_HSTRING_HAS_STRICT_RESERVED_WORD(e) ? 1 : 0),
			                 (long) (DUK_HSTRING_HAS_EVAL_OR_ARGUMENTS(e) ? 1 : 0)));
#endif
		}
	}
}
#endif

#if 0  /*unused*/
DUK_LOCAL void duk__dump_strcache(duk_heap *heap) {
	duk_uint_fast32_t i;
	char buf[64+1];

	DUK_D(DUK_DPRINT("stringcache"));

	for (i = 0; i < (duk_uint_fast32_t) DUK_HEAP_STRCACHE_SIZE; i++) {
		duk_strcache *c = &heap->strcache[i];
		if (!c->h) {
			DUK_D(DUK_DPRINT("  [%ld]: bidx=%ld, cidx=%ld, str=NULL",
			                 (long) i, (long) c->bidx, (long) c->cidx));
		} else {
			duk__sanitize_snippet(buf, sizeof(buf), c->h);
			DUK_D(DUK_DPRINT("  [%ld]: bidx=%ld cidx=%ld str=%s",
			                 (long) i, (long) c->bidx, (long) c->cidx, (const char *) buf));
		}
	}
}
#endif

#if 0  /*unused*/
DUK_INTERNAL void duk_debug_dump_heap(duk_heap *heap) {
	char buf[64+1];

	DUK_D(DUK_DPRINT("=== heap %p ===", (void *) heap));
	DUK_D(DUK_DPRINT("  flags: 0x%08lx", (unsigned long) heap->flags));

	/* Note: there is no standard formatter for function pointers */
#ifdef DUK_USE_GCC_PRAGMAS
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#endif
	duk_debug_format_funcptr(buf, sizeof(buf), (duk_uint8_t *) &heap->alloc_func, sizeof(heap->alloc_func));
	DUK_D(DUK_DPRINT("  alloc_func: %s", (const char *) buf));
	duk_debug_format_funcptr(buf, sizeof(buf), (duk_uint8_t *) &heap->realloc_func, sizeof(heap->realloc_func));
	DUK_D(DUK_DPRINT("  realloc_func: %s", (const char *) buf));
	duk_debug_format_funcptr(buf, sizeof(buf), (duk_uint8_t *) &heap->free_func, sizeof(heap->free_func));
	DUK_D(DUK_DPRINT("  free_func: %s", (const char *) buf));
	duk_debug_format_funcptr(buf, sizeof(buf), (duk_uint8_t *) &heap->fatal_func, sizeof(heap->fatal_func));
	DUK_D(DUK_DPRINT("  fatal_func: %s", (const char *) buf));
#ifdef DUK_USE_GCC_PRAGMAS
#pragma GCC diagnostic pop
#endif

	DUK_D(DUK_DPRINT("  heap_udata: %p", (void *) heap->heap_udata));

#ifdef DUK_USE_MARK_AND_SWEEP
#ifdef DUK_USE_VOLUNTARY_GC
	DUK_D(DUK_DPRINT("  mark-and-sweep trig counter: %ld", (long) heap->mark_and_sweep_trigger_counter));
#endif
	DUK_D(DUK_DPRINT("  mark-and-sweep rec depth: %ld", (long) heap->mark_and_sweep_recursion_depth));
	DUK_D(DUK_DPRINT("  mark-and-sweep base flags: 0x%08lx", (unsigned long) heap->mark_and_sweep_base_flags));
#endif

	DUK_D(DUK_DPRINT("  lj.jmpbuf_ptr: %p", (void *) heap->lj.jmpbuf_ptr));
	DUK_D(DUK_DPRINT("  lj.type: %ld", (long) heap->lj.type));
	DUK_D(DUK_DPRINT("  lj.value1: %!T", (duk_tval *) &heap->lj.value1));
	DUK_D(DUK_DPRINT("  lj.value2: %!T", (duk_tval *) &heap->lj.value2));
	DUK_D(DUK_DPRINT("  lj.iserror: %ld", (long) heap->lj.iserror));

	DUK_D(DUK_DPRINT("  handling_error: %ld", (long) heap->handling_error));

	DUK_D(DUK_DPRINT("  heap_thread: %!@O", (duk_heaphdr *) heap->heap_thread));
	DUK_D(DUK_DPRINT("  curr_thread: %!@O", (duk_heaphdr *) heap->curr_thread));
	DUK_D(DUK_DPRINT("  heap_object: %!@O", (duk_heaphdr *) heap->heap_object));

	DUK_D(DUK_DPRINT("  call_recursion_depth: %ld", (long) heap->call_recursion_depth));
	DUK_D(DUK_DPRINT("  call_recursion_limit: %ld", (long) heap->call_recursion_limit));

	DUK_D(DUK_DPRINT("  hash_seed: 0x%08lx", (unsigned long) heap->hash_seed));
	DUK_D(DUK_DPRINT("  rnd_state: 0x%08lx", (unsigned long) heap->rnd_state));

	duk__dump_strcache(heap);

	duk__dump_heaphdr_list(heap, heap->heap_allocated, "heap allocated");

#ifdef DUK_USE_REFERENCE_COUNTING
	duk__dump_heaphdr_list(heap, heap->refzero_list, "refcounting refzero list");
#endif

#ifdef DUK_USE_MARK_AND_SWEEP
	duk__dump_heaphdr_list(heap, heap->finalize_list, "mark-and-sweep finalize list");
#endif

	duk__dump_stringtable(heap);

	/* heap->strs: not worth dumping */
}
#endif

#endif  /* DUK_USE_DEBUG */
