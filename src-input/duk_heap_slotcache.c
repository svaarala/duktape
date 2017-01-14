/*
 *  Object property table slot cache.
 */

#include "duk_internal.h"

DUK_INTERNAL duk_uint32_t duk_heap_slotcache_lookup(duk_heap *heap, duk_hobject *obj, duk_hstring *key) {
	duk_uint32_t idx;
	duk_slotcache_entry *ent;

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);

	/* FIXME: assert: hashlimit <= 256 */

	idx = ((duk_uint32_t) obj) ^ DUK_HSTRING_GET_HASH(key);
	ent = heap->slotcache + (idx & (DUK_HEAP_SLOTCACHE_SIZE - 1));
	return ent->slot;
}

DUK_INTERNAL void duk_heap_slotcache_insert(duk_heap *heap, duk_hobject *obj, duk_hstring *key, duk_uint32_t slot) {
	duk_uint32_t idx;
	duk_slotcache_entry *ent;

	/* FIXME: assert: hashlimit <= 256 */
	/* FIXME: skip check if hash part present and hashlimit correct */
	if (DUK_UNLIKELY(slot >= DUK_UINT16_MAX)) {
		return;
	}
	idx = ((duk_uint32_t) obj) ^ DUK_HSTRING_GET_HASH(key);
	ent = heap->slotcache + (idx & (DUK_HEAP_SLOTCACHE_SIZE - 1));
	ent->slot = (duk_uint16_t) slot;
}
