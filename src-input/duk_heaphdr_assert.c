/*
 *  duk_heaphdr assertion helpers
 */

#include "duk_internal.h"

#if defined(DUK_USE_ASSERTIONS)

#if defined(DUK_USE_DOUBLE_LINKED_HEAP)
DUK_INTERNAL void duk_heaphdr_assert_links(duk_heap *heap, duk_heaphdr *h) {
	DUK_UNREF(heap);
	if (h != NULL) {
		duk_heaphdr *h_prev, *h_next;
		h_prev = DUK_HEAPHDR_GET_PREV(heap, h);
		h_next = DUK_HEAPHDR_GET_NEXT(heap, h);
		DUK_ASSERT(h_prev == NULL || (DUK_HEAPHDR_GET_NEXT(heap, h_prev) == h));
		DUK_ASSERT(h_next == NULL || (DUK_HEAPHDR_GET_PREV(heap, h_next) == h));
	}
}
#else
DUK_INTERNAL void duk_heaphdr_assert_links(duk_heap *heap, duk_heaphdr *h) {
	DUK_UNREF(heap);
	DUK_UNREF(h);
}
#endif

DUK_INTERNAL void duk_heaphdr_assert_valid(duk_heaphdr *h) {
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(h));
}

/* Assert validity of a heaphdr, including all subclasses. */
DUK_INTERNAL void duk_heaphdr_assert_valid_subclassed(duk_heap *heap, duk_heaphdr *h) {
	if (DUK_HEAPHDR_IS_ANY_OBJECT(h)) {
		DUK_HOBJECT_ASSERT_VALID(heap, (duk_hobject *) h);
	}

	switch (DUK_HEAPHDR_GET_HTYPE(h)) {
	case DUK_HTYPE_STRING_INTERNAL:
	case DUK_HTYPE_STRING_EXTERNAL: {
		duk_hstring *h_str = (duk_hstring *) h;
		DUK_HSTRING_ASSERT_VALID(h_str);
		break;
	}
	case DUK_HTYPE_BUFFER_FIXED:
	case DUK_HTYPE_BUFFER_DYNAMIC:
	case DUK_HTYPE_BUFFER_EXTERNAL: {
		duk_hbuffer *h_buf = (duk_hbuffer *) h;
		DUK_HBUFFER_ASSERT_VALID(h_buf);
		break;
	}
	case DUK_HTYPE_COMPFUNC: {
		DUK_HCOMPFUNC_ASSERT_VALID((duk_hcompfunc *) h);
		break;
	}
	case DUK_HTYPE_NATFUNC: {
		DUK_HNATFUNC_ASSERT_VALID((duk_hnatfunc *) h);
		break;
	}
	case DUK_HTYPE_BOUNDFUNC: {
		DUK_HBOUNDFUNC_ASSERT_VALID((duk_hboundfunc *) h);
		break;
	}
	case DUK_HTYPE_DECENV: {
		DUK_HDECENV_ASSERT_VALID((duk_hdecenv *) h);
		break;
	}
	case DUK_HTYPE_OBJENV: {
		DUK_HOBJENV_ASSERT_VALID((duk_hobjenv *) h);
		break;
	}
	case DUK_HTYPE_PROXY: {
		DUK_HPROXY_ASSERT_VALID((duk_hproxy *) h);
		break;
	}
	case DUK_HTYPE_THREAD: {
		DUK_HTHREAD_ASSERT_VALID((duk_hthread *) h);
		break;
	}
	default: {
	}
	}

	if (DUK_HEAPHDR_IS_ANY_BUFOBJ(h)) {
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
		DUK_HBUFOBJ_ASSERT_VALID((duk_hbufobj *) h);
#endif
	}
}

#endif /* DUK_USE_ASSERTIONS */
