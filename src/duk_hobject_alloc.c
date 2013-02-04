/*
 *  Hobject allocation.
 *
 *  Provides primitive allocation functions for all object types (plain object,
 *  compiled function, native function, thread).  The object return is not yet
 *  in "heap allocated" list and has a refcount of zero, so caller must careful.
 */

#include "duk_internal.h"

static void init_object_parts(duk_heap *heap, duk_hobject *obj, int hobject_flags) {
#ifdef DUK_USE_EXPLICIT_NULL_INIT
	obj->p = NULL;
#endif

	/* FIXME: macro? sets both heaphdr and object flags */
	obj->hdr.h_flags = hobject_flags;
	DUK_HEAPHDR_SET_TYPE(&obj->hdr, DUK_HTYPE_OBJECT);  /* also goes into flags */

        DUK_HEAP_INSERT_INTO_HEAP_ALLOCATED(heap, &obj->hdr);

	/*
	 *  obj->p is intentionally left as NULL, and duk_hobject_props.c must deal
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

duk_hobject *duk_hobject_alloc(duk_heap *heap, int hobject_flags) {
	duk_hobject *res;

	DUK_ASSERT(heap != NULL);

	/* different memory layout, alloc size, and init */
	DUK_ASSERT((hobject_flags & DUK_HOBJECT_FLAG_COMPILEDFUNCTION) == 0);
	DUK_ASSERT((hobject_flags & DUK_HOBJECT_FLAG_NATIVEFUNCTION) == 0);
	DUK_ASSERT((hobject_flags & DUK_HOBJECT_FLAG_THREAD) == 0);

	res = DUK_ALLOC(heap, sizeof(duk_hobject));
	if (!res) {
		return NULL;
	}
	memset(res, 0, sizeof(duk_hobject));

	init_object_parts(heap, res, hobject_flags);

	return res;
}

duk_hcompiledfunction *duk_hcompiledfunction_alloc(duk_heap *heap, int hobject_flags) {
	duk_hcompiledfunction *res;

	res = DUK_ALLOC(heap, sizeof(duk_hcompiledfunction));
	if (!res) {
		return NULL;
	}
	memset(res, 0, sizeof(duk_hcompiledfunction));

	init_object_parts(heap, &res->obj, hobject_flags);

#ifdef DUK_USE_EXPLICIT_NULL_INIT
	res->data = NULL;
	res->funcs = NULL;
	res->bytecode = NULL;
#endif

	return res;
}

duk_hnativefunction *duk_hnativefunction_alloc(duk_heap *heap, int hobject_flags) {
	duk_hnativefunction *res;

	res = DUK_ALLOC(heap, sizeof(duk_hnativefunction));
	if (!res) {
		return NULL;
	}
	memset(res, 0, sizeof(duk_hnativefunction));

	init_object_parts(heap, &res->obj, hobject_flags);

#ifdef DUK_USE_EXPLICIT_NULL_INIT
	res->func = NULL;
#endif

	return res;
}

/*
 *  Allocate a new thread.
 *
 *  Leaves the built-ins array uninitialized.  The caller must either
 *  initialize a new global context or share existing built-ins from
 *  another thread.
 */

duk_hthread *duk_hthread_alloc(duk_heap *heap, int hobject_flags) {
	duk_hthread *res;

	res = DUK_ALLOC(heap, sizeof(duk_hthread));
	if (!res) {
		return NULL;
	}
	memset(res, 0, sizeof(duk_hthread));

	init_object_parts(heap, &res->obj, hobject_flags);

#ifdef DUK_USE_EXPLICIT_NULL_INIT
	res->heap = NULL;
	res->valstack = NULL;
	res->valstack_end = NULL;
	res->valstack_bottom = NULL;
	res->valstack_top = NULL;
	res->callstack = NULL;
	res->catchstack = NULL;
	res->resumer = NULL;
	res->strs = NULL;
	{
		int i;
		for (i = 0; i < DUK_NUM_BUILTINS; i++) {
			res->builtins[i] = NULL;
		}
	}
#endif

	res->heap = heap;
	res->valstack_max = DUK_VALSTACK_DEFAULT_MAX;
	res->callstack_max = DUK_CALLSTACK_DEFAULT_MAX;
	res->catchstack_max = DUK_CATCHSTACK_DEFAULT_MAX;

	return res;
}

/* FIXME: unused now, remove */
duk_hobject *duk_hobject_alloc_checked(duk_hthread *thr, int hobject_flags) {
	duk_hobject *res = duk_hobject_alloc(thr->heap, hobject_flags);
	if (!res) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, "failed to allocate an object");
	}
	return res;
}

