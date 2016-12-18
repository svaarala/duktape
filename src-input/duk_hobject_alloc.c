/*
 *  Hobject allocation.
 *
 *  Provides primitive allocation functions for all object types (plain object,
 *  compiled function, native function, thread).  The object return is not yet
 *  in "heap allocated" list and has a refcount of zero, so caller must careful.
 */

#include "duk_internal.h"

DUK_LOCAL void duk__init_object_parts(duk_heap *heap, duk_hobject *obj, duk_uint_t hobject_flags) {
#if defined(DUK_USE_EXPLICIT_NULL_INIT)
	DUK_HOBJECT_SET_PROPS(heap, obj, NULL);
#endif

	/* XXX: macro? sets both heaphdr and object flags */
	obj->hdr.h_flags = hobject_flags;
	DUK_HEAPHDR_SET_TYPE(&obj->hdr, DUK_HTYPE_OBJECT);  /* also goes into flags */

#if defined(DUK_USE_HEAPPTR16)
	/* Zero encoded pointer is required to match NULL */
	DUK_HEAPHDR_SET_NEXT(heap, &obj->hdr, NULL);
#if defined(DUK_USE_DOUBLE_LINKED_HEAP)
	DUK_HEAPHDR_SET_PREV(heap, &obj->hdr, NULL);
#endif
#endif
	DUK_ASSERT_HEAPHDR_LINKS(heap, &obj->hdr);
	DUK_HEAP_INSERT_INTO_HEAP_ALLOCATED(heap, &obj->hdr);

	/*
	 *  obj->props is intentionally left as NULL, and duk_hobject_props.c must deal
	 *  with this properly.  This is intentional: empty objects consume a minimum
	 *  amount of memory.  Further, an initial allocation might fail and cause
	 *  'obj' to "leak" (require a mark-and-sweep) since it is not reachable yet.
	 */
}

/*
 *  Allocate an duk_hobject.
 *
 *  The allocated object has no allocation for properties; the caller may
 *  want to force a resize if a desired size is known.
 *
 *  The allocated object has zero reference count and is not reachable.
 *  The caller MUST make the object reachable and increase its reference
 *  count before invoking any operation that might require memory allocation.
 */

DUK_INTERNAL duk_hobject *duk_hobject_alloc(duk_heap *heap, duk_uint_t hobject_flags) {
	duk_hobject *res;

	DUK_ASSERT(heap != NULL);

	/* different memory layout, alloc size, and init */
	DUK_ASSERT((hobject_flags & DUK_HOBJECT_FLAG_COMPFUNC) == 0);
	DUK_ASSERT((hobject_flags & DUK_HOBJECT_FLAG_NATFUNC) == 0);
	DUK_ASSERT((hobject_flags & DUK_HOBJECT_FLAG_THREAD) == 0);

	res = (duk_hobject *) DUK_ALLOC(heap, sizeof(duk_hobject));
	if (!res) {
		return NULL;
	}
	DUK_MEMZERO(res, sizeof(duk_hobject));

	duk__init_object_parts(heap, res, hobject_flags);

	return res;
}

DUK_INTERNAL duk_hcompfunc *duk_hcompfunc_alloc(duk_heap *heap, duk_uint_t hobject_flags) {
	duk_hcompfunc *res;

	res = (duk_hcompfunc *) DUK_ALLOC(heap, sizeof(duk_hcompfunc));
	if (!res) {
		return NULL;
	}
	DUK_MEMZERO(res, sizeof(duk_hcompfunc));

	duk__init_object_parts(heap, &res->obj, hobject_flags);

#if defined(DUK_USE_EXPLICIT_NULL_INIT)
#if defined(DUK_USE_HEAPPTR16)
	/* NULL pointer is required to encode to zero, so memset is enough. */
#else
	res->data = NULL;
	res->funcs = NULL;
	res->bytecode = NULL;
#endif
	res->lex_env = NULL;
	res->var_env = NULL;
#endif

	return res;
}

DUK_INTERNAL duk_hnatfunc *duk_hnatfunc_alloc(duk_heap *heap, duk_uint_t hobject_flags) {
	duk_hnatfunc *res;

	res = (duk_hnatfunc *) DUK_ALLOC(heap, sizeof(duk_hnatfunc));
	if (!res) {
		return NULL;
	}
	DUK_MEMZERO(res, sizeof(duk_hnatfunc));

	duk__init_object_parts(heap, &res->obj, hobject_flags);

#if defined(DUK_USE_EXPLICIT_NULL_INIT)
	res->func = NULL;
#endif

	return res;
}

#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
DUK_INTERNAL duk_hbufobj *duk_hbufobj_alloc(duk_heap *heap, duk_uint_t hobject_flags) {
	duk_hbufobj *res;

	res = (duk_hbufobj *) DUK_ALLOC(heap, sizeof(duk_hbufobj));
	if (!res) {
		return NULL;
	}
	DUK_MEMZERO(res, sizeof(duk_hbufobj));

	duk__init_object_parts(heap, &res->obj, hobject_flags);

#if defined(DUK_USE_EXPLICIT_NULL_INIT)
	res->buf = NULL;
	res->buf_prop = NULL;
#endif

	DUK_ASSERT_HBUFOBJ_VALID(res);
	return res;
}
#endif  /* DUK_USE_BUFFEROBJECT_SUPPORT */

/*
 *  Allocate a new thread.
 *
 *  Leaves the built-ins array uninitialized.  The caller must either
 *  initialize a new global context or share existing built-ins from
 *  another thread.
 */

DUK_INTERNAL duk_hthread *duk_hthread_alloc(duk_heap *heap, duk_uint_t hobject_flags) {
	duk_hthread *res;

	res = (duk_hthread *) DUK_ALLOC(heap, sizeof(duk_hthread));
	if (!res) {
		return NULL;
	}
	DUK_MEMZERO(res, sizeof(duk_hthread));

	duk__init_object_parts(heap, &res->obj, hobject_flags);

#if defined(DUK_USE_EXPLICIT_NULL_INIT)
	res->ptr_curr_pc = NULL;
	res->heap = NULL;
	res->valstack = NULL;
	res->valstack_end = NULL;
	res->valstack_bottom = NULL;
	res->valstack_top = NULL;
	res->callstack = NULL;
	res->catchstack = NULL;
	res->resumer = NULL;
	res->compile_ctx = NULL,
#if defined(DUK_USE_HEAPPTR16)
	res->strs16 = NULL;
#else
	res->strs = NULL;
#endif
	{
		int i;
		for (i = 0; i < DUK_NUM_BUILTINS; i++) {
			res->builtins[i] = NULL;
		}
	}
#endif
	/* when nothing is running, API calls are in non-strict mode */
	DUK_ASSERT(res->strict == 0);

	res->heap = heap;
	res->valstack_max = DUK_VALSTACK_DEFAULT_MAX;
	res->callstack_max = DUK_CALLSTACK_DEFAULT_MAX;
	res->catchstack_max = DUK_CATCHSTACK_DEFAULT_MAX;

	return res;
}

#if 0  /* unused now */
DUK_INTERNAL duk_hobject *duk_hobject_alloc_checked(duk_hthread *thr, duk_uint_t hobject_flags) {
	duk_hobject *res = duk_hobject_alloc(thr->heap, hobject_flags);
	if (!res) {
		DUK_ERROR_ALLOC_FAILED(thr);
	}
	return res;
}
#endif

/*
 *  Allocate a new array.
 */

DUK_INTERNAL duk_harray *duk_harray_alloc(duk_heap *heap, duk_uint_t hobject_flags) {
	duk_harray *res;

	res = (duk_harray *) DUK_ALLOC(heap, sizeof(duk_harray));
	if (!res) {
		return NULL;
	}
	DUK_MEMZERO(res, sizeof(duk_harray));

	duk__init_object_parts(heap, &res->obj, hobject_flags);

	DUK_ASSERT(res->length == 0);

	return res;
}
