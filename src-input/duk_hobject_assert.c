/*
 *  duk_hobject and subclass assertion helpers
 */

#include "duk_internal.h"

#if defined(DUK_USE_ASSERTIONS)

DUK_INTERNAL void duk_hobject_assert_valid(duk_heap *heap, duk_hobject *h) {
	duk_uint32_t i, j, n;
	duk_hstring **keys;

	DUK_UNREF(heap);
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(!DUK_HOBJECT_IS_CALLABLE(h) ||
	           (DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_COMPFUNC || DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_NATFUNC ||
	            DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_BOUNDFUNC || DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_PROXY));
	DUK_ASSERT(!DUK_HOBJECT_IS_BUFOBJ(h) ||
	           (DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_ARRAYBUFFER || DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_DATAVIEW ||
	            DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_INT8ARRAY || DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_UINT8ARRAY ||
	            DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_UINT8CLAMPEDARRAY || DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_INT16ARRAY ||
	            DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_UINT16ARRAY || DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_INT32ARRAY ||
	            DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_UINT32ARRAY || DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_FLOAT32ARRAY ||
	            DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_FLOAT64ARRAY));

	/* Object is an Array <=> object has exotic array behavior (=> duk_harray layout) */
	DUK_ASSERT((DUK_HOBJECT_GET_HTYPE(h) == DUK_HTYPE_ARRAY && DUK_HOBJECT_HAS_EXOTIC_ARRAY(h)) ||
	           (DUK_HOBJECT_GET_HTYPE(h) != DUK_HTYPE_ARRAY && !DUK_HOBJECT_HAS_EXOTIC_ARRAY(h)));

	/* No duplicate keys. */
	n = DUK_HOBJECT_GET_ENEXT(h);
	keys = DUK_HOBJECT_E_GET_KEY_BASE(heap, h);
	for (i = 0; i < n; i++) {
		duk_hstring *k1 = keys[i];
		if (k1 == NULL) {
			continue;
		}
		for (j = i + 1; j < n; j++) {
			duk_hstring *k2 = keys[j];
			if (k2 == NULL) {
				continue;
			}
			DUK_ASSERT(k1 != k2);
		}
	}

	/* Keys in string property part must never be arridx keys. */
	n = DUK_HOBJECT_GET_ENEXT(h);
	keys = DUK_HOBJECT_E_GET_KEY_BASE(heap, h);
	for (i = 0; i < n; i++) {
		duk_hstring *k1 = keys[i];
		if (k1 == NULL) {
			continue;
		}
		DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(k1));
	}

	/* If an object has a linear array items part, it must not have an
	 * index key part.
	 */
	if (DUK_HOBJECT_HAS_ARRAY_ITEMS(h)) {
		DUK_ASSERT(h->idx_props == NULL);
		DUK_ASSERT(h->idx_hash == NULL);
		DUK_ASSERT(h->i_size == 0);
		DUK_ASSERT(h->i_next == 0);
	}
}

DUK_INTERNAL void duk_hobject_assert_compact(duk_heap *heap, duk_hobject *h) {
	duk_uint32_t i, n;
	duk_hstring **keys;

	DUK_HOBJECT_ASSERT_VALID(heap, h);

	n = DUK_HOBJECT_GET_ENEXT(h);
	keys = DUK_HOBJECT_E_GET_KEY_BASE(heap, h);
	for (i = 0; i < n; i++) {
		duk_hstring *k1 = keys[i];
		DUK_ASSERT(k1 != NULL);
	}

	if (DUK_HOBJECT_IS_HARRAY(h)) {
		duk_harray *a = (duk_harray *) h;

		/* Compactness for 'items' doesn't mean there are no gaps,
		 * but that the 'items' part has no trailing unused values.
		 */
		if (DUK_HARRAY_GET_ITEMS(heap, a) != NULL && DUK_HARRAY_GET_ITEMS_LENGTH(a) > 0) {
			duk_tval *tv = DUK_HARRAY_GET_ITEMS(heap, a) + DUK_HARRAY_GET_ITEMS_LENGTH(a) - 1;
			DUK_ASSERT(!DUK_TVAL_IS_UNUSED(tv));
		}
	}
}

DUK_INTERNAL void duk_hobject_assert_key_absent(duk_heap *heap, duk_hobject *h, duk_hstring *key) {
	duk_uint32_t i, n;
	duk_hstring **keys;

	DUK_HOBJECT_ASSERT_VALID(heap, h);

	n = DUK_HOBJECT_GET_ENEXT(h);
	keys = DUK_HOBJECT_E_GET_KEY_BASE(heap, h);
	for (i = 0; i < n; i++) {
		duk_hstring *k = keys[i];
		if (k == NULL) {
			continue;
		}
		DUK_ASSERT(k != key);
	}
}

DUK_INTERNAL void duk_harray_assert_valid(duk_heap *heap, duk_harray *h) {
	DUK_UNREF(heap);
	DUK_ASSERT(h != NULL);
	if (DUK_HOBJECT_IS_ARRAY((duk_hobject *) h)) {
		DUK_ASSERT(DUK_HOBJECT_HAS_EXOTIC_ARRAY((duk_hobject *) h));
	} else if (DUK_HOBJECT_IS_ARGUMENTS((duk_hobject *) h)) {
		/* Not necessarily Arguments exotic. */
	} else {
		DUK_ASSERT(0);
	}
	if (DUK_HARRAY_GET_ITEMS(heap, h) != NULL) {
		DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS((duk_hobject *) h));
	}
	if (DUK_HARRAY_GET_ITEMS(heap, h) == NULL) {
		DUK_ASSERT(DUK_HARRAY_GET_ITEMS_LENGTH(h) == 0);
	}

	/* Array length doesn't relate directly to items_length, but for
	 * actual Arrays, any value at index >= .length must be unused.
	 * For Arguments objects this isn't necessarily the case.
	 */
	if (DUK_HOBJECT_IS_ARRAY((duk_hobject *) h) && DUK_HOBJECT_HAS_ARRAY_ITEMS((duk_hobject *) h)) {
		duk_uint32_t i;
		for (i = DUK_HARRAY_GET_LENGTH(h); i < DUK_HARRAY_GET_ITEMS_LENGTH(h); i++) {
			DUK_ASSERT(DUK_HARRAY_GET_ITEMS(heap, h) != NULL);
			DUK_ASSERT(DUK_TVAL_IS_UNUSED(DUK_HARRAY_GET_ITEMS(heap, h) + i));
		}
	}

	if (DUK_HOBJECT_HAS_ARRAY_ITEMS((duk_hobject *) h)) {
		DUK_ASSERT(((duk_hobject *) h)->idx_props == NULL);
		DUK_ASSERT(((duk_hobject *) h)->idx_hash == NULL);
		DUK_ASSERT(((duk_hobject *) h)->i_size == 0);
		DUK_ASSERT(((duk_hobject *) h)->i_next == 0);
	}
}

DUK_INTERNAL void duk_hboundfunc_assert_valid(duk_hboundfunc *h) {
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(DUK_HOBJECT_IS_BOUNDFUNC((duk_hobject *) h));
	DUK_ASSERT(DUK_TVAL_IS_LIGHTFUNC(&h->target) ||
	           (DUK_TVAL_IS_OBJECT(&h->target) && DUK_HOBJECT_IS_CALLABLE(DUK_TVAL_GET_OBJECT(&h->target))));
	DUK_ASSERT(!DUK_TVAL_IS_UNUSED(&h->this_binding));
	DUK_ASSERT(h->nargs == 0 || h->args != NULL);
}

#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
DUK_INTERNAL void duk_hbufobj_assert_valid(duk_hbufobj *h) {
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(h->shift <= 3);
	DUK_ASSERT(h->elem_type <= DUK_HBUFOBJ_ELEM_MAX);
	DUK_ASSERT((h->shift == 0 && h->elem_type == DUK_HBUFOBJ_ELEM_UINT8) ||
	           (h->shift == 0 && h->elem_type == DUK_HBUFOBJ_ELEM_UINT8CLAMPED) ||
	           (h->shift == 0 && h->elem_type == DUK_HBUFOBJ_ELEM_INT8) ||
	           (h->shift == 1 && h->elem_type == DUK_HBUFOBJ_ELEM_UINT16) ||
	           (h->shift == 1 && h->elem_type == DUK_HBUFOBJ_ELEM_INT16) ||
	           (h->shift == 2 && h->elem_type == DUK_HBUFOBJ_ELEM_UINT32) ||
	           (h->shift == 2 && h->elem_type == DUK_HBUFOBJ_ELEM_INT32) ||
	           (h->shift == 2 && h->elem_type == DUK_HBUFOBJ_ELEM_FLOAT32) ||
	           (h->shift == 3 && h->elem_type == DUK_HBUFOBJ_ELEM_FLOAT64));
	DUK_ASSERT(h->is_typedarray == 0 || h->is_typedarray == 1);
	DUK_ASSERT(DUK_HOBJECT_IS_BUFOBJ((duk_hobject *) h));
	if (h->buf == NULL) {
		DUK_ASSERT(h->offset == 0);
		DUK_ASSERT(h->length == 0);
	} else {
		/* No assertions for offset or length; in particular,
		 * it's OK for length to be longer than underlying
		 * buffer.  Just ensure they don't wrap when added.
		 */
		DUK_ASSERT(h->offset + h->length >= h->offset);
	}
}
#endif /* DUK_USE_BUFFEROBJECT_SUPPORT */

DUK_INTERNAL void duk_hcompfunc_assert_valid(duk_hcompfunc *h) {
	DUK_ASSERT(h != NULL);
}

DUK_INTERNAL void duk_hnatfunc_assert_valid(duk_hnatfunc *h) {
	DUK_ASSERT(h != NULL);
}

DUK_INTERNAL void duk_hdecenv_assert_valid(duk_hdecenv *h) {
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(DUK_HOBJECT_IS_DECENV((duk_hobject *) h));
	DUK_ASSERT(h->thread == NULL || h->varmap != NULL);
}

DUK_INTERNAL void duk_hobjenv_assert_valid(duk_hobjenv *h) {
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(DUK_HOBJECT_IS_OBJENV((duk_hobject *) h));
	DUK_ASSERT(h->target != NULL);
	DUK_ASSERT(h->has_this == 0 || h->has_this == 1);
}

DUK_INTERNAL void duk_hproxy_assert_valid(duk_hproxy *h) {
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(h->target != NULL);
	DUK_ASSERT(h->handler != NULL);
	DUK_ASSERT(DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ((duk_hobject *) h));
}

DUK_INTERNAL void duk_hthread_assert_valid(duk_hthread *thr) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) thr) == DUK_HTYPE_THREAD);
	DUK_ASSERT(DUK_HOBJECT_IS_THREAD((duk_hobject *) thr));
	DUK_ASSERT(thr->unused1 == 0);
	DUK_ASSERT(thr->unused2 == 0);
}

DUK_INTERNAL void duk_ctx_assert_valid(duk_hthread *thr) {
	DUK_ASSERT(thr != NULL);
	DUK_HTHREAD_ASSERT_VALID(thr);
	DUK_ASSERT(thr->valstack != NULL);
	DUK_ASSERT(thr->valstack_bottom != NULL);
	DUK_ASSERT(thr->valstack_top != NULL);
	DUK_ASSERT(thr->valstack_end != NULL);
	DUK_ASSERT(thr->valstack_alloc_end != NULL);
	DUK_ASSERT(thr->valstack_alloc_end >= thr->valstack);
	DUK_ASSERT(thr->valstack_end >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);
	DUK_ASSERT(thr->valstack_alloc_end >= thr->valstack_end);
}

#endif /* DUK_USE_ASSERTIONS */
