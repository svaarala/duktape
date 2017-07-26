/*
 *  Object property table slot cache.
 */

#include "duk_internal.h"

/* FIXME: check if it's useful to compute the slotcache key only once in property code
 * (live range is not trivial so maybe not).
 */

DUK_LOCAL DUK_ALWAYS_INLINE duk_uint32_t duk__slotcache_compute_key(duk_hobject *obj, duk_hstring *key) {
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);

	return (((duk_uint32_t) obj) ^ DUK_HSTRING_GET_HASH(key)) & (DUK_HEAP_SLOTCACHE_SIZE - 1);
}

DUK_INTERNAL duk_uint32_t duk_heap_slotcache_getkey(duk_hobject *obj, duk_hstring *key) {
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);

	return duk__slotcache_compute_key(obj, key);
}

DUK_INTERNAL duk_uint32_t duk_heap_slotcache_lookup(duk_heap *heap, duk_hobject *obj, duk_hstring *key) {
	duk_uint32_t idx;
	duk_slotcache_entry *ent;

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);

	/* FIXME: assert: hashlimit <= 256 */

	idx = duk__slotcache_compute_key(obj, key);
	ent = heap->slotcache + idx;
	return ent->slot;
}

DUK_INTERNAL void duk_heap_slotcache_insert(duk_heap *heap, duk_hobject *obj, duk_hstring *key, duk_uint32_t slot) {
	duk_uint32_t idx;
	duk_slotcache_entry *ent;

	/* FIXME: assert: hashlimit <= 256 */
	/* FIXME: skip check if hash part present and hashlimit correct */
	if (DUK_UNLIKELY(slot >= DUK_UINT8_MAX)) {
		return;
	}
	idx = duk__slotcache_compute_key(obj, key);
	ent = heap->slotcache + idx;
	ent->slot = (duk_uint16_t) slot;
}
