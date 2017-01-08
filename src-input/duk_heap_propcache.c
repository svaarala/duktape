#include "duk_internal.h"

DUK_LOCAL DUK_NOINLINE void duk__propcache_scrub(duk_heap *heap) {
	/* Explicit scrub to avoid reuse.  Generation counts are set
	 * to zero, and the generation count is then bumped once more
	 * to one to invalidate the scrubbed entries.
	 */
	DUK_D(DUK_DPRINT("INVALIDATE propcache"));
	DUK_MEMZERO((void *) heap->propcache, sizeof(heap->propcache));
	heap->propcache_generation++;
}

DUK_INTERNAL DUK_ALWAYS_INLINE void duk_propcache_invalidate_heap(duk_heap *heap) {
	if (DUK_UNLIKELY(++heap->propcache_generation == 0)) {
		duk__propcache_scrub(heap);
	}
}

DUK_INTERNAL DUK_ALWAYS_INLINE void duk_propcache_invalidate(duk_hthread *thr) {
	duk_propcache_invalidate_heap(thr->heap);
}

DUK_LOCAL duk_size_t duk__compute_prophash(duk_hobject *obj, duk_hstring *key) {
	duk_size_t prophash;
	prophash = ((duk_size_t) obj) ^ ((duk_size_t) DUK_HSTRING_GET_HASH(key));
	prophash = prophash % DUK_HEAP_PROPCACHE_SIZE;  /* FIXME: force to be a mask */
	return prophash;
}

DUK_INTERNAL duk_tval *duk_propcache_lookup(duk_hthread *thr, duk_hobject *obj, duk_hstring *key) {
	duk_size_t prophash;
	duk_propcache_entry *ent;

	prophash = duk__compute_prophash(obj, key);
	ent = thr->heap->propcache + prophash;

	DUK_D(DUK_DPRINT("lookup, prophash %lu, gen %lu, ent->gen %lu, ent->obj %p, ent->key %p, ent->val %p",
	                 (unsigned long) prophash, (unsigned long) thr->heap->propcache_generation,
	                 (unsigned long) ent->generation, (void *) ent->obj_lookup,
	                 (void *) ent->key_lookup, (void *) ent->val_storage));

	if (ent->generation == thr->heap->propcache_generation &&
	    ent->obj_lookup == obj &&
	    ent->key_lookup == key) {
		return ent->val_storage;  /* Storage location. */
	}

	return NULL;
}

DUK_INTERNAL void duk_propcache_insert(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_tval *tv) {
	duk_size_t prophash;
	duk_propcache_entry *ent;

	prophash = duk__compute_prophash(obj, key);
	ent = thr->heap->propcache + prophash;
	ent->generation = thr->heap->propcache_generation;
	ent->obj_lookup = obj;
	ent->key_lookup = key;
	ent->val_storage = tv;
}
