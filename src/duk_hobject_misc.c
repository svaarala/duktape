/*
 *  Misc support functions
 */

#include "duk_internal.h"

duk_bool_t duk_hobject_prototype_chain_contains(duk_hthread *thr, duk_hobject *h, duk_hobject *p) {
	duk_uint_t sanity;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(h != NULL);
	/* allow 'p' to be NULL; then the result is always false */

	sanity = DUK_HOBJECT_PROTOTYPE_CHAIN_SANITY;
	do {
		if (h == p) {
			return 1;
		}

		if (sanity-- == 0) {
			DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, DUK_STR_PROTOTYPE_CHAIN_LIMIT);
		}
		h = h->prototype;
	} while (h);

	return 0;
}

/* FIXME: needed? */
void duk_hobject_set_prototype(duk_hthread *thr, duk_hobject *h, duk_hobject *p) {
#ifdef DUK_USE_REFERENCE_COUNTING
	duk_hobject *tmp;

	DUK_ASSERT(h);
	tmp = h->prototype;
	h->prototype = p;
	DUK_HOBJECT_INCREF(thr, p);  /* avoid problems if p == h->prototype */
	DUK_HOBJECT_DECREF(thr, tmp);
#else
	DUK_ASSERT(h);
	h->prototype = p;
#endif
}
