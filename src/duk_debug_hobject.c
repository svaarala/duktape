/*
 *  Debug dumping of duk_hobject.
 */

#include "duk_internal.h"

#ifdef DUK_USE_DEBUG

/* must match duk_hobject.h */
DUK_LOCAL const char *duk__class_names[32] = {
	"unused",
	"Arguments",
	"Array",
	"Boolean",
	"Date",
	"Error",
	"Function",
	"JSON",
	"Math",
	"Number",
	"Object",
	"RegExp",
	"String",
	"global",
	"ObjEnv",
	"DecEnv",
	"Buffer",
	"Pointer",
	"unused",
	"unused",
	"unused",
	"unused",
	"unused",
	"unused",
	"unused",
	"unused",
	"unused",
	"unused",
	"unused",
	"unused",
	"unused",
	"unused",
};

/* for thread dumping */
DUK_LOCAL char duk__get_act_summary_char(duk_activation *act) {
	if (act->func) {
		if (DUK_HOBJECT_IS_COMPILEDFUNCTION(act->func)) {
			return 'c';
		} else if (DUK_HOBJECT_IS_NATIVEFUNCTION(act->func)) {
			return 'n';
		} else {
			/* should not happen */
			return '?';
		}
	} else {
		/* should not happen */
		return '?';
	}
}

/* for thread dumping */
DUK_LOCAL char duk__get_tval_summary_char(duk_tval *tv) {
	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED:
		if (DUK_TVAL_IS_UNDEFINED_UNUSED(tv)) {
			return '.';
		}
		return 'u';
	case DUK_TAG_NULL:
		return 'n';
	case DUK_TAG_BOOLEAN:
		return 'b';
	case DUK_TAG_STRING:
		return 's';
	case DUK_TAG_OBJECT: {
		duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);

		if (DUK_HOBJECT_IS_ARRAY(h)) {
			return 'A';
		} else if (DUK_HOBJECT_IS_COMPILEDFUNCTION(h)) {
			return 'C';
		} else if (DUK_HOBJECT_IS_NATIVEFUNCTION(h)) {
			return 'N';
		} else if (DUK_HOBJECT_IS_THREAD(h)) {
			return 'T';
		}
		return 'O';
	}
	case DUK_TAG_BUFFER: {
		return 'B';
	}
	case DUK_TAG_POINTER: {
		return 'P';
	}
	default:
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		return 'd';
	}

	DUK_UNREACHABLE();
}

/* for thread dumping */
DUK_LOCAL char duk__get_cat_summary_char(duk_catcher *catcher) {
	switch (DUK_CAT_GET_TYPE(catcher)) {
	case DUK_CAT_TYPE_TCF:
		if (DUK_CAT_HAS_CATCH_ENABLED(catcher)) {
			if (DUK_CAT_HAS_FINALLY_ENABLED(catcher)) {
				return 'C';  /* catch and finally active */
			} else {
				return 'c';  /* only catch active */
			}
		} else {
			if (DUK_CAT_HAS_FINALLY_ENABLED(catcher)) {
				return 'f';  /* only finally active */
			} else {
				return 'w';  /* neither active (usually 'with') */
			}
		}
	case DUK_CAT_TYPE_LABEL:
		return 'l';
	case DUK_CAT_TYPE_UNKNOWN:
	default:
		return '?';
	}

	DUK_UNREACHABLE();
}

DUK_INTERNAL void duk_debug_dump_hobject(duk_hobject *obj) {
	duk_uint_fast32_t i;
	const char *str_empty = "";
	const char *str_excl = "!";

	DUK_UNREF(str_empty);
	DUK_UNREF(str_excl);
	DUK_UNREF(duk__class_names);

	DUK_D(DUK_DPRINT("=== hobject %p ===", (void *) obj));
	if (!obj) {
		return;
	}

	DUK_D(DUK_DPRINT("  %sextensible", (const char *) (DUK_HOBJECT_HAS_EXTENSIBLE(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %sconstructable", (const char *) (DUK_HOBJECT_HAS_CONSTRUCTABLE(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %sbound", (const char *) (DUK_HOBJECT_HAS_BOUND(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %scompiledfunction", (const char *) (DUK_HOBJECT_HAS_COMPILEDFUNCTION(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %snativefunction", (const char *) (DUK_HOBJECT_HAS_NATIVEFUNCTION(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %sthread", (const char *) (DUK_HOBJECT_HAS_THREAD(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %sarray_part", (const char *) (DUK_HOBJECT_HAS_ARRAY_PART(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %sstrict", (const char *) (DUK_HOBJECT_HAS_STRICT(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %snewenv", (const char *) (DUK_HOBJECT_HAS_NEWENV(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %snamebinding", (const char *) (DUK_HOBJECT_HAS_NAMEBINDING(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %screateargs", (const char *) (DUK_HOBJECT_HAS_CREATEARGS(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %senvrecclosed", (const char *) (DUK_HOBJECT_HAS_ENVRECCLOSED(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %sexotic_array", (const char *) (DUK_HOBJECT_HAS_EXOTIC_ARRAY(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %sexotic_stringobj", (const char *) (DUK_HOBJECT_HAS_EXOTIC_STRINGOBJ(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %sexotic_arguments", (const char *) (DUK_HOBJECT_HAS_EXOTIC_ARGUMENTS(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %sexotic_dukfunc", (const char *) (DUK_HOBJECT_HAS_EXOTIC_DUKFUNC(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %sexotic_bufferobj", (const char *) (DUK_HOBJECT_HAS_EXOTIC_BUFFEROBJ(obj) ? str_empty : str_excl)));
	DUK_D(DUK_DPRINT("  %sexotic_proxyobj", (const char *) (DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(obj) ? str_empty : str_excl)));

	DUK_D(DUK_DPRINT("  class: number %ld -> %s",
	                 (long) DUK_HOBJECT_GET_CLASS_NUMBER(obj),
	                 (const char *) (duk__class_names[(DUK_HOBJECT_GET_CLASS_NUMBER(obj)) & ((1 << DUK_HOBJECT_FLAG_CLASS_BITS) - 1)])));

	DUK_D(DUK_DPRINT("  prototype: %p -> %!O",
	                 (void *) obj->prototype,
	                 (duk_heaphdr *) obj->prototype));

	DUK_D(DUK_DPRINT("  props: p=%p, e_size=%ld, e_next=%ld, a_size=%ld, h_size=%ld",
	                 (void *) obj->p,
	                 (long) obj->e_size,
	                 (long) obj->e_next,
	                 (long) obj->a_size,
	                 (long) obj->h_size));

	/*
	 *  Object (struct layout) specific dumping.  Inline code here
	 *  instead of helpers, to ensure debug line prefix is identical.
	 */

	if (DUK_HOBJECT_IS_COMPILEDFUNCTION(obj)) {
		duk_hcompiledfunction *h = (duk_hcompiledfunction *) obj;

		DUK_D(DUK_DPRINT("  hcompiledfunction"));
		DUK_D(DUK_DPRINT("  data: %!O", (duk_heaphdr *) h->data));
		DUK_D(DUK_DPRINT("  nregs: %ld", (long) h->nregs));
		DUK_D(DUK_DPRINT("  nargs: %ld", (long) h->nargs));

		if (h->data && DUK_HBUFFER_HAS_DYNAMIC(h->data) && DUK_HBUFFER_GET_DATA_PTR(h->data)) {
			DUK_D(DUK_DPRINT("  consts: %p (%ld, %ld bytes)",
			                 (void *) DUK_HCOMPILEDFUNCTION_GET_CONSTS_BASE(h),
			                 (long) DUK_HCOMPILEDFUNCTION_GET_CONSTS_COUNT(h),
			                 (long) DUK_HCOMPILEDFUNCTION_GET_CONSTS_SIZE(h)));
			DUK_D(DUK_DPRINT("  funcs: %p (%ld, %ld bytes)",
			                 (void *) DUK_HCOMPILEDFUNCTION_GET_FUNCS_BASE(h),
			                 (long) DUK_HCOMPILEDFUNCTION_GET_FUNCS_COUNT(h),
			                 (long) DUK_HCOMPILEDFUNCTION_GET_FUNCS_SIZE(h)));
			DUK_D(DUK_DPRINT("  bytecode: %p (%ld, %ld bytes)",
			                 (void *) DUK_HCOMPILEDFUNCTION_GET_CODE_BASE(h),
			                 (long) DUK_HCOMPILEDFUNCTION_GET_CODE_COUNT(h),
			                 (long) DUK_HCOMPILEDFUNCTION_GET_CODE_SIZE(h)));
		} else {
			DUK_D(DUK_DPRINT("  consts: ???"));
			DUK_D(DUK_DPRINT("  funcs: ???"));
			DUK_D(DUK_DPRINT("  bytecode: ???"));
		}
	} else if (DUK_HOBJECT_IS_NATIVEFUNCTION(obj)) {
		duk_hnativefunction *h = (duk_hnativefunction *) obj;
		DUK_UNREF(h);

		DUK_D(DUK_DPRINT("  hnativefunction"));
		/* XXX: h->func, cannot print function pointers portably */
		DUK_D(DUK_DPRINT("  nargs: %ld", (long) h->nargs));
	} else if (DUK_HOBJECT_IS_THREAD(obj)) {
		duk_hthread *thr = (duk_hthread *) obj;
		duk_tval *p;

		DUK_D(DUK_DPRINT("  hthread"));
		DUK_D(DUK_DPRINT("  strict: %ld", (long) thr->strict));
		DUK_D(DUK_DPRINT("  state: %ld", (long) thr->state));

		DUK_D(DUK_DPRINT("  valstack_max: %ld, callstack_max: %ld, catchstack_max: %ld",
		                 (long) thr->valstack_max, (long) thr->callstack_max, (long) thr->catchstack_max));

		DUK_D(DUK_DPRINT("  callstack: ptr %p, size %ld, top %ld, preventcount %ld, used size %ld entries (%ld bytes), alloc size %ld entries (%ld bytes)",
		                 (void *) thr->callstack,
		                 (long) thr->callstack_size,
		                 (long) thr->callstack_top,
		                 (long) thr->callstack_preventcount,
		                 (long) thr->callstack_top,
		                 (long) (thr->callstack_top * sizeof(duk_activation)),
		                 (long) thr->callstack_size,
		                 (long) (thr->callstack_size * sizeof(duk_activation))));

		DUK_DEBUG_SUMMARY_INIT();
		DUK_DEBUG_SUMMARY_CHAR('[');
		for (i = 0; i <= (duk_uint_fast32_t) thr->callstack_size; i++) {
			if (i == thr->callstack_top) {
				DUK_DEBUG_SUMMARY_CHAR('|');
			}
			if (!thr->callstack) {
				DUK_DEBUG_SUMMARY_CHAR('@');
			} else if (i < thr->callstack_size) {
				if (i < thr->callstack_top) {
					/* tailcalling is nice to see immediately; other flags (e.g. strict)
					 * not that important.
					 */
					if (thr->callstack[i].flags & DUK_ACT_FLAG_TAILCALLED) {
						DUK_DEBUG_SUMMARY_CHAR('/');
					}
					DUK_DEBUG_SUMMARY_CHAR(duk__get_act_summary_char(thr->callstack + i));
				} else {
					DUK_DEBUG_SUMMARY_CHAR('.');
				}
			}
		}
		DUK_DEBUG_SUMMARY_CHAR(']');
		DUK_DEBUG_SUMMARY_FINISH();

		DUK_D(DUK_DPRINT("  valstack: ptr %p, end %p (%ld), bottom %p (%ld), top %p (%ld), used size %ld entries (%ld bytes), alloc size %ld entries (%ld bytes)",
		                 (void *) thr->valstack,
		                 (void *) thr->valstack_end,
		                 (long) (thr->valstack_end - thr->valstack),
		                 (void *) thr->valstack_bottom,
		                 (long) (thr->valstack_bottom - thr->valstack),
		                 (void *) thr->valstack_top,
		                 (long) (thr->valstack_top - thr->valstack),
		                 (long) (thr->valstack_top - thr->valstack),
		                 (long) (thr->valstack_top - thr->valstack) * sizeof(duk_tval),
		                 (long) (thr->valstack_end - thr->valstack),
		                 (long) (thr->valstack_end - thr->valstack) * sizeof(duk_tval)));

		DUK_DEBUG_SUMMARY_INIT();
		DUK_DEBUG_SUMMARY_CHAR('[');
		p = thr->valstack;
		while (p <= thr->valstack_end) {
			i = (duk_uint_fast32_t) (p - thr->valstack);
			if (thr->callstack &&
			    thr->callstack_top > 0 &&
			    i == (duk_size_t) (thr->callstack + thr->callstack_top - 1)->idx_bottom) {
				DUK_DEBUG_SUMMARY_CHAR('>');
			}
			if (p == thr->valstack_top) {
				DUK_DEBUG_SUMMARY_CHAR('|');
			}
			if (p < thr->valstack_end) {
				if (p < thr->valstack_top) {
					DUK_DEBUG_SUMMARY_CHAR(duk__get_tval_summary_char(p));
				} else {
					/* XXX: safe printer for these?  would be nice, because
					 * we could visualize whether the values are in proper
					 * state.
					 */
					DUK_DEBUG_SUMMARY_CHAR('.');
				}
			}
			p++;
		}
		DUK_DEBUG_SUMMARY_CHAR(']');
		DUK_DEBUG_SUMMARY_FINISH();

		DUK_D(DUK_DPRINT("  catchstack: ptr %p, size %ld, top %ld, used size %ld entries (%ld bytes), alloc size %ld entries (%ld bytes)",
		                 (void *) thr->catchstack,
		                 (long) thr->catchstack_size,
		                 (long) thr->catchstack_top,
		                 (long) thr->catchstack_top,
		                 (long) (thr->catchstack_top * sizeof(duk_catcher)),
		                 (long) thr->catchstack_size,
		                 (long) (thr->catchstack_size * sizeof(duk_catcher))));

		DUK_DEBUG_SUMMARY_INIT();
		DUK_DEBUG_SUMMARY_CHAR('[');
		for (i = 0; i <= (duk_uint_fast32_t) thr->catchstack_size; i++) {
			if (i == thr->catchstack_top) {
				DUK_DEBUG_SUMMARY_CHAR('|');
			}
			if (!thr->catchstack) {
				DUK_DEBUG_SUMMARY_CHAR('@');
			} else if (i < thr->catchstack_size) {
				if (i < thr->catchstack_top) {
					DUK_DEBUG_SUMMARY_CHAR(duk__get_cat_summary_char(thr->catchstack + i));
				} else {
					DUK_DEBUG_SUMMARY_CHAR('.');
				}
			}
		}
		DUK_DEBUG_SUMMARY_CHAR(']');
		DUK_DEBUG_SUMMARY_FINISH();

		DUK_D(DUK_DPRINT("  resumer: ptr %p", (void *) thr->resumer));

#if 0  /* worth dumping? */
		for (i = 0; i < DUK_NUM_BUILTINS; i++) {
			DUK_D(DUK_DPRINT("  builtins[%ld] -> %!@O", (long) i, (duk_heaphdr *) thr->builtins[i]));
		}
#endif
	}

	if (obj->p) {
		DUK_D(DUK_DPRINT("  props alloc size: %ld",
		                 (long) DUK_HOBJECT_P_COMPUTE_SIZE(obj->e_size, obj->a_size, obj->h_size)));
	} else {
		DUK_D(DUK_DPRINT("  props alloc size: n/a"));
	}

	DUK_D(DUK_DPRINT("  prop entries:"));
	for (i = 0; i < (duk_uint_fast32_t) obj->e_size; i++) {
		duk_hstring *k;
		duk_propvalue *v;

		k = DUK_HOBJECT_E_GET_KEY(obj, i);
		v = DUK_HOBJECT_E_GET_VALUE_PTR(obj, i);
		DUK_UNREF(v);

		if (i >= obj->e_next) {
			DUK_D(DUK_DPRINT("    [%ld]: UNUSED", (long) i));
			continue;
		}

		if (!k) {
			DUK_D(DUK_DPRINT("    [%ld]: NULL", (long) i));
			continue;
		}

		if (DUK_HOBJECT_E_SLOT_IS_ACCESSOR(obj, i)) {
			DUK_D(DUK_DPRINT("    [%ld]: [w=%ld e=%ld c=%ld a=%ld] %!O -> get:%p set:%p; get %!O; set %!O",
			                 (long) i,
			                 (long) (DUK_HOBJECT_E_SLOT_IS_WRITABLE(obj, i) ? 1 : 0),
			                 (long) (DUK_HOBJECT_E_SLOT_IS_ENUMERABLE(obj, i) ? 1 : 0),
			                 (long) (DUK_HOBJECT_E_SLOT_IS_CONFIGURABLE(obj, i) ? 1 : 0),
			                 (long) (DUK_HOBJECT_E_SLOT_IS_ACCESSOR(obj, i) ? 1 : 0),
			                 (duk_heaphdr *) k,
			                 (void *) v->a.get,
			                 (void *) v->a.set,
			                 (duk_heaphdr *) v->a.get,
			                 (duk_heaphdr *) v->a.set));
		} else {
			DUK_D(DUK_DPRINT("    [%ld]: [w=%ld e=%ld c=%ld a=%ld] %!O -> %!T",
			                 (long) i,
			                 (long) (DUK_HOBJECT_E_SLOT_IS_WRITABLE(obj, i) ? 1 : 0),
			                 (long) (DUK_HOBJECT_E_SLOT_IS_ENUMERABLE(obj, i) ? 1 : 0),
			                 (long) (DUK_HOBJECT_E_SLOT_IS_CONFIGURABLE(obj, i) ? 1 : 0),
			                 (long) (DUK_HOBJECT_E_SLOT_IS_ACCESSOR(obj, i) ? 1 : 0),
			                 (duk_heaphdr *) k,
			                 (duk_tval *) &v->v));
		}
	}

	DUK_D(DUK_DPRINT("  array entries:"));
	for (i = 0; i < (duk_uint_fast32_t) obj->a_size; i++) {
		DUK_D(DUK_DPRINT("    [%ld]: [w=%ld e=%ld c=%ld a=%ld] %ld -> %!T",
		                 (long) i,
		                 (long) 1,  /* implicit attributes */
		                 (long) 1,
		                 (long) 1,
		                 (long) 0,
		                 (long) i,
		                 (duk_tval *) DUK_HOBJECT_A_GET_VALUE_PTR(obj, i)));
	}

	DUK_D(DUK_DPRINT("  hash entries:"));
	for (i = 0; i < (duk_uint_fast32_t) obj->h_size; i++) {
		duk_uint32_t t = DUK_HOBJECT_H_GET_INDEX(obj, i);
		if (t == DUK_HOBJECT_HASHIDX_UNUSED) {
			DUK_D(DUK_DPRINT("    [%ld]: unused", (long) i));
		} else if (t == DUK_HOBJECT_HASHIDX_DELETED) {
			DUK_D(DUK_DPRINT("    [%ld]: deleted", (long) i));
		} else {
			DUK_D(DUK_DPRINT("    [%ld]: %ld", (long) i, (long) t));
		}
	}
}

#if 0  /*unused*/
DUK_INTERNAL void duk_debug_dump_callstack(duk_hthread *thr) {
	duk_uint_fast32_t i;

	DUK_D(DUK_DPRINT("=== hthread %p callstack: %ld entries ===",
	                 (void *) thr,
	                 (thr == NULL ? (long) 0 : (long) thr->callstack_top)));
	if (!thr) {
		return;
	}

	for (i = 0; i < (duk_uint_fast32_t) thr->callstack_top; i++) {
		duk_activation *act = thr->callstack + i;
		duk_tval *this_binding = NULL;

		this_binding = thr->valstack + act->idx_bottom - 1;
		if (this_binding < thr->valstack || this_binding >= thr->valstack_top) {
			this_binding = NULL;
		}

		DUK_D(DUK_DPRINT("  [%ld] -> flags=0x%08lx, func=%!O, var_env=%!iO, lex_env=%!iO, "
		                 "pc=%ld, idx_bottom=%ld, idx_retval=%ld, this_binding=%!T",
		                 (long) i,
		                 (unsigned long) act->flags,
		                 (duk_heaphdr *) act->func,
		                 (duk_heaphdr *) act->var_env,
		                 (duk_heaphdr *) act->lex_env,
		                 (long) act->pc,
		                 (long) act->idx_bottom,
		                 (long) act->idx_retval,
		                 (duk_tval *) this_binding));
	}
}
#endif

#if 0  /*unused*/
DUK_INTERNAL void duk_debug_dump_activation(duk_hthread *thr, duk_activation *act) {
	if (!act) {
		DUK_D(DUK_DPRINT("duk_activation: NULL"));
	} else {
		duk_tval *this_binding = NULL;

		this_binding = thr->valstack + act->idx_bottom - 1;
		if (this_binding < thr->valstack || this_binding >= thr->valstack_top) {
			this_binding = NULL;
		}

		DUK_D(DUK_DPRINT("duk_activation: %p -> flags=0x%08lx, func=%!O, var_env=%!O, "
		                 "lex_env=%!O, pc=%ld, idx_bottom=%ld, idx_retval=%ld, this_binding=%!T",
		                 (void *) act,
		                 (unsigned long) act->flags,
		                 (duk_heaphdr *) act->func,
		                 (duk_heaphdr *) act->var_env,
		                 (duk_heaphdr *) act->lex_env,
		                 (long) act->pc,
		                 (long) act->idx_bottom,
		                 (long) act->idx_retval,
		                 (duk_tval *) this_binding));
	}
}
#endif

#endif  /* DUK_USE_DEBUG */
