/*
 *  Promise built-ins (ES7 Section 25.4)
 *  http://www.ecma-international.org/ecma-262/7.0/#sec-promise-objects
 *
 *  Also refer to the Promises/A+ specification, which Ecmascript Promises are
 *  based on:
 *  https://promisesaplus.com/
 */

#include "duk_internal.h"

#define DUK__PS_PENDING (0)
#define DUK__PS_FULFILL (1)
#define DUK__PS_REJECT  (2)

typedef struct {
	duk_uint8_t handled;
	duk_uint8_t state;
} duk__promise;

DUK_LOCAL void duk__perform_promise_then(duk_context *ctx);
DUK_LOCAL void duk__trigger_promise_reactions(duk_context *ctx, duk_idx_t idx);

#if defined(DUK_USE_PROMISE_BUILTIN)
DUK_LOCAL void duk__push_promise(duk_context *ctx)
{
	/*
	 *  [ ... ] -> [ ... promise ]
	 */

	duk__promise *prom;

	duk_push_object_helper(ctx,
	                       DUK_HOBJECT_FLAG_EXTENSIBLE |
	                       DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                       DUK_BIDX_PROMISE_PROTOTYPE);

	/*  [ ... promise ] */

	prom = (duk__promise *) duk_push_fixed_buffer(ctx, sizeof(duk__promise));
	prom->state = DUK__PS_PENDING;
	prom->handled = 0;
	duk_put_prop_string(ctx, -2, "\xff" "PromData");

	duk_push_array(ctx);
	duk_put_prop_string(ctx, -2, "\xff" "FulfillActs");

	duk_push_array(ctx);
	duk_put_prop_string(ctx, -2, "\xff" "RejectActs");

	/*  [ ... promise ] */
}

DUK_LOCAL duk_bool_t duk__is_promise(duk_context *ctx, duk_idx_t idx) {
	return duk_is_object(ctx, idx) &&
	       duk_has_prop_string(ctx, idx, "\xff" "PromData");
}

DUK_LOCAL duk__promise *duk__require_promise(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk__promise *prom;

	if (!(duk__is_promise(ctx, idx))) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "promise", DUK_STR_NOT_PROMISE);
	}

	duk_get_prop_string(ctx, idx, "\xff" "PromData");
	prom = (duk__promise *) duk_require_buffer(ctx, -1, NULL);
	duk_pop(ctx);
	return prom;
}

DUK_LOCAL void duk__settle_promise(duk_context *ctx, duk_idx_t idx, duk_uint8_t state) {
	/*
	 *  [ ... promise ... value ] -> [ ... promise ... ]
	 */

	duk__promise *prom;

	DUK_ASSERT(state == DUK__PS_FULFILL || state == DUK__PS_REJECT);

	idx = duk_require_normalize_index(ctx, idx);
	prom = duk__require_promise(ctx, idx);
	if (prom->state != DUK__PS_PENDING) {
		duk_pop(ctx);
		return;  /* nop, already settled */
	}

	prom->state = state;
	duk_put_prop_string(ctx, idx, "\xff" "Value");
	duk__trigger_promise_reactions(ctx, idx);
}

DUK_LOCAL duk_ret_t duk__promise_settler(duk_context *ctx) {
	duk_uint8_t state;

	DUK_ASSERT_TOP(ctx, 1);

	duk_push_current_function(ctx);
	state = (duk_uint8_t) duk_get_magic(ctx, -1);
	duk_get_prop_string(ctx, -1, "\xff" "Promise");
	duk_dup(ctx, 0);
	duk__settle_promise(ctx, -2, state);
	return 0;
}

DUK_LOCAL void duk__push_promise_settler(duk_context *ctx, duk_idx_t idx_prom, duk_uint8_t state) {
	idx_prom = duk_require_normalize_index(ctx, idx_prom);
	duk_push_c_function(ctx, duk__promise_settler, 1);
	duk_set_magic(ctx, -1, (duk_int_t) state);
	duk_put_prop_string(ctx, -1, "\xff" "Promise");
}

/* GetCapabilitiesExecutor (ES7 25.4.1.5.1) */
DUK_LOCAL duk_ret_t duk__get_caps_executor(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 2);

	duk_push_current_function(ctx);
	duk_get_prop_string(ctx, -1, "capability");

	duk_dup(ctx, 0);
	duk_put_prop_string(ctx, -2, "resolve");
	duk_dup(ctx, 1);
	duk_put_prop_string(ctx, -2, "reject");
	return 0;
}

/*  NewPromiseCapability (ES7 Section 25.4.1.5) */
DUK_LOCAL void duk__new_promise_cap(duk_context *ctx) {
	/* [ ... ctor ] -> [ promCap ... ] */

	/* FIXME: duk_require_constructable() */
	duk_require_callable(ctx, -1);

	duk_push_object(ctx);

	/* [ ... ctor promCap ]

	/* Steps 4-6 */
	duk_dup(ctx, -2);
	duk_push_c_function(ctx, duk__get_caps_executor, 2);
	duk_dup(ctx, -3);  /* promCap */
	duk_put_prop_string(ctx, -2, "capability");
	duk_new(ctx, 1);

	/* [ ... ctor promCap promise ]

	/* Steps 7-8 */
	duk_get_prop_string(ctx, -2, "resolve");
	duk_require_callable(ctx, -1);
	duk_get_prop_string(ctx, -3, "reject");
	duk_require_callable(ctx, -1);
	duk_pop_2(ctx);

	/* Steps 9-10 */
	duk_put_prop_string(ctx, -2, "promise");

	/* [ ... ctor promCap ] */

	duk_remove(ctx, -2);
}

/*
 *  PerformPromiseThen (ES7 Section 25.4.5.3.1)
 *
 *  There are some slight differences to the algorithm described in ES7 in an
 *  effort to save footprint.  In particular, we avoid the PromiseCapability
 *  concept entirely and just compose the result promise on the fly.
 */
DUK_LOCAL void duk__perform_promise_then(duk_context *ctx) {
	/*
	 *  [ ... promise onFulfilled onRejected promCap ] -> [ ... newPromise ]
	 */

	duk_uarridx_t len;
	duk_idx_t idx_prom;
	duk_idx_t idx_fulfill_func;
	duk_idx_t idx_reject_func;
	duk_idx_t idx_cap;
	duk__promise *prom;

	idx_prom = duk_require_normalize_index(ctx, -4);
	idx_fulfill_func = duk_require_normalize_index(ctx, -3);
	idx_reject_func = duk_require_normalize_index(ctx, -2);
	idx_cap = duk_require_normalize_index(ctx, -1);

	prom = duk__require_promise(ctx, idx_prom);

	/* Steps 1-9 */
	if (duk_is_callable(ctx, idx_fulfill_func)) {
		duk_get_prop_string(ctx, idx_prom, "\xff" "FulfillActs");
		len = (duk_uarridx_t) duk_get_length(ctx, -1);
		duk_push_object(ctx);
		duk_dup(ctx, idx_cap);
		duk_put_prop_string(ctx, -2, "capability");
		duk_dup(ctx, idx_fulfill_func);
		duk_put_prop_string(ctx, -2, "handler");
		duk_put_prop_index(ctx, -2, len);
		duk_pop(ctx);
	}
	if (duk_is_callable(ctx, idx_reject_func)) {
		duk_get_prop_string(ctx, idx_prom, "\xff" "RejectActs");
		len = (duk_uarridx_t) duk_get_length(ctx, -1);
		duk_push_object(ctx);
		duk_dup(ctx, idx_cap);
		duk_put_prop_string(ctx, -2, "capability");
		duk_dup(ctx, idx_fulfill_func);
		duk_put_prop_string(ctx, -2, "handler");
		duk_put_prop_index(ctx, -2, len);
		duk_pop(ctx);
	}
	duk__trigger_promise_reactions(ctx, idx_prom);

	/* Step 10 */
	prom->handled = 1;

	/* [ ... promise ] */

	duk__push_promise(ctx);
	duk__push_promise_settler(ctx, -1, DUK__PS_FULFILL);
	duk__push_promise_settler(ctx, -2, DUK__PS_REJECT);
}

/*
 *  TriggerPromiseReactions (ES7 25.4.1.8)
 */
DUK_LOCAL void duk__trigger_promise_reactions(duk_context *ctx, duk_idx_t idx) {
	duk__promise *prom;

	idx = duk_require_normalize_index(ctx, idx);
	prom = duk__require_promise(ctx, idx);
	if (prom->state != DUK__PS_PENDING) {
		duk_get_prop_string(ctx, idx, "\xff" "Value");
	} else {
		/* pending promise, do nothing */
		return;
	}

	/* [ ... value ] */

	DUK_ASSERT(prom->state == DUK__PS_FULFILL || prom->state == DUK_PS_REJECT);
	if (prom->state == DUK__PS_FULFILL) {
		duk_get_prop_string(ctx, idx, "\xff" "FulfillActs");
	} else {
		duk_get_prop_string(ctx, idx, "\xff" "RejectActs");
	}

	/* FIXME: PromiseReactionJob */
	duk_enum(ctx, -1, DUK_ENUM_ARRAY_INDICES_ONLY);
	while (duk_next(ctx, -1, 1)) {
		/* [ ... value handlers enum idx reaction ] */

		duk_get_prop_string(ctx, -1, "capability");

		/* [ ... value handlers enum idx reaction promCap ] */

		duk_get_prop_string(ctx, -2, "handler");
		duk_push_undefined(ctx);
		duk_dup(ctx, -6);
		if (duk_pcall_method(ctx, 1) != 0) {
			duk_get_prop_string(ctx, -2, "reject");
			duk_dup(ctx, -2);
			duk_call(ctx, 1);
			duk_pop_2(ctx);
		} else {
			duk_get_prop_string(ctx, -2, "resolve");
			duk_dup(ctx, -2);
			duk_call(ctx, 1);
			duk_pop_2(ctx);
		}
		
		duk_dup(ctx, -5);  /* value */
		duk_call(ctx, 1);
		duk_pop_2(ctx);
	}
	duk_set_length(ctx, -2, 0);

	/* [ ... value handlers enum ] */

	duk_pop_n(ctx, 3);
}

DUK_INTERNAL duk_ret_t duk_bi_promise_constructor(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 1);

	/* [ executorFunc ] */

	duk_require_constructor_call(ctx);
	duk_require_callable(ctx, 0);

	duk__push_promise(ctx);

	/* [ executorFunc promise ] */

	duk_dup(ctx, 0);
	duk__push_promise_settler(ctx, -2, DUK__PS_FULFILL);
	duk__push_promise_settler(ctx, -2, DUK__PS_REJECT);
	if (duk_pcall(ctx, 2) != 0) {
		/* If the executor function throws, reject the promise with whatever
		 * was thrown as the reason value.
		 */
		duk__settle_promise(ctx, -2, DUK__PS_REJECT);
	} else {
		/* don't need return value */
		duk_pop(ctx);
	}

	/* [ executorFunc promise ] */

	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_promise_all(duk_context *ctx) {
	DUK_ERROR_UNSUPPORTED((duk_hthread *) ctx);
}

DUK_INTERNAL duk_ret_t duk_bi_promise_race(duk_context *ctx) {
	DUK_ERROR_UNSUPPORTED((duk_hthread *) ctx);
}

/* Shared helper for Promise.resolve() and Promise.reject() */
DUK_INTERNAL duk_ret_t duk_bi_promise_resolve_shared(duk_context *ctx) {
	/*
	 *  magic = 0: Promise.resolve()
	 *  magic = 1: Promise.reject()
	 */
	duk_int_t magic;

	duk_tval *tv_ctor;
	duk_tval *tv_this;

	magic = duk_get_current_magic(ctx);
	DUK_ASSERT_TOP(ctx, 1);
	DUK_ASSERT(magic == 0 || magic == 1);

	/* [ value ] */

	/* ES7 Sections 25.4.4.4, 25.4.4.5 */
	duk_push_this(ctx);
	duk_require_hobject(ctx, -1);  /* Step 2 */
	if (magic == 0 && duk__is_promise(ctx, 0)) {
		duk_get_prop_string(ctx, 0, "constructor");
		tv_ctor = DUK_GET_TVAL_NEGIDX(ctx, -1);
		tv_this = DUK_GET_TVAL_NEGIDX(ctx, -2);
		if (duk_js_samevalue(tv_ctor, tv_this)) {  /* Step 3b */
			duk_pop_2(ctx);
			return 1;
		}
	}

	duk__push_promise(ctx);
	duk_dup(ctx, 0);
	duk__settle_promise(ctx, -2, magic == 0 ?
	                    DUK__PS_FULFILL : DUK__PS_REJECT);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_promise_prototype_catch(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 1);

	/* [ onRejected ] */

	/* Unusually for an Ecmascript built-in, .catch() must call .then()
	 * polymorphically.  See ES7 Section 25.4.5.1.
	 */
	duk_get_prop_string(ctx, 0, "then");
	duk_push_this(ctx);
	duk_push_undefined(ctx);
	duk_dup(ctx, 0);
	duk_call_method(ctx, 2);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_promise_prototype_then(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 2);

	duk_push_this(ctx);
	duk__require_promise(ctx, -1);
	duk_insert(ctx, 0);

	/* [ promise onFulfilled onRejected ] */

	/* FIXME: SpeciesConstructor */
	duk_push_hobject_bidx(ctx, DUK_BIDX_PROMISE_CONSTRUCTOR);
	duk__new_promise_cap(ctx);

	/* [ promise onFulfilled onRejected promCap ] */

	duk__perform_promise_then(ctx);
	return 1;
}
#endif
