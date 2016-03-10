/*
 *  duk_heap allocation and freeing.
 */

#include "duk_internal.h"

/* Constants for built-in string data depacking. */
#define DUK__BITPACK_LETTER_LIMIT  26
#define DUK__BITPACK_UNDERSCORE    26
#define DUK__BITPACK_FF            27
#define DUK__BITPACK_SWITCH1       29
#define DUK__BITPACK_SWITCH        30
#define DUK__BITPACK_SEVENBIT      31

#if defined(DUK_USE_ROM_STRINGS)
/* Fixed seed value used with ROM strings. */
#define DUK__FIXED_HASH_SEED       0xabcd1234
#endif

/*
 *  Free a heap object.
 *
 *  Free heap object and its internal (non-heap) pointers.  Assumes that
 *  caller has removed the object from heap allocated list or the string
 *  intern table, and any weak references (which strings may have) have
 *  been already dealt with.
 */

DUK_INTERNAL void duk_free_hobject_inner(duk_heap *heap, duk_hobject *h) {
	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(h != NULL);

	DUK_FREE(heap, DUK_HOBJECT_GET_PROPS(heap, h));

	if (DUK_HOBJECT_IS_COMPILEDFUNCTION(h)) {
		duk_hcompiledfunction *f = (duk_hcompiledfunction *) h;
		DUK_UNREF(f);
		/* Currently nothing to free; 'data' is a heap object */
	} else if (DUK_HOBJECT_IS_NATIVEFUNCTION(h)) {
		duk_hnativefunction *f = (duk_hnativefunction *) h;
		DUK_UNREF(f);
		/* Currently nothing to free */
	} else if (DUK_HOBJECT_IS_THREAD(h)) {
		duk_hthread *t = (duk_hthread *) h;
		DUK_FREE(heap, t->valstack);
		DUK_FREE(heap, t->callstack);
		DUK_FREE(heap, t->catchstack);
		/* Don't free h->resumer because it exists in the heap.
		 * Callstack entries also contain function pointers which
		 * are not freed for the same reason.
		 */

		/* XXX: with 'caller' property the callstack would need
		 * to be unwound to update the 'caller' properties of
		 * functions in the callstack.
		 */
	}
}

DUK_INTERNAL void duk_free_hbuffer_inner(duk_heap *heap, duk_hbuffer *h) {
	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(h != NULL);

	if (DUK_HBUFFER_HAS_DYNAMIC(h) && !DUK_HBUFFER_HAS_EXTERNAL(h)) {
		duk_hbuffer_dynamic *g = (duk_hbuffer_dynamic *) h;
		DUK_DDD(DUK_DDDPRINT("free dynamic buffer %p", (void *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(heap, g)));
		DUK_FREE(heap, DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(heap, g));
	}
}

DUK_INTERNAL void duk_free_hstring_inner(duk_heap *heap, duk_hstring *h) {
	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(h != NULL);

	DUK_UNREF(heap);
	DUK_UNREF(h);

#if defined(DUK_USE_HSTRING_EXTDATA) && defined(DUK_USE_EXTSTR_FREE)
	if (DUK_HSTRING_HAS_EXTDATA(h)) {
		DUK_DDD(DUK_DDDPRINT("free extstr: hstring %!O, extdata: %p",
		                     h, DUK_HSTRING_GET_EXTDATA((duk_hstring_external *) h)));
		DUK_USE_EXTSTR_FREE(heap->heap_udata, (const void *) DUK_HSTRING_GET_EXTDATA((duk_hstring_external *) h));
	}
#endif
}

DUK_INTERNAL void duk_heap_free_heaphdr_raw(duk_heap *heap, duk_heaphdr *hdr) {
	DUK_ASSERT(heap);
	DUK_ASSERT(hdr);

	DUK_DDD(DUK_DDDPRINT("free heaphdr %p, htype %ld", (void *) hdr, (long) DUK_HEAPHDR_GET_TYPE(hdr)));

	switch ((int) DUK_HEAPHDR_GET_TYPE(hdr)) {
	case DUK_HTYPE_STRING:
		duk_free_hstring_inner(heap, (duk_hstring *) hdr);
		break;
	case DUK_HTYPE_OBJECT:
		duk_free_hobject_inner(heap, (duk_hobject *) hdr);
		break;
	case DUK_HTYPE_BUFFER:
		duk_free_hbuffer_inner(heap, (duk_hbuffer *) hdr);
		break;
	default:
		DUK_UNREACHABLE();
	}

	DUK_FREE(heap, hdr);
}

/*
 *  Free the heap.
 *
 *  Frees heap-related non-heap-tracked allocations such as the
 *  string intern table; then frees the heap allocated objects;
 *  and finally frees the heap structure itself.  Reference counts
 *  and GC markers are ignored (and not updated) in this process,
 *  and finalizers won't be called.
 *
 *  The heap pointer and heap object pointers must not be used
 *  after this call.
 */

DUK_LOCAL void duk__free_allocated(duk_heap *heap) {
	duk_heaphdr *curr;
	duk_heaphdr *next;

	curr = heap->heap_allocated;
	while (curr) {
		/* We don't log or warn about freeing zero refcount objects
		 * because they may happen with finalizer processing.
		 */

		DUK_DDD(DUK_DDDPRINT("FINALFREE (allocated): %!iO",
		                     (duk_heaphdr *) curr));
		next = DUK_HEAPHDR_GET_NEXT(heap, curr);
		duk_heap_free_heaphdr_raw(heap, curr);
		curr = next;
	}
}

#if defined(DUK_USE_REFERENCE_COUNTING)
DUK_LOCAL void duk__free_refzero_list(duk_heap *heap) {
	duk_heaphdr *curr;
	duk_heaphdr *next;

	curr = heap->refzero_list;
	while (curr) {
		DUK_DDD(DUK_DDDPRINT("FINALFREE (refzero_list): %!iO",
		                     (duk_heaphdr *) curr));
		next = DUK_HEAPHDR_GET_NEXT(heap, curr);
		duk_heap_free_heaphdr_raw(heap, curr);
		curr = next;
	}
}
#endif

#if defined(DUK_USE_MARK_AND_SWEEP)
DUK_LOCAL void duk__free_markandsweep_finalize_list(duk_heap *heap) {
	duk_heaphdr *curr;
	duk_heaphdr *next;

	curr = heap->finalize_list;
	while (curr) {
		DUK_DDD(DUK_DDDPRINT("FINALFREE (finalize_list): %!iO",
		                     (duk_heaphdr *) curr));
		next = DUK_HEAPHDR_GET_NEXT(heap, curr);
		duk_heap_free_heaphdr_raw(heap, curr);
		curr = next;
	}
}
#endif

DUK_LOCAL void duk__free_stringtable(duk_heap *heap) {
	/* strings are only tracked by stringtable */
	duk_heap_free_strtab(heap);
}

DUK_LOCAL void duk__free_run_finalizers(duk_heap *heap) {
	duk_hthread *thr;
	duk_heaphdr *curr;
	duk_uint_t round_no;
	duk_size_t count_all;
	duk_size_t count_finalized;
	duk_size_t curr_limit;

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(heap->heap_thread != NULL);

#if defined(DUK_USE_REFERENCE_COUNTING)
	DUK_ASSERT(heap->refzero_list == NULL);  /* refzero not running -> must be empty */
#endif
#if defined(DUK_USE_MARK_AND_SWEEP)
	DUK_ASSERT(heap->finalize_list == NULL);  /* mark-and-sweep not running -> must be empty */
#endif

	/* XXX: here again finalizer thread is the heap_thread which needs
	 * to be coordinated with finalizer thread fixes.
	 */
	thr = heap->heap_thread;
	DUK_ASSERT(thr != NULL);

	/* Prevent mark-and-sweep for the pending finalizers, also prevents
	 * refzero handling from moving objects away from the heap_allocated
	 * list.  (The flag meaning is slightly abused here.)
	 */
	DUK_ASSERT(!DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap));
	DUK_HEAP_SET_MARKANDSWEEP_RUNNING(heap);

	curr_limit = 0;  /* suppress warning, not used */
	for (round_no = 0; ; round_no++) {
		curr = heap->heap_allocated;
		count_all = 0;
		count_finalized = 0;
		while (curr) {
			count_all++;
			if (DUK_HEAPHDR_GET_TYPE(curr) == DUK_HTYPE_OBJECT) {
				/* Only objects in heap_allocated may have finalizers.  Check that
				 * the object itself has a _Finalizer property (own or inherited)
				 * so that we don't execute finalizers for e.g. Proxy objects.
				 */
				DUK_ASSERT(thr != NULL);
				DUK_ASSERT(curr != NULL);

				if (duk_hobject_hasprop_raw(thr, (duk_hobject *) curr, DUK_HTHREAD_STRING_INT_FINALIZER(thr))) {
					if (!DUK_HEAPHDR_HAS_FINALIZED((duk_heaphdr *) curr)) {
						DUK_ASSERT(DUK_HEAP_HAS_FINALIZER_NORESCUE(heap));  /* maps to finalizer 2nd argument */
						duk_hobject_run_finalizer(thr, (duk_hobject *) curr);
						count_finalized++;
					}
				}
			}
			curr = DUK_HEAPHDR_GET_NEXT(heap, curr);
		}

		/* Each round of finalizer execution may spawn new finalizable objects
		 * which is normal behavior for some applications.  Allow multiple
		 * rounds of finalization, but use a shrinking limit based on the
		 * first round to detect the case where a runaway finalizer creates
		 * an unbounded amount of new finalizable objects.  Finalizer rescue
		 * is not supported: the semantics are unclear because most of the
		 * objects being finalized here are already reachable.  The finalizer
		 * is given a boolean to indicate that rescue is not possible.
		 *
		 * See discussion in: https://github.com/svaarala/duktape/pull/473
		 */

		if (round_no == 0) {
			/* Cannot wrap: each object is at least 8 bytes so count is
			 * at most 1/8 of that.
			 */
			curr_limit = count_all * 2;
		} else {
			curr_limit = (curr_limit * 3) / 4;   /* Decrease by 25% every round */
		}
		DUK_D(DUK_DPRINT("finalizer round %ld complete, %ld objects, tried to execute %ld finalizers, current limit is %ld",
		                 (long) round_no, (long) count_all, (long) count_finalized, (long) curr_limit));

		if (count_finalized == 0) {
			DUK_D(DUK_DPRINT("no more finalizable objects, forced finalization finished"));
			break;
		}
		if (count_finalized >= curr_limit) {
			DUK_D(DUK_DPRINT("finalizer count above limit, potentially runaway finalizer; skip remaining finalizers"));
			break;
		}
	}

	DUK_ASSERT(DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap));
	DUK_HEAP_CLEAR_MARKANDSWEEP_RUNNING(heap);
}

DUK_INTERNAL void duk_heap_free(duk_heap *heap) {
	DUK_D(DUK_DPRINT("free heap: %p", (void *) heap));

#if defined(DUK_USE_DEBUG)
	duk_heap_dump_strtab(heap);
#endif

#if defined(DUK_USE_DEBUGGER_SUPPORT)
	/* Detach a debugger if attached (can be called multiple times)
	 * safely.
	 */
	/* XXX: Add a flag to reject an attempt to re-attach?  Otherwise
	 * the detached callback may immediately reattach.
	 */
	duk_debug_do_detach(heap);
#endif

	/* Execute finalizers before freeing the heap, even for reachable
	 * objects, and regardless of whether or not mark-and-sweep is
	 * enabled.  This gives finalizers the chance to free any native
	 * resources like file handles, allocations made outside Duktape,
	 * etc.  This is quite tricky to get right, so that all finalizer
	 * guarantees are honored.
	 *
	 * XXX: this perhaps requires an execution time limit.
	 */
	DUK_D(DUK_DPRINT("execute finalizers before freeing heap"));
#if defined(DUK_USE_MARK_AND_SWEEP)
	/* Run mark-and-sweep a few times just in case (unreachable object
	 * finalizers run already here).  The last round must rescue objects
	 * from the previous round without running any more finalizers.  This
	 * ensures rescued objects get their FINALIZED flag cleared so that
	 * their finalizer is called once more in forced finalization to
	 * satisfy finalizer guarantees.  However, we don't want to run any
	 * more finalizer because that'd required one more loop, and so on.
	 */
	DUK_D(DUK_DPRINT("forced gc #1 in heap destruction"));
	duk_heap_mark_and_sweep(heap, 0);
	DUK_D(DUK_DPRINT("forced gc #2 in heap destruction"));
	duk_heap_mark_and_sweep(heap, 0);
	DUK_D(DUK_DPRINT("forced gc #3 in heap destruction (don't run finalizers)"));
	duk_heap_mark_and_sweep(heap, DUK_MS_FLAG_SKIP_FINALIZERS);  /* skip finalizers; queue finalizable objects to heap_allocated */
#endif

	DUK_HEAP_SET_FINALIZER_NORESCUE(heap);  /* rescue no longer supported */
	duk__free_run_finalizers(heap);

	/* Note: heap->heap_thread, heap->curr_thread, and heap->heap_object
	 * are on the heap allocated list.
	 */

	DUK_D(DUK_DPRINT("freeing heap objects of heap: %p", (void *) heap));
	duk__free_allocated(heap);

#if defined(DUK_USE_REFERENCE_COUNTING)
	DUK_D(DUK_DPRINT("freeing refzero list of heap: %p", (void *) heap));
	duk__free_refzero_list(heap);
#endif

#if defined(DUK_USE_MARK_AND_SWEEP)
	DUK_D(DUK_DPRINT("freeing mark-and-sweep finalize list of heap: %p", (void *) heap));
	duk__free_markandsweep_finalize_list(heap);
#endif

	DUK_D(DUK_DPRINT("freeing string table of heap: %p", (void *) heap));
	duk__free_stringtable(heap);

	DUK_D(DUK_DPRINT("freeing heap structure: %p", (void *) heap));
	heap->free_func(heap->heap_udata, heap);
}

/*
 *  Allocate a heap.
 *
 *  String table is initialized with built-in strings from genbuiltins.py,
 *  either by dynamically creating the strings or by referring to ROM strings.
 */

#if defined(DUK_USE_ROM_STRINGS)
DUK_LOCAL duk_bool_t duk__init_heap_strings(duk_heap *heap) {
#if defined(DUK_USE_ASSERTIONS)
	duk_small_uint_t i;
#endif

	/* With ROM-based strings, heap->strs[] and thr->strs[] are omitted
	 * so nothing to initialize for strs[].
	 */

#if defined(DUK_USE_ASSERTIONS)
	for (i = 0; i < sizeof(duk_rom_strings) / sizeof(const duk_hstring *); i++) {
		duk_uint32_t hash;
		const duk_hstring *h;
		h = duk_rom_strings[i];
		DUK_ASSERT(h != NULL);
		hash = duk_heap_hashstring(heap, (const duk_uint8_t *) DUK_HSTRING_GET_DATA(h), DUK_HSTRING_GET_BYTELEN(h));
		DUK_DD(DUK_DDPRINT("duk_rom_strings[%d] -> hash 0x%08lx, computed 0x%08lx",
		                   (int) i, (unsigned long) DUK_HSTRING_GET_HASH(h), (unsigned long) hash));
		DUK_ASSERT(hash == (duk_uint32_t) DUK_HSTRING_GET_HASH(h));
	}
#endif
	return 1;
}
#else  /* DUK_USE_ROM_STRINGS */
DUK_LOCAL duk_bool_t duk__init_heap_strings(duk_heap *heap) {
	duk_bitdecoder_ctx bd_ctx;
	duk_bitdecoder_ctx *bd = &bd_ctx;  /* convenience */
	duk_small_uint_t i, j;

	DUK_MEMZERO(&bd_ctx, sizeof(bd_ctx));
	bd->data = (const duk_uint8_t *) duk_strings_data;
	bd->length = (duk_size_t) DUK_STRDATA_DATA_LENGTH;

	for (i = 0; i < DUK_HEAP_NUM_STRINGS; i++) {
		duk_uint8_t tmp[DUK_STRDATA_MAX_STRLEN];
		duk_hstring *h;
		duk_small_uint_t len;
		duk_small_uint_t mode;
		duk_small_uint_t t;

		len = duk_bd_decode(bd, 5);
		mode = 32;  /* 0 = uppercase, 32 = lowercase (= 'a' - 'A') */
		for (j = 0; j < len; j++) {
			t = duk_bd_decode(bd, 5);
			if (t < DUK__BITPACK_LETTER_LIMIT) {
				t = t + DUK_ASC_UC_A + mode;
			} else if (t == DUK__BITPACK_UNDERSCORE) {
				t = DUK_ASC_UNDERSCORE;
			} else if (t == DUK__BITPACK_FF) {
				/* Internal keys are prefixed with 0xFF in the stringtable
				 * (which makes them invalid UTF-8 on purpose).
				 */
				t = 0xff;
			} else if (t == DUK__BITPACK_SWITCH1) {
				t = duk_bd_decode(bd, 5);
				DUK_ASSERT_DISABLE(t >= 0);  /* unsigned */
				DUK_ASSERT(t <= 25);
				t = t + DUK_ASC_UC_A + (mode ^ 32);
			} else if (t == DUK__BITPACK_SWITCH) {
				mode = mode ^ 32;
				t = duk_bd_decode(bd, 5);
				DUK_ASSERT_DISABLE(t >= 0);
				DUK_ASSERT(t <= 25);
				t = t + DUK_ASC_UC_A + mode;
			} else if (t == DUK__BITPACK_SEVENBIT) {
				t = duk_bd_decode(bd, 7);
			}
			tmp[j] = (duk_uint8_t) t;
		}

		/* No need to length check string: it will never exceed even
		 * the 16-bit length maximum.
		 */
		DUK_ASSERT(len <= 0xffffUL);
		DUK_DDD(DUK_DDDPRINT("intern built-in string %ld", (long) i));
		h = duk_heap_string_intern(heap, tmp, len);
		if (!h) {
			goto error;
		}
		DUK_ASSERT(!DUK_HEAPHDR_HAS_READONLY((duk_heaphdr *) h));

		/* Special flags checks.  Since these strings are always
		 * reachable and a string cannot appear twice in the string
		 * table, there's no need to check/set these flags elsewhere.
		 * The 'internal' flag is set by string intern code.
		 */
		if (i == DUK_STRIDX_EVAL || i == DUK_STRIDX_LC_ARGUMENTS) {
			DUK_HSTRING_SET_EVAL_OR_ARGUMENTS(h);
		}
		if (i >= DUK_STRIDX_START_RESERVED && i < DUK_STRIDX_END_RESERVED) {
			DUK_HSTRING_SET_RESERVED_WORD(h);
			if (i >= DUK_STRIDX_START_STRICT_RESERVED) {
				DUK_HSTRING_SET_STRICT_RESERVED_WORD(h);
			}
		}

		DUK_DDD(DUK_DDDPRINT("interned: %!O", (duk_heaphdr *) h));

		/* XXX: The incref macro takes a thread pointer but doesn't
		 * use it right now.
		 */
		DUK_HSTRING_INCREF(_never_referenced_, h);

#if defined(DUK_USE_HEAPPTR16)
		heap->strs16[i] = DUK_USE_HEAPPTR_ENC16(heap->heap_udata, (void *) h);
#else
		heap->strs[i] = h;
#endif
	}

	return 1;

 error:
	return 0;
}
#endif  /* DUK_USE_ROM_STRINGS */

DUK_LOCAL duk_bool_t duk__init_heap_thread(duk_heap *heap) {
	duk_hthread *thr;

	DUK_DD(DUK_DDPRINT("heap init: alloc heap thread"));
	thr = duk_hthread_alloc(heap,
	                        DUK_HOBJECT_FLAG_EXTENSIBLE |
	                        DUK_HOBJECT_FLAG_THREAD |
	                        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_THREAD));
	if (!thr) {
		DUK_D(DUK_DPRINT("failed to alloc heap_thread"));
		return 0;
	}
	thr->state = DUK_HTHREAD_STATE_INACTIVE;
#if defined(DUK_USE_ROM_STRINGS)
	/* No strs[] pointer. */
#else  /* DUK_USE_ROM_STRINGS */
#if defined(DUK_USE_HEAPPTR16)
	thr->strs16 = heap->strs16;
#else
	thr->strs = heap->strs;
#endif
#endif  /* DUK_USE_ROM_STRINGS */

	heap->heap_thread = thr;
	DUK_HTHREAD_INCREF(thr, thr);  /* Note: first argument not really used */

	/* 'thr' is now reachable */

	if (!duk_hthread_init_stacks(heap, thr)) {
		return 0;
	}

	/* XXX: this may now fail, and is not handled correctly */
	duk_hthread_create_builtin_objects(thr);

	/* default prototype (Note: 'thr' must be reachable) */
	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, (duk_hobject *) thr, thr->builtins[DUK_BIDX_THREAD_PROTOTYPE]);

	return 1;
}

#if defined(DUK_USE_DEBUG)
#define DUK__DUMPSZ(t)  do { \
		DUK_D(DUK_DPRINT("" #t "=%ld", (long) sizeof(t))); \
	} while (0)

/* These is not 100% because format would need to be non-portable "long long".
 * Also print out as doubles to catch cases where the "long" type is not wide
 * enough; the limits will then not be printed accurately but the magnitude
 * will be correct.
 */
#define DUK__DUMPLM_SIGNED_RAW(t,a,b)  do { \
		DUK_D(DUK_DPRINT(t "=[%ld,%ld]=[%lf,%lf]", \
		                 (long) (a), (long) (b), \
		                 (double) (a), (double) (b))); \
	} while (0)
#define DUK__DUMPLM_UNSIGNED_RAW(t,a,b)  do { \
		DUK_D(DUK_DPRINT(t "=[%lu,%lu]=[%lf,%lf]", \
		                 (unsigned long) (a), (unsigned long) (b), \
		                 (double) (a), (double) (b))); \
	} while (0)
#define DUK__DUMPLM_SIGNED(t)  do { \
		DUK__DUMPLM_SIGNED_RAW("DUK_" #t "_{MIN,MAX}", DUK_##t##_MIN, DUK_##t##_MAX); \
	} while (0)
#define DUK__DUMPLM_UNSIGNED(t)  do { \
		DUK__DUMPLM_UNSIGNED_RAW("DUK_" #t "_{MIN,MAX}", DUK_##t##_MIN, DUK_##t##_MAX); \
	} while (0)

DUK_LOCAL void duk__dump_type_sizes(void) {
	DUK_D(DUK_DPRINT("sizeof()"));

	/* basic platform types */
	DUK__DUMPSZ(char);
	DUK__DUMPSZ(short);
	DUK__DUMPSZ(int);
	DUK__DUMPSZ(long);
	DUK__DUMPSZ(double);
	DUK__DUMPSZ(void *);
	DUK__DUMPSZ(size_t);

	/* basic types from duk_features.h */
	DUK__DUMPSZ(duk_uint8_t);
	DUK__DUMPSZ(duk_int8_t);
	DUK__DUMPSZ(duk_uint16_t);
	DUK__DUMPSZ(duk_int16_t);
	DUK__DUMPSZ(duk_uint32_t);
	DUK__DUMPSZ(duk_int32_t);
	DUK__DUMPSZ(duk_uint64_t);
	DUK__DUMPSZ(duk_int64_t);
	DUK__DUMPSZ(duk_uint_least8_t);
	DUK__DUMPSZ(duk_int_least8_t);
	DUK__DUMPSZ(duk_uint_least16_t);
	DUK__DUMPSZ(duk_int_least16_t);
	DUK__DUMPSZ(duk_uint_least32_t);
	DUK__DUMPSZ(duk_int_least32_t);
#if defined(DUK_USE_64BIT_OPS)
	DUK__DUMPSZ(duk_uint_least64_t);
	DUK__DUMPSZ(duk_int_least64_t);
#endif
	DUK__DUMPSZ(duk_uint_fast8_t);
	DUK__DUMPSZ(duk_int_fast8_t);
	DUK__DUMPSZ(duk_uint_fast16_t);
	DUK__DUMPSZ(duk_int_fast16_t);
	DUK__DUMPSZ(duk_uint_fast32_t);
	DUK__DUMPSZ(duk_int_fast32_t);
#if defined(DUK_USE_64BIT_OPS)
	DUK__DUMPSZ(duk_uint_fast64_t);
	DUK__DUMPSZ(duk_int_fast64_t);
#endif
	DUK__DUMPSZ(duk_uintptr_t);
	DUK__DUMPSZ(duk_intptr_t);
	DUK__DUMPSZ(duk_uintmax_t);
	DUK__DUMPSZ(duk_intmax_t);
	DUK__DUMPSZ(duk_double_t);

	/* important chosen base types */
	DUK__DUMPSZ(duk_int_t);
	DUK__DUMPSZ(duk_uint_t);
	DUK__DUMPSZ(duk_int_fast_t);
	DUK__DUMPSZ(duk_uint_fast_t);
	DUK__DUMPSZ(duk_small_int_t);
	DUK__DUMPSZ(duk_small_uint_t);
	DUK__DUMPSZ(duk_small_int_fast_t);
	DUK__DUMPSZ(duk_small_uint_fast_t);

	/* some derived types */
	DUK__DUMPSZ(duk_codepoint_t);
	DUK__DUMPSZ(duk_ucodepoint_t);
	DUK__DUMPSZ(duk_idx_t);
	DUK__DUMPSZ(duk_errcode_t);
	DUK__DUMPSZ(duk_uarridx_t);

	/* tval */
	DUK__DUMPSZ(duk_double_union);
	DUK__DUMPSZ(duk_tval);

	/* structs from duk_forwdecl.h */
	DUK__DUMPSZ(duk_jmpbuf);  /* just one 'int' for C++ exceptions */
	DUK__DUMPSZ(duk_heaphdr);
	DUK__DUMPSZ(duk_heaphdr_string);
	DUK__DUMPSZ(duk_hstring);
	DUK__DUMPSZ(duk_hstring_external);
	DUK__DUMPSZ(duk_hobject);
	DUK__DUMPSZ(duk_hcompiledfunction);
	DUK__DUMPSZ(duk_hnativefunction);
	DUK__DUMPSZ(duk_hthread);
	DUK__DUMPSZ(duk_hbuffer);
	DUK__DUMPSZ(duk_hbuffer_fixed);
	DUK__DUMPSZ(duk_hbuffer_dynamic);
	DUK__DUMPSZ(duk_hbuffer_external);
	DUK__DUMPSZ(duk_propaccessor);
	DUK__DUMPSZ(duk_propvalue);
	DUK__DUMPSZ(duk_propdesc);
	DUK__DUMPSZ(duk_heap);
#if defined(DUK_USE_STRTAB_CHAIN)
	DUK__DUMPSZ(duk_strtab_entry);
#endif
	DUK__DUMPSZ(duk_activation);
	DUK__DUMPSZ(duk_catcher);
	DUK__DUMPSZ(duk_strcache);
	DUK__DUMPSZ(duk_ljstate);
	DUK__DUMPSZ(duk_fixedbuffer);
	DUK__DUMPSZ(duk_bitdecoder_ctx);
	DUK__DUMPSZ(duk_bitencoder_ctx);
	DUK__DUMPSZ(duk_token);
	DUK__DUMPSZ(duk_re_token);
	DUK__DUMPSZ(duk_lexer_point);
	DUK__DUMPSZ(duk_lexer_ctx);
	DUK__DUMPSZ(duk_compiler_instr);
	DUK__DUMPSZ(duk_compiler_func);
	DUK__DUMPSZ(duk_compiler_ctx);
	DUK__DUMPSZ(duk_re_matcher_ctx);
	DUK__DUMPSZ(duk_re_compiler_ctx);
}
DUK_LOCAL void duk__dump_type_limits(void) {
	DUK_D(DUK_DPRINT("limits"));

	/* basic types */
	DUK__DUMPLM_SIGNED(INT8);
	DUK__DUMPLM_UNSIGNED(UINT8);
	DUK__DUMPLM_SIGNED(INT_FAST8);
	DUK__DUMPLM_UNSIGNED(UINT_FAST8);
	DUK__DUMPLM_SIGNED(INT_LEAST8);
	DUK__DUMPLM_UNSIGNED(UINT_LEAST8);
	DUK__DUMPLM_SIGNED(INT16);
	DUK__DUMPLM_UNSIGNED(UINT16);
	DUK__DUMPLM_SIGNED(INT_FAST16);
	DUK__DUMPLM_UNSIGNED(UINT_FAST16);
	DUK__DUMPLM_SIGNED(INT_LEAST16);
	DUK__DUMPLM_UNSIGNED(UINT_LEAST16);
	DUK__DUMPLM_SIGNED(INT32);
	DUK__DUMPLM_UNSIGNED(UINT32);
	DUK__DUMPLM_SIGNED(INT_FAST32);
	DUK__DUMPLM_UNSIGNED(UINT_FAST32);
	DUK__DUMPLM_SIGNED(INT_LEAST32);
	DUK__DUMPLM_UNSIGNED(UINT_LEAST32);
#if defined(DUK_USE_64BIT_OPS)
	DUK__DUMPLM_SIGNED(INT64);
	DUK__DUMPLM_UNSIGNED(UINT64);
	DUK__DUMPLM_SIGNED(INT_FAST64);
	DUK__DUMPLM_UNSIGNED(UINT_FAST64);
	DUK__DUMPLM_SIGNED(INT_LEAST64);
	DUK__DUMPLM_UNSIGNED(UINT_LEAST64);
#endif
	DUK__DUMPLM_SIGNED(INTPTR);
	DUK__DUMPLM_UNSIGNED(UINTPTR);
	DUK__DUMPLM_SIGNED(INTMAX);
	DUK__DUMPLM_UNSIGNED(UINTMAX);

	/* derived types */
	DUK__DUMPLM_SIGNED(INT);
	DUK__DUMPLM_UNSIGNED(UINT);
	DUK__DUMPLM_SIGNED(INT_FAST);
	DUK__DUMPLM_UNSIGNED(UINT_FAST);
	DUK__DUMPLM_SIGNED(SMALL_INT);
	DUK__DUMPLM_UNSIGNED(SMALL_UINT);
	DUK__DUMPLM_SIGNED(SMALL_INT_FAST);
	DUK__DUMPLM_UNSIGNED(SMALL_UINT_FAST);
}
#undef DUK__DUMPSZ
#undef DUK__DUMPLM_SIGNED_RAW
#undef DUK__DUMPLM_UNSIGNED_RAW
#undef DUK__DUMPLM_SIGNED
#undef DUK__DUMPLM_UNSIGNED

DUK_LOCAL void duk__dump_misc_options(void) {
	DUK_D(DUK_DPRINT("DUK_VERSION: %ld", (long) DUK_VERSION));
	DUK_D(DUK_DPRINT("DUK_GIT_DESCRIBE: %s", DUK_GIT_DESCRIBE));
	DUK_D(DUK_DPRINT("OS string: %s", DUK_USE_OS_STRING));
	DUK_D(DUK_DPRINT("architecture string: %s", DUK_USE_ARCH_STRING));
	DUK_D(DUK_DPRINT("compiler string: %s", DUK_USE_COMPILER_STRING));
#if defined(DUK_USE_PACKED_TVAL)
	DUK_D(DUK_DPRINT("DUK_USE_PACKED_TVAL: yes"));
#else
	DUK_D(DUK_DPRINT("DUK_USE_PACKED_TVAL: no"));
#endif
#if defined(DUK_USE_VARIADIC_MACROS)
	DUK_D(DUK_DPRINT("DUK_USE_VARIADIC_MACROS: yes"));
#else
	DUK_D(DUK_DPRINT("DUK_USE_VARIADIC_MACROS: no"));
#endif
#if defined(DUK_USE_INTEGER_LE)
	DUK_D(DUK_DPRINT("integer endianness: little"));
#elif defined(DUK_USE_INTEGER_ME)
	DUK_D(DUK_DPRINT("integer endianness: mixed"));
#elif defined(DUK_USE_INTEGER_BE)
	DUK_D(DUK_DPRINT("integer endianness: big"));
#else
	DUK_D(DUK_DPRINT("integer endianness: ???"));
#endif
#if defined(DUK_USE_DOUBLE_LE)
	DUK_D(DUK_DPRINT("IEEE double endianness: little"));
#elif defined(DUK_USE_DOUBLE_ME)
	DUK_D(DUK_DPRINT("IEEE double endianness: mixed"));
#elif defined(DUK_USE_DOUBLE_BE)
	DUK_D(DUK_DPRINT("IEEE double endianness: big"));
#else
	DUK_D(DUK_DPRINT("IEEE double endianness: ???"));
#endif
}
#endif  /* DUK_USE_DEBUG */

DUK_INTERNAL
duk_heap *duk_heap_alloc(duk_alloc_function alloc_func,
                         duk_realloc_function realloc_func,
                         duk_free_function free_func,
                         void *heap_udata,
                         duk_fatal_function fatal_func) {
	duk_heap *res = NULL;

	/* Silence a few global unused warnings here. */
	DUK_UNREF(duk_str_unsupported);

	DUK_D(DUK_DPRINT("allocate heap"));

	/*
	 *  Debug dump type sizes
	 */

#if defined(DUK_USE_DEBUG)
	duk__dump_misc_options();
	duk__dump_type_sizes();
	duk__dump_type_limits();
#endif

	/*
	 *  If selftests enabled, run them as early as possible
	 */
#if defined(DUK_USE_SELF_TESTS)
	DUK_D(DUK_DPRINT("running self tests"));
	duk_selftest_run_tests();
	DUK_D(DUK_DPRINT("self tests passed"));
#endif

	/*
	 *  Computed values (e.g. INFINITY)
	 */

#if defined(DUK_USE_COMPUTED_NAN)
	do {
		/* Workaround for some exotic platforms where NAN is missing
		 * and the expression (0.0 / 0.0) does NOT result in a NaN.
		 * Such platforms use the global 'duk_computed_nan' which must
		 * be initialized at runtime.  Use 'volatile' to ensure that
		 * the compiler will actually do the computation and not try
		 * to do constant folding which might result in the original
		 * problem.
		 */
		volatile double dbl1 = 0.0;
		volatile double dbl2 = 0.0;
		duk_computed_nan = dbl1 / dbl2;
	} while (0);
#endif

#if defined(DUK_USE_COMPUTED_INFINITY)
	do {
		/* Similar workaround for INFINITY. */
		volatile double dbl1 = 1.0;
		volatile double dbl2 = 0.0;
		duk_computed_infinity = dbl1 / dbl2;
	} while (0);
#endif

	/*
	 *  Allocate heap struct
	 *
	 *  Use a raw call, all macros expect the heap to be initialized
	 */

	res = (duk_heap *) alloc_func(heap_udata, sizeof(duk_heap));
	if (!res) {
		goto error;
	}

	/*
	 *  Zero the struct, and start initializing roughly in order
	 */

	DUK_MEMZERO(res, sizeof(*res));

	/* explicit NULL inits */
#if defined(DUK_USE_EXPLICIT_NULL_INIT)
	res->heap_udata = NULL;
	res->heap_allocated = NULL;
#if defined(DUK_USE_REFERENCE_COUNTING)
	res->refzero_list = NULL;
	res->refzero_list_tail = NULL;
#endif
#if defined(DUK_USE_MARK_AND_SWEEP)
	res->finalize_list = NULL;
#endif
	res->heap_thread = NULL;
	res->curr_thread = NULL;
	res->heap_object = NULL;
#if defined(DUK_USE_STRTAB_CHAIN)
	/* nothing to NULL */
#elif defined(DUK_USE_STRTAB_PROBE)
#if defined(DUK_USE_HEAPPTR16)
	res->strtable16 = (duk_uint16_t *) NULL;
#else
	res->strtable = (duk_hstring **) NULL;
#endif
#endif
#if defined(DUK_USE_ROM_STRINGS)
	/* no res->strs[] */
#else  /* DUK_USE_ROM_STRINGS */
#if defined(DUK_USE_HEAPPTR16)
	/* res->strs16[] is zeroed and zero decodes to NULL, so no NULL inits. */
#else
	{
		duk_small_uint_t i;
	        for (i = 0; i < DUK_HEAP_NUM_STRINGS; i++) {
			res->strs[i] = NULL;
	        }
	}
#endif
#endif  /* DUK_USE_ROM_STRINGS */
#if defined(DUK_USE_DEBUGGER_SUPPORT)
	res->dbg_read_cb = NULL;
	res->dbg_write_cb = NULL;
	res->dbg_peek_cb = NULL;
	res->dbg_read_flush_cb = NULL;
	res->dbg_write_flush_cb = NULL;
	res->dbg_request_cb = NULL;
	res->dbg_udata = NULL;
	res->dbg_step_thread = NULL;
#endif
#endif  /* DUK_USE_EXPLICIT_NULL_INIT */

	res->alloc_func = alloc_func;
	res->realloc_func = realloc_func;
	res->free_func = free_func;
	res->heap_udata = heap_udata;
	res->fatal_func = fatal_func;

#if defined(DUK_USE_HEAPPTR16)
	/* XXX: zero assumption */
	res->heapptr_null16 = DUK_USE_HEAPPTR_ENC16(res->heap_udata, (void *) NULL);
	res->heapptr_deleted16 = DUK_USE_HEAPPTR_ENC16(res->heap_udata, (void *) DUK_STRTAB_DELETED_MARKER(res));
#endif

	/* res->mark_and_sweep_trigger_counter == 0 -> now causes immediate GC; which is OK */

	res->call_recursion_depth = 0;
	res->call_recursion_limit = DUK_USE_NATIVE_CALL_RECLIMIT;

	/* XXX: use the pointer as a seed for now: mix in time at least */

	/* The casts through duk_intr_pt is to avoid the following GCC warning:
	 *
	 *   warning: cast from pointer to integer of different size [-Wpointer-to-int-cast]
	 *
	 * This still generates a /Wp64 warning on VS2010 when compiling for x86.
	 */
#if defined(DUK_USE_ROM_STRINGS)
	/* XXX: make a common DUK_USE_ option, and allow custom fixed seed? */
	DUK_D(DUK_DPRINT("using rom strings, force heap hash_seed to fixed value 0x%08lx", (long) DUK__FIXED_HASH_SEED));
	res->hash_seed = (duk_uint32_t) DUK__FIXED_HASH_SEED;
#else  /* DUK_USE_ROM_STRINGS */
	res->hash_seed = (duk_uint32_t) (duk_intptr_t) res;
	res->rnd_state = (duk_uint32_t) (duk_intptr_t) res;
#if !defined(DUK_USE_STRHASH_DENSE)
	res->hash_seed ^= 5381;  /* Bernstein hash init value is normally 5381; XOR it in in case pointer low bits are 0 */
#endif
#endif  /* DUK_USE_ROM_STRINGS */

#if defined(DUK_USE_EXPLICIT_NULL_INIT)
	res->lj.jmpbuf_ptr = NULL;
#endif
	DUK_ASSERT(res->lj.type == DUK_LJ_TYPE_UNKNOWN);  /* zero */

	DUK_TVAL_SET_UNDEFINED(&res->lj.value1);
	DUK_TVAL_SET_UNDEFINED(&res->lj.value2);

#if (DUK_STRTAB_INITIAL_SIZE < DUK_UTIL_MIN_HASH_PRIME)
#error initial heap stringtable size is defined incorrectly
#endif

	/*
	 *  Init stringtable: fixed variant
	 */

#if defined(DUK_USE_STRTAB_CHAIN)
	DUK_MEMZERO(res->strtable, sizeof(duk_strtab_entry) * DUK_STRTAB_CHAIN_SIZE);
#if defined(DUK_USE_EXPLICIT_NULL_INIT)
	{
		duk_small_uint_t i;
	        for (i = 0; i < DUK_STRTAB_CHAIN_SIZE; i++) {
#if defined(DUK_USE_HEAPPTR16)
			res->strtable[i].u.str16 = res->heapptr_null16;
#else
			res->strtable[i].u.str = NULL;
#endif
	        }
	}
#endif  /* DUK_USE_EXPLICIT_NULL_INIT */
#endif  /* DUK_USE_STRTAB_CHAIN */

	/*
	 *  Init stringtable: probe variant
	 */

#if defined(DUK_USE_STRTAB_PROBE)
#if defined(DUK_USE_HEAPPTR16)
	res->strtable16 = (duk_uint16_t *) alloc_func(heap_udata, sizeof(duk_uint16_t) * DUK_STRTAB_INITIAL_SIZE);
	if (!res->strtable16) {
		goto error;
	}
#else  /* DUK_USE_HEAPPTR16 */
	res->strtable = (duk_hstring **) alloc_func(heap_udata, sizeof(duk_hstring *) * DUK_STRTAB_INITIAL_SIZE);
	if (!res->strtable) {
		goto error;
	}
#endif  /* DUK_USE_HEAPPTR16 */
	res->st_size = DUK_STRTAB_INITIAL_SIZE;
#if defined(DUK_USE_EXPLICIT_NULL_INIT)
	{
		duk_small_uint_t i;
		DUK_ASSERT(res->st_size == DUK_STRTAB_INITIAL_SIZE);
	        for (i = 0; i < DUK_STRTAB_INITIAL_SIZE; i++) {
#if defined(DUK_USE_HEAPPTR16)
			res->strtable16[i] = res->heapptr_null16;
#else
			res->strtable[i] = NULL;
#endif
	        }
	}
#else  /* DUK_USE_EXPLICIT_NULL_INIT */
#if defined(DUK_USE_HEAPPTR16)
	DUK_MEMZERO(res->strtable16, sizeof(duk_uint16_t) * DUK_STRTAB_INITIAL_SIZE);
#else
	DUK_MEMZERO(res->strtable, sizeof(duk_hstring *) * DUK_STRTAB_INITIAL_SIZE);
#endif
#endif  /* DUK_USE_EXPLICIT_NULL_INIT */
#endif  /* DUK_USE_STRTAB_PROBE */

	/*
	 *  Init stringcache
	 */

#if defined(DUK_USE_EXPLICIT_NULL_INIT)
	{
		duk_small_uint_t i;
		for (i = 0; i < DUK_HEAP_STRCACHE_SIZE; i++) {
			res->strcache[i].h = NULL;
		}
	}
#endif

	/* XXX: error handling is incomplete.  It would be cleanest if
	 * there was a setjmp catchpoint, so that all init code could
	 * freely throw errors.  If that were the case, the return code
	 * passing here could be removed.
	 */

	/*
	 *  Init built-in strings
	 */

	DUK_DD(DUK_DDPRINT("HEAP: INIT STRINGS"));
	if (!duk__init_heap_strings(res)) {
		goto error;
	}

	/*
	 *  Init the heap thread
	 */

	DUK_DD(DUK_DDPRINT("HEAP: INIT HEAP THREAD"));
	if (!duk__init_heap_thread(res)) {
		goto error;
	}

	/*
	 *  Init the heap object
	 */

	DUK_DD(DUK_DDPRINT("HEAP: INIT HEAP OBJECT"));
	DUK_ASSERT(res->heap_thread != NULL);
	res->heap_object = duk_hobject_alloc(res, DUK_HOBJECT_FLAG_EXTENSIBLE |
	                                          DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT));
	if (!res->heap_object) {
		goto error;
	}
	DUK_HOBJECT_INCREF(res->heap_thread, res->heap_object);

	/*
	 *  All done
	 */

	DUK_D(DUK_DPRINT("allocated heap: %p", (void *) res));
	return res;

 error:
	DUK_D(DUK_DPRINT("heap allocation failed"));

	if (res) {
		/* assumes that allocated pointers and alloc funcs are valid
		 * if res exists
		 */
		DUK_ASSERT(res->alloc_func != NULL);
		DUK_ASSERT(res->realloc_func != NULL);
		DUK_ASSERT(res->free_func != NULL);
		duk_heap_free(res);
	}
	return NULL;
}

#undef DUK__BITPACK_LETTER_LIMIT
#undef DUK__BITPACK_UNDERSCORE
#undef DUK__BITPACK_FF
#undef DUK__BITPACK_SWITCH1
#undef DUK__BITPACK_SWITCH
#undef DUK__BITPACK_SEVENBIT
#undef DUK__FIXED_HASH_SEED
