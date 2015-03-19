/*
 *  Initialize built-in objects.  Current thread must have a valstack
 *  and initialization errors may longjmp, so a setjmp() catch point
 *  must exist.
 */

#include "duk_internal.h"

/*
 *  Encoding constants, must match genbuiltins.py
 */

#define DUK__CLASS_BITS                  5
#define DUK__BIDX_BITS                   6
#define DUK__STRIDX_BITS                 9  /* XXX: try to optimize to 8 */
#define DUK__NATIDX_BITS                 8
#define DUK__NUM_NORMAL_PROPS_BITS       6
#define DUK__NUM_FUNC_PROPS_BITS         6
#define DUK__PROP_FLAGS_BITS             3
#define DUK__STRING_LENGTH_BITS          8
#define DUK__STRING_CHAR_BITS            7
#define DUK__LENGTH_PROP_BITS            3
#define DUK__NARGS_BITS                  3
#define DUK__PROP_TYPE_BITS              3
#define DUK__MAGIC_BITS                  16

#define DUK__NARGS_VARARGS_MARKER        0x07
#define DUK__NO_CLASS_MARKER             0x00   /* 0 = DUK_HOBJECT_CLASS_UNUSED */
#define DUK__NO_BIDX_MARKER              0x3f
#define DUK__NO_STRIDX_MARKER            0xff

#define DUK__PROP_TYPE_DOUBLE            0
#define DUK__PROP_TYPE_STRING            1
#define DUK__PROP_TYPE_STRIDX            2
#define DUK__PROP_TYPE_BUILTIN           3
#define DUK__PROP_TYPE_UNDEFINED         4
#define DUK__PROP_TYPE_BOOLEAN_TRUE      5
#define DUK__PROP_TYPE_BOOLEAN_FALSE     6
#define DUK__PROP_TYPE_ACCESSOR          7

/*
 *  Create built-in objects by parsing an init bitstream generated
 *  by genbuiltins.py.
 */

DUK_INTERNAL void duk_hthread_create_builtin_objects(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
	duk_bitdecoder_ctx bd_ctx;
	duk_bitdecoder_ctx *bd = &bd_ctx;  /* convenience */
	duk_hobject *h;
	duk_small_uint_t i, j;

	DUK_D(DUK_DPRINT("INITBUILTINS BEGIN"));

	DUK_MEMZERO(&bd_ctx, sizeof(bd_ctx));
	bd->data = (const duk_uint8_t *) duk_builtins_data;
	bd->length = (duk_size_t) DUK_BUILTINS_DATA_LENGTH;

	/*
	 *  First create all built-in bare objects on the empty valstack.
	 *  During init, their indices will correspond to built-in indices.
	 *
	 *  Built-ins will be reachable from both valstack and thr->builtins.
	 */

	/* XXX: there is no need to resize valstack because builtin count
	 * is much less than the default space; assert for it.
	 */

	DUK_DD(DUK_DDPRINT("create empty built-ins"));
	DUK_ASSERT_TOP(ctx, 0);
	for (i = 0; i < DUK_NUM_BUILTINS; i++) {
		duk_small_uint_t class_num;
		duk_small_int_t len = -1;  /* must be signed */

		class_num = (duk_small_uint_t) duk_bd_decode(bd, DUK__CLASS_BITS);
		len = (duk_small_int_t) duk_bd_decode_flagged(bd, DUK__LENGTH_PROP_BITS, (duk_int32_t) -1 /*def_value*/);

		if (class_num == DUK_HOBJECT_CLASS_FUNCTION) {
			duk_small_uint_t natidx;
			duk_small_uint_t stridx;
			duk_int_t c_nargs;  /* must hold DUK_VARARGS */
			duk_c_function c_func;
			duk_int16_t magic;

			DUK_DDD(DUK_DDDPRINT("len=%ld", (long) len));
			DUK_ASSERT(len >= 0);

			natidx = (duk_small_uint_t) duk_bd_decode(bd, DUK__NATIDX_BITS);
			stridx = (duk_small_uint_t) duk_bd_decode(bd, DUK__STRIDX_BITS);
			c_func = duk_bi_native_functions[natidx];

			c_nargs = (duk_small_uint_t) duk_bd_decode_flagged(bd, DUK__NARGS_BITS, len /*def_value*/);
			if (c_nargs == DUK__NARGS_VARARGS_MARKER) {
				c_nargs = DUK_VARARGS;
			}

			/* XXX: set magic directly here? (it could share the c_nargs arg) */
			duk_push_c_function_noexotic(ctx, c_func, c_nargs);

			h = duk_require_hobject(ctx, -1);
			DUK_ASSERT(h != NULL);

			/* Currently all built-in native functions are strict.
			 * duk_push_c_function() now sets strict flag, so
			 * assert for it.
			 */
			DUK_ASSERT(DUK_HOBJECT_HAS_STRICT(h));

			/* XXX: function properties */

			duk_push_hstring_stridx(ctx, stridx);
			duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_NAME, DUK_PROPDESC_FLAGS_NONE);

			/* Almost all global level Function objects are constructable
			 * but not all: Function.prototype is a non-constructable,
			 * callable Function.
			 */
			if (duk_bd_decode_flag(bd)) {
				DUK_ASSERT(DUK_HOBJECT_HAS_CONSTRUCTABLE(h));
			} else {
				DUK_HOBJECT_CLEAR_CONSTRUCTABLE(h);
			}

			/* Cast converts magic to 16-bit signed value */
			magic = (duk_int16_t) duk_bd_decode_flagged(bd, DUK__MAGIC_BITS, 0 /*def_value*/);
			((duk_hnativefunction *) h)->magic = magic;
		} else {
			/* XXX: ARRAY_PART for Array prototype? */

			duk_push_object_helper(ctx,
			                       DUK_HOBJECT_FLAG_EXTENSIBLE,
			                       -1);  /* no prototype or class yet */

			h = duk_require_hobject(ctx, -1);
			DUK_ASSERT(h != NULL);
		}

		DUK_HOBJECT_SET_CLASS_NUMBER(h, class_num);

		thr->builtins[i] = h;
		DUK_HOBJECT_INCREF(thr, &h->hdr);

		if (len >= 0) {
			/*
			 *  For top-level objects, 'length' property has the following
			 *  default attributes: non-writable, non-enumerable, non-configurable
			 *  (E5 Section 15).
			 *
			 *  However, 'length' property for Array.prototype has attributes
			 *  expected of an Array instance which are different: writable,
			 *  non-enumerable, non-configurable (E5 Section 15.4.5.2).
			 *
			 *  This is currently determined implicitly based on class; there are
			 *  no attribute flags in the init data.
			 */

			duk_push_int(ctx, len);
			duk_xdef_prop_stridx(ctx,
			                     -2,
			                     DUK_STRIDX_LENGTH,
			                     (class_num == DUK_HOBJECT_CLASS_ARRAY ?  /* only Array.prototype matches */
			                      DUK_PROPDESC_FLAGS_W : DUK_PROPDESC_FLAGS_NONE));
		}

		/* enable exotic behaviors last */

		if (class_num == DUK_HOBJECT_CLASS_ARRAY) {
			DUK_HOBJECT_SET_EXOTIC_ARRAY(h);
		}
		if (class_num == DUK_HOBJECT_CLASS_STRING) {
			DUK_HOBJECT_SET_EXOTIC_STRINGOBJ(h);
		}

		/* some assertions */

		DUK_ASSERT(DUK_HOBJECT_HAS_EXTENSIBLE(h));
		/* DUK_HOBJECT_FLAG_CONSTRUCTABLE varies */
		DUK_ASSERT(!DUK_HOBJECT_HAS_BOUND(h));
		DUK_ASSERT(!DUK_HOBJECT_HAS_COMPILEDFUNCTION(h));
		/* DUK_HOBJECT_FLAG_NATIVEFUNCTION varies */
		DUK_ASSERT(!DUK_HOBJECT_HAS_THREAD(h));
		DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_PART(h));       /* currently, even for Array.prototype */
		/* DUK_HOBJECT_FLAG_STRICT varies */
		DUK_ASSERT(!DUK_HOBJECT_HAS_NATIVEFUNCTION(h) ||  /* all native functions have NEWENV */
		           DUK_HOBJECT_HAS_NEWENV(h));
		DUK_ASSERT(!DUK_HOBJECT_HAS_NAMEBINDING(h));
		DUK_ASSERT(!DUK_HOBJECT_HAS_CREATEARGS(h));
		DUK_ASSERT(!DUK_HOBJECT_HAS_ENVRECCLOSED(h));
		/* DUK_HOBJECT_FLAG_EXOTIC_ARRAY varies */
		/* DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ varies */
		DUK_ASSERT(!DUK_HOBJECT_HAS_EXOTIC_ARGUMENTS(h));

		DUK_DDD(DUK_DDDPRINT("created built-in %ld, class=%ld, length=%ld", (long) i, (long) class_num, (long) len));
	}

	/*
	 *  Then decode the builtins init data (see genbuiltins.py) to
	 *  init objects
	 */

	DUK_DD(DUK_DDPRINT("initialize built-in object properties"));
	for (i = 0; i < DUK_NUM_BUILTINS; i++) {
		duk_small_uint_t t;
		duk_small_uint_t num;

		DUK_DDD(DUK_DDDPRINT("initializing built-in object at index %ld", (long) i));
		h = thr->builtins[i];

		t = (duk_small_uint_t) duk_bd_decode(bd, DUK__BIDX_BITS);
		if (t != DUK__NO_BIDX_MARKER) {
			DUK_DDD(DUK_DDDPRINT("set internal prototype: built-in %ld", (long) t));
			DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, h, thr->builtins[t]);
		}

		t = (duk_small_uint_t) duk_bd_decode(bd, DUK__BIDX_BITS);
		if (t != DUK__NO_BIDX_MARKER) {
			/* 'prototype' property for all built-in objects (which have it) has attributes:
			 *  [[Writable]] = false,
			 *  [[Enumerable]] = false,
			 *  [[Configurable]] = false
			 */
			DUK_DDD(DUK_DDDPRINT("set external prototype: built-in %ld", (long) t));
			duk_xdef_prop_stridx_builtin(ctx, i, DUK_STRIDX_PROTOTYPE, t, DUK_PROPDESC_FLAGS_NONE);
		}

		t = (duk_small_uint_t) duk_bd_decode(bd, DUK__BIDX_BITS);
		if (t != DUK__NO_BIDX_MARKER) {
			/* 'constructor' property for all built-in objects (which have it) has attributes:
			 *  [[Writable]] = true,
			 *  [[Enumerable]] = false,
			 *  [[Configurable]] = true
			 */
			DUK_DDD(DUK_DDDPRINT("set external constructor: built-in %ld", (long) t));
			duk_xdef_prop_stridx_builtin(ctx, i, DUK_STRIDX_CONSTRUCTOR, t, DUK_PROPDESC_FLAGS_WC);
		}

		/* normal valued properties */
		num = (duk_small_uint_t) duk_bd_decode(bd, DUK__NUM_NORMAL_PROPS_BITS);
		DUK_DDD(DUK_DDDPRINT("built-in object %ld, %ld normal valued properties", (long) i, (long) num));
		for (j = 0; j < num; j++) {
			duk_small_uint_t stridx;
			duk_small_uint_t prop_flags;

			stridx = (duk_small_uint_t) duk_bd_decode(bd, DUK__STRIDX_BITS);

			/*
			 *  Property attribute defaults are defined in E5 Section 15 (first
			 *  few pages); there is a default for all properties and a special
			 *  default for 'length' properties.  Variation from the defaults is
			 *  signaled using a single flag bit in the bitstream.
			 */

			if (duk_bd_decode_flag(bd)) {
				prop_flags = (duk_small_uint_t) duk_bd_decode(bd, DUK__PROP_FLAGS_BITS);
			} else {
				if (stridx == DUK_STRIDX_LENGTH) {
					prop_flags = DUK_PROPDESC_FLAGS_NONE;
				} else {
					prop_flags = DUK_PROPDESC_FLAGS_WC;
				}
			}

			t = (duk_small_uint_t) duk_bd_decode(bd, DUK__PROP_TYPE_BITS);

			DUK_DDD(DUK_DDDPRINT("built-in %ld, normal-valued property %ld, stridx %ld, flags 0x%02lx, type %ld",
			                     (long) i, (long) j, (long) stridx, (unsigned long) prop_flags, (long) t));

			switch (t) {
			case DUK__PROP_TYPE_DOUBLE: {
				duk_double_union du;
				duk_small_uint_t k;

				for (k = 0; k < 8; k++) {
					/* Encoding endianness must match target memory layout,
					 * build scripts and genbuiltins.py must ensure this.
					 */
					du.uc[k] = (duk_uint8_t) duk_bd_decode(bd, 8);
				}

				duk_push_number(ctx, du.d);  /* push operation normalizes NaNs */
				break;
			}
			case DUK__PROP_TYPE_STRING: {
				duk_small_uint_t n;
				duk_small_uint_t k;
				duk_uint8_t *p;

				n = (duk_small_uint_t) duk_bd_decode(bd, DUK__STRING_LENGTH_BITS);
				p = (duk_uint8_t *) duk_push_fixed_buffer(ctx, n);
				for (k = 0; k < n; k++) {
					*p++ = (duk_uint8_t) duk_bd_decode(bd, DUK__STRING_CHAR_BITS);
				}

				duk_to_string(ctx, -1);
				break;
			}
			case DUK__PROP_TYPE_STRIDX: {
				duk_small_uint_t n;

				n = (duk_small_uint_t) duk_bd_decode(bd, DUK__STRIDX_BITS);
				DUK_ASSERT_DISABLE(n >= 0);  /* unsigned */
				DUK_ASSERT(n < DUK_HEAP_NUM_STRINGS);
				duk_push_hstring_stridx(ctx, n);
				break;
			}
			case DUK__PROP_TYPE_BUILTIN: {
				duk_small_uint_t bidx;

				bidx = (duk_small_uint_t) duk_bd_decode(bd, DUK__BIDX_BITS);
				DUK_ASSERT(bidx != DUK__NO_BIDX_MARKER);
				duk_dup(ctx, (duk_idx_t) bidx);
				break;
			}
			case DUK__PROP_TYPE_UNDEFINED: {
				duk_push_undefined(ctx);
				break;
			}
			case DUK__PROP_TYPE_BOOLEAN_TRUE: {
				duk_push_true(ctx);
				break;
			}
			case DUK__PROP_TYPE_BOOLEAN_FALSE: {
				duk_push_false(ctx);
				break;
			}
			case DUK__PROP_TYPE_ACCESSOR: {
				duk_small_uint_t natidx_getter = (duk_small_uint_t) duk_bd_decode(bd, DUK__NATIDX_BITS);
				duk_small_uint_t natidx_setter = (duk_small_uint_t) duk_bd_decode(bd, DUK__NATIDX_BITS);
				duk_c_function c_func_getter;
				duk_c_function c_func_setter;

				/* XXX: this is a bit awkward because there is no exposed helper
				 * in the API style, only this internal helper.
				 */
				DUK_DDD(DUK_DDDPRINT("built-in accessor property: objidx=%ld, stridx=%ld, getteridx=%ld, setteridx=%ld, flags=0x%04lx",
				                     (long) i, (long) stridx, (long) natidx_getter, (long) natidx_setter, (unsigned long) prop_flags));

				c_func_getter = duk_bi_native_functions[natidx_getter];
				c_func_setter = duk_bi_native_functions[natidx_setter];
				duk_push_c_function_noconstruct_noexotic(ctx, c_func_getter, 0);  /* always 0 args */
				duk_push_c_function_noconstruct_noexotic(ctx, c_func_setter, 1);  /* always 1 arg */

				/* XXX: magic for getter/setter? */

				prop_flags |= DUK_PROPDESC_FLAG_ACCESSOR;  /* accessor flag not encoded explicitly */
				duk_hobject_define_accessor_internal(thr,
				                                     duk_require_hobject(ctx, i),
				                                     DUK_HTHREAD_GET_STRING(thr, stridx),
				                                     duk_require_hobject(ctx, -2),
				                                     duk_require_hobject(ctx, -1),
				                                     prop_flags);
				duk_pop_2(ctx);  /* getter and setter, now reachable through object */
				goto skip_value;
			}
			default: {
				/* exhaustive */
				DUK_UNREACHABLE();
			}
			}

			DUK_ASSERT((prop_flags & DUK_PROPDESC_FLAG_ACCESSOR) == 0);
			duk_xdef_prop_stridx(ctx, i, stridx, prop_flags);

		 skip_value:
			continue;  /* avoid empty label at the end of a compound statement */
		}

		/* native function properties */
		num = (duk_small_uint_t) duk_bd_decode(bd, DUK__NUM_FUNC_PROPS_BITS);
		DUK_DDD(DUK_DDDPRINT("built-in object %ld, %ld function valued properties", (long) i, (long) num));
		for (j = 0; j < num; j++) {
			duk_small_uint_t stridx;
			duk_small_uint_t natidx;
			duk_int_t c_nargs;  /* must hold DUK_VARARGS */
			duk_small_uint_t c_length;
			duk_int16_t magic;
			duk_c_function c_func;
			duk_hnativefunction *h_func;
#if defined(DUK_USE_LIGHTFUNC_BUILTINS)
			duk_small_int_t lightfunc_eligible;
#endif

			stridx = (duk_small_uint_t) duk_bd_decode(bd, DUK__STRIDX_BITS);
			natidx = (duk_small_uint_t) duk_bd_decode(bd, DUK__NATIDX_BITS);

			c_length = (duk_small_uint_t) duk_bd_decode(bd, DUK__LENGTH_PROP_BITS);
			c_nargs = (duk_int_t) duk_bd_decode_flagged(bd, DUK__NARGS_BITS, (duk_int32_t) c_length /*def_value*/);
			if (c_nargs == DUK__NARGS_VARARGS_MARKER) {
				c_nargs = DUK_VARARGS;
			}

			c_func = duk_bi_native_functions[natidx];

			DUK_DDD(DUK_DDDPRINT("built-in %ld, function-valued property %ld, stridx %ld, natidx %ld, length %ld, nargs %ld",
			                     (long) i, (long) j, (long) stridx, (long) natidx, (long) c_length,
			                     (c_nargs == DUK_VARARGS ? (long) -1 : (long) c_nargs)));

			/* Cast converts magic to 16-bit signed value */
			magic = (duk_int16_t) duk_bd_decode_flagged(bd, DUK__MAGIC_BITS, 0);

#if defined(DUK_USE_LIGHTFUNC_BUILTINS)
			lightfunc_eligible =
				((c_nargs >= DUK_LFUNC_NARGS_MIN && c_nargs <= DUK_LFUNC_NARGS_MAX) || (c_nargs == DUK_VARARGS)) &&
				(c_length <= DUK_LFUNC_LENGTH_MAX) &&
				(magic >= DUK_LFUNC_MAGIC_MIN && magic <= DUK_LFUNC_MAGIC_MAX);
			if (stridx == DUK_STRIDX_EVAL ||
			    stridx == DUK_STRIDX_YIELD ||
			    stridx == DUK_STRIDX_RESUME ||
			    stridx == DUK_STRIDX_REQUIRE) {
				/* These functions have trouble working as lightfuncs.
				 * Some of them have specific asserts and some may have
			         * additional properties (e.g. 'require.id' may be written).
				 */
				DUK_D(DUK_DPRINT("reject as lightfunc: stridx=%d, i=%d, j=%d", (int) stridx, (int) i, (int) j));
				lightfunc_eligible = 0;
			}

			if (lightfunc_eligible) {
				duk_tval tv_lfunc;
				duk_small_uint_t lf_nargs = (c_nargs == DUK_VARARGS ? DUK_LFUNC_NARGS_VARARGS : c_nargs);
				duk_small_uint_t lf_flags = DUK_LFUNC_FLAGS_PACK(magic, c_length, lf_nargs);
				DUK_TVAL_SET_LIGHTFUNC(&tv_lfunc, c_func, lf_flags);
				duk_push_tval(ctx, &tv_lfunc);
				DUK_D(DUK_DPRINT("built-in function eligible as light function: i=%d, j=%d c_length=%ld, c_nargs=%ld, magic=%ld -> %!iT", (int) i, (int) j, (long) c_length, (long) c_nargs, (long) magic, duk_get_tval(ctx, -1)));
				goto lightfunc_skip;
			}

			DUK_D(DUK_DPRINT("built-in function NOT ELIGIBLE as light function: i=%d, j=%d c_length=%ld, c_nargs=%ld, magic=%ld", (int) i, (int) j, (long) c_length, (long) c_nargs, (long) magic));
#endif  /* DUK_USE_LIGHTFUNC_BUILTINS */

			/* [ (builtin objects) ] */

			duk_push_c_function_noconstruct_noexotic(ctx, c_func, c_nargs);
			h_func = duk_require_hnativefunction(ctx, -1);
			DUK_UNREF(h_func);

			/* Currently all built-in native functions are strict.
			 * This doesn't matter for many functions, but e.g.
			 * String.prototype.charAt (and other string functions)
			 * rely on being strict so that their 'this' binding is
			 * not automatically coerced.
			 */
			DUK_HOBJECT_SET_STRICT((duk_hobject *) h_func);

			/* No built-in functions are constructable except the top
			 * level ones (Number, etc).
			 */
			DUK_ASSERT(!DUK_HOBJECT_HAS_CONSTRUCTABLE((duk_hobject *) h_func));

			/* XXX: any way to avoid decoding magic bit; there are quite
			 * many function properties and relatively few with magic values.
			 */
			h_func->magic = magic;

			/* [ (builtin objects) func ] */

			duk_push_int(ctx, c_length);
			duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_LENGTH, DUK_PROPDESC_FLAGS_NONE);

			duk_push_hstring_stridx(ctx, stridx);
			duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_NAME, DUK_PROPDESC_FLAGS_NONE);

			/* XXX: other properties of function instances; 'arguments', 'caller'. */

			DUK_DD(DUK_DDPRINT("built-in object %ld, function property %ld -> %!T",
			                   (long) i, (long) j, (duk_tval *) duk_get_tval(ctx, -1)));

			/* [ (builtin objects) func ] */

			/*
			 *  The default property attributes are correct for all
			 *  function valued properties of built-in objects now.
			 */

#if defined(DUK_USE_LIGHTFUNC_BUILTINS)
		 lightfunc_skip:
#endif

			duk_xdef_prop_stridx(ctx, i, stridx, DUK_PROPDESC_FLAGS_WC);

			/* [ (builtin objects) ] */
		}
	}

	/*
	 *  Special post-tweaks, for cases not covered by the init data format.
	 *
	 *  - Set Date.prototype.toGMTString to Date.prototype.toUTCString.
	 *    toGMTString is required to have the same Function object as
	 *    toUTCString in E5 Section B.2.6.  Note that while Smjs respects
	 *    this, V8 does not (the Function objects are distinct).
	 *
	 *  - Make DoubleError non-extensible.
	 *
	 *  - Add info about most important effective compile options to Duktape.
	 *
	 *  - Possibly remove some properties (values or methods) which are not
	 *    desirable with current feature options but are not currently
	 *    conditional in init data.
	 */

	duk_get_prop_stridx(ctx, DUK_BIDX_DATE_PROTOTYPE, DUK_STRIDX_TO_UTC_STRING);
	duk_xdef_prop_stridx(ctx, DUK_BIDX_DATE_PROTOTYPE, DUK_STRIDX_TO_GMT_STRING, DUK_PROPDESC_FLAGS_WC);

	h = duk_require_hobject(ctx, DUK_BIDX_DOUBLE_ERROR);
	DUK_ASSERT(h != NULL);
	DUK_HOBJECT_CLEAR_EXTENSIBLE(h);

#if !defined(DUK_USE_ES6_OBJECT_PROTO_PROPERTY)
	DUK_DD(DUK_DDPRINT("delete Object.prototype.__proto__ built-in which is not enabled in features"));
	(void) duk_hobject_delprop_raw(thr, thr->builtins[DUK_BIDX_OBJECT_PROTOTYPE], DUK_HTHREAD_STRING___PROTO__(thr), DUK_DELPROP_FLAG_THROW);
#endif

#if !defined(DUK_USE_ES6_OBJECT_SETPROTOTYPEOF)
	DUK_DD(DUK_DDPRINT("delete Object.setPrototypeOf built-in which is not enabled in features"));
	(void) duk_hobject_delprop_raw(thr, thr->builtins[DUK_BIDX_OBJECT_CONSTRUCTOR], DUK_HTHREAD_STRING_SET_PROTOTYPE_OF(thr), DUK_DELPROP_FLAG_THROW);
#endif

	duk_push_string(ctx,
			/* Endianness indicator */
#if defined(DUK_USE_INTEGER_LE)
	                "l"
#elif defined(DUK_USE_INTEGER_BE)
	                "b"
#elif defined(DUK_USE_INTEGER_ME)  /* integer mixed endian not really used now */
	                "m"
#else
	                "?"
#endif
#if defined(DUK_USE_DOUBLE_LE)
	                "l"
#elif defined(DUK_USE_DOUBLE_BE)
	                "b"
#elif defined(DUK_USE_DOUBLE_ME)
	                "m"
#else
	                "?"
#endif
#if defined(DUK_USE_BYTEORDER_FORCED)
			"f"
#endif
	                " "
			/* Packed or unpacked tval */
#if defined(DUK_USE_PACKED_TVAL)
	                "p"
#else
	                "u"
#endif
#if defined(DUK_USE_FASTINT)
			"f"
#endif
			" "
			/* Low memory options */
#if defined(DUK_USE_STRTAB_CHAIN)
			"c"  /* chain */
#elif defined(DUK_USE_STRTAB_PROBE)
			"p"  /* probe */
#else
			"?"
#endif
#if !defined(DUK_USE_HEAPPTR16) && !defined(DUK_DATAPTR16) && !defined(DUK_FUNCPTR16)
			"n"
#endif
#if defined(DUK_USE_HEAPPTR16)
			"h"
#endif
#if defined(DUK_USE_DATAPTR16)
			"d"
#endif
#if defined(DUK_USE_FUNCPTR16)
			"f"
#endif
#if defined(DUK_USE_REFCOUNT16)
			"R"
#endif
#if defined(DUK_USE_STRHASH16)
			"H"
#endif
#if defined(DUK_USE_STRLEN16)
			"S"
#endif
#if defined(DUK_USE_BUFLEN16)
			"B"
#endif
#if defined(DUK_USE_OBJSIZES16)
			"O"
#endif
#if defined(DUK_USE_LIGHTFUNC_BUILTINS)
			"L"
#endif
	                " "
			/* Object property allocation layout */
#if defined(DUK_USE_HOBJECT_LAYOUT_1)
			"p1"
#elif defined(DUK_USE_HOBJECT_LAYOUT_2)
			"p2"
#elif defined(DUK_USE_HOBJECT_LAYOUT_3)
			"p3"
#else
			"p?"
#endif
			" "
			/* Alignment guarantee */
#if defined(DUK_USE_ALIGN_4)
			"a4"
#elif defined(DUK_USE_ALIGN_8)
			"a8"
#else
			"a1"
#endif
			" "
			/* Architecture, OS, and compiler strings */
	                DUK_USE_ARCH_STRING
			" "
	                DUK_USE_OS_STRING
			" "
	                DUK_USE_COMPILER_STRING);
	duk_xdef_prop_stridx(ctx, DUK_BIDX_DUKTAPE, DUK_STRIDX_ENV, DUK_PROPDESC_FLAGS_WC);

	/*
	 *  InitJS code - Ecmascript code evaluated from a built-in source
	 *  which provides e.g. backward compatibility.  User can also provide
	 *  JS code to be evaluated at startup.
	 */

#ifdef DUK_USE_BUILTIN_INITJS
	/* XXX: compression */
	DUK_DD(DUK_DDPRINT("running built-in initjs"));
	duk_eval_string(ctx, (const char *) duk_initjs_data);  /* initjs data is NUL terminated */
	duk_pop(ctx);
#endif  /* DUK_USE_BUILTIN_INITJS */

#ifdef DUK_USE_USER_INITJS
	/* XXX: compression (as an option) */
	DUK_DD(DUK_DDPRINT("running user initjs"));
	duk_eval_string_noresult(ctx, (const char *) DUK_USE_USER_INITJS);
#endif  /* DUK_USE_USER_INITJS */

	/*
	 *  Since built-ins are not often extended, compact them.
	 */

	DUK_DD(DUK_DDPRINT("compact built-ins"));
	for (i = 0; i < DUK_NUM_BUILTINS; i++) {
		duk_hobject_compact_props(thr, thr->builtins[i]);
	}

	DUK_D(DUK_DPRINT("INITBUILTINS END"));

#ifdef DUK_USE_DDPRINT
	for (i = 0; i < DUK_NUM_BUILTINS; i++) {
		DUK_DD(DUK_DDPRINT("built-in object %ld after initialization and compacting: %!@iO",
		                   (long) i, (duk_heaphdr *) thr->builtins[i]));
	}
#endif

	/*
	 *  Pop built-ins from stack: they are now INCREF'd and
	 *  reachable from the builtins[] array.
	 */

	duk_pop_n(ctx, DUK_NUM_BUILTINS);
	DUK_ASSERT_TOP(ctx, 0);
}

DUK_INTERNAL void duk_hthread_copy_builtin_objects(duk_hthread *thr_from, duk_hthread *thr_to) {
	duk_small_uint_t i;

	for (i = 0; i < DUK_NUM_BUILTINS; i++) {
		thr_to->builtins[i] = thr_from->builtins[i];
		DUK_HOBJECT_INCREF_ALLOWNULL(thr_to, thr_to->builtins[i]);  /* side effect free */
	}
}
