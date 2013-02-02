/*
 *  Initialize built-in objects.  Current thread must have a valstack
 *  and initialization errors may longjmp, so a setjmp() catch point
 *  must exist.
 */

#include "duk_internal.h"

/*
 *  Helper union to convert between raw bytes and a double portably.
 */

typedef union {
	unsigned char b[8];
	double d;
} duk_double_and_bytes;

/*
 *  Encoding constants, must match genbuiltins.py
 */

#define  CLASS_BITS                  4
#define  BIDX_BITS                   6
#define  STRIDX_BITS                 9  /* FIXME: try to optimize to 8 */
#define  NATIDX_BITS                 8
#define  NUM_NORMAL_PROPS_BITS       6
#define  NUM_FUNC_PROPS_BITS         6
#define  PROP_FLAGS_BITS             3
#define  STRING_LENGTH_BITS          8
#define  STRING_CHAR_BITS            7
#define  LENGTH_PROP_BITS            3
#define  NARGS_BITS                  3
#define  PROP_TYPE_BITS              3

#define  NARGS_VARARGS_MARKER        0x07
#define  NO_CLASS_MARKER             0x00   /* 0 = DUK_HOBJECT_CLASS_UNUSED */
#define  NO_BIDX_MARKER              0x3f
#define  NO_STRIDX_MARKER            0xff

#define  PROP_TYPE_DOUBLE            0
#define  PROP_TYPE_STRING            1
#define  PROP_TYPE_STRIDX            2
#define  PROP_TYPE_BUILTIN           3
#define  PROP_TYPE_UNDEFINED         4
#define  PROP_TYPE_BOOLEAN_TRUE      5
#define  PROP_TYPE_BOOLEAN_FALSE     6

/*
 *  Create built-in objects by parsing an init bitstream generated
 *  by genbuiltins.py.
 */

void duk_hthread_create_builtin_objects(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
	duk_bitdecoder_ctx bd_ctx;
	duk_bitdecoder_ctx *bd = &bd_ctx;  /* convenience */
	duk_hobject *h;
	int i, j;

	DUK_DPRINT("INITBUILTINS BEGIN");

	memset(&bd_ctx, 0, sizeof(bd_ctx));
	bd->data = (duk_u8 *) duk_builtins_data;
	bd->length = DUK_BUILTINS_DATA_LENGTH;

	/*
	 *  First create all built-in bare objects on the empty valstack.
	 *  During init, their indices will correspond to built-in indices.
	 *
	 *  Built-ins will be reachable from both valstack and thr->builtins.
	 */

	DUK_DDPRINT("create empty built-ins");
	DUK_ASSERT(duk_get_top(ctx) == 0);
	for (i = 0; i < DUK_NUM_BUILTINS; i++) {
		int class_num;
		int len = -1;

		class_num = duk_bd_decode(bd, CLASS_BITS);

		if (duk_bd_decode_flag(bd)) {
			len = duk_bd_decode(bd, LENGTH_PROP_BITS);
		}

		if (class_num == DUK_HOBJECT_CLASS_FUNCTION) {
			int natidx;
			int c_nargs;
			duk_c_function c_func;

			DUK_DDDPRINT("len=%d", len);
			DUK_ASSERT(len >= 0);

			natidx = duk_bd_decode(bd, NATIDX_BITS);
			c_func = duk_builtin_native_functions[natidx];

			c_nargs = len;
			if (duk_bd_decode_flag(bd)) {
				c_nargs = duk_bd_decode(bd, NARGS_BITS);
				if (c_nargs == NARGS_VARARGS_MARKER) {
					c_nargs = DUK_VARARGS;
				}
			}

			duk_push_new_c_function(ctx, c_func, c_nargs);

			h = duk_require_hobject(ctx, -1);
			DUK_ASSERT(h != NULL);

			/* Almost all global level Function objects are constructable
			 * but not all: Function.prototype is a non-constructable,
			 * callable Function.
			 */
			if (duk_bd_decode_flag(bd)) {
				DUK_HOBJECT_SET_CONSTRUCTABLE(h);
			}
		} else {
			/* FIXME: ARRAY_PART for Array prototype? */

			duk_push_new_object_helper(ctx,
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
			duk_def_prop_stridx(ctx,
			                    -2,
			                    DUK_HEAP_STRIDX_LENGTH,
			                    (class_num == DUK_HOBJECT_CLASS_ARRAY ?  /* only Array.prototype matches */
			                     DUK_PROPDESC_FLAGS_W : DUK_PROPDESC_FLAGS_NONE));
		}

		/* enable special behaviors last */

		if (class_num == DUK_HOBJECT_CLASS_ARRAY) {
			DUK_HOBJECT_SET_SPECIAL_ARRAY(h);
		}
		if (class_num == DUK_HOBJECT_CLASS_STRING) {
			DUK_HOBJECT_SET_SPECIAL_STRINGOBJ(h);
		}

		/* some assertions */

		DUK_ASSERT(DUK_HOBJECT_HAS_EXTENSIBLE(h));
		/* DUK_HOBJECT_FLAG_CONSTRUCTABLE varies */
		DUK_ASSERT(!DUK_HOBJECT_HAS_BOUND(h));
		DUK_ASSERT(!DUK_HOBJECT_HAS_COMPILEDFUNCTION(h));
		/* DUK_HOBJECT_FLAG_NATIVEFUNCTION varies */
		DUK_ASSERT(!DUK_HOBJECT_HAS_THREAD(h));
		DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_PART(h));       /* currently, even for Array.prototype */
		DUK_ASSERT(!DUK_HOBJECT_HAS_STRICT(h));
		DUK_ASSERT(!DUK_HOBJECT_HAS_NATIVEFUNCTION(h) ||  /* all native functions have NEWENV */
		           DUK_HOBJECT_HAS_NEWENV(h));
		DUK_ASSERT(!DUK_HOBJECT_HAS_NAMEBINDING(h));
		DUK_ASSERT(!DUK_HOBJECT_HAS_CREATEARGS(h));
		DUK_ASSERT(!DUK_HOBJECT_HAS_ENVRECCLOSED(h));
		/* DUK_HOBJECT_FLAG_SPECIAL_ARRAY varies */
		/* DUK_HOBJECT_FLAG_SPECIAL_STRINGOBJ varies */
		DUK_ASSERT(!DUK_HOBJECT_HAS_SPECIAL_ARGUMENTS(h));

		DUK_DDDPRINT("created built-in %d, class=%d, length=%d", i, class_num, len);
	}

	/*
	 *  Then decode the builtins init data (see genbuiltins.py) to
	 *  init objects
	 */

	DUK_DDPRINT("initialize built-in object properties");
	for (i = 0; i < DUK_NUM_BUILTINS; i++) {
		unsigned char t;
		int num;

		DUK_DDDPRINT("initializing built-in object at index %d", i);
		h = thr->builtins[i];

		t = duk_bd_decode(bd, BIDX_BITS);
		if (t != NO_BIDX_MARKER) {
			DUK_DDDPRINT("set prototype: built-in %d", (int) t);
			DUK_HOBJECT_SET_PROTOTYPE(thr, h, thr->builtins[t]);
		}

		t = duk_bd_decode(bd, BIDX_BITS);
		if (t != NO_BIDX_MARKER) {
			/* 'prototype' property for all built-in objects (which have it) has attributes:
			 *  [[Writable]] = false,
			 *  [[Enumerable]] = false,
			 *  [[Configurable]] = false
			 */
			DUK_DDDPRINT("set external prototype: built-in %d", (int) t);
			duk_def_prop_stridx_builtin(ctx, i, DUK_HEAP_STRIDX_PROTOTYPE, t, DUK_PROPDESC_FLAGS_NONE);
		}

		t = duk_bd_decode(bd, BIDX_BITS);
		if (t != NO_BIDX_MARKER) {
			/* 'constructor' property for all built-in objects (which have it) has attributes:
			 *  [[Writable]] = true,
			 *  [[Enumerable]] = false,	
			 *  [[Configurable]] = true
			 */
			DUK_DDDPRINT("set external constructor: built-in %d", (int) t);
			duk_def_prop_stridx_builtin(ctx, i, DUK_HEAP_STRIDX_CONSTRUCTOR, t, DUK_PROPDESC_FLAGS_WC);
		}

		/* normal valued properties */
		num = duk_bd_decode(bd, NUM_NORMAL_PROPS_BITS);
		DUK_DDDPRINT("built-in object %d, %d normal valued properties", i, num);
		for (j = 0; j < num; j++) {
			int stridx;
			int prop_flags;

			stridx = duk_bd_decode(bd, STRIDX_BITS);

			/*
			 *  Property attribute defaults are defined in E5 Section 15 (first
			 *  few pages); there is a default for all properties and a special
			 *  default for 'length' properties.  Variation from the defaults is
			 *  signaled using a single flag bit in the bitstream.
			 */

			if (duk_bd_decode_flag(bd)) {
				prop_flags = duk_bd_decode(bd, PROP_FLAGS_BITS);
			} else {
				if (stridx == DUK_HEAP_STRIDX_LENGTH) {
					prop_flags = DUK_PROPDESC_FLAGS_NONE;
				} else {
					prop_flags = DUK_PROPDESC_FLAGS_WC;
				}
			}

			t = duk_bd_decode(bd, PROP_TYPE_BITS);

			DUK_DDDPRINT("built-in %d, normal-valued property %d, stridx %d, flags 0x%02x, type %d",
			             i, j, stridx, prop_flags, (int) t);

			switch (t) {
			case PROP_TYPE_DOUBLE: {
				duk_double_and_bytes tmp;
				int k;

				for (k = 0; k < 8; k++) {
					/* Encoding endianness must match target memory layout,
					 * build scripts and genbuiltins.py must ensure this.
					 */
					tmp.b[k] = duk_bd_decode(bd, 8);
				}

				duk_push_number(ctx, tmp.d);  /* push operation normalizes NaNs */
				break;
			}
			case PROP_TYPE_STRING: {
				int n;
				int k;
				char *p;

				n = duk_bd_decode(bd, STRING_LENGTH_BITS);
				p = (char *) duk_push_new_fixed_buffer(ctx, n);
				for (k = 0; k < n; k++) {
					*p++ = duk_bd_decode(bd, STRING_CHAR_BITS);
				}

				duk_to_string(ctx, -1);
				break;
			}
			case PROP_TYPE_STRIDX: {
				int n;

				n = duk_bd_decode(bd, STRIDX_BITS);
				DUK_ASSERT(n >= 0 && n < DUK_HEAP_NUM_STRINGS);
				duk_push_hstring_stridx(ctx, n);
				break;
			}
			case PROP_TYPE_BUILTIN: {
				int bidx;

				bidx = duk_bd_decode(bd, BIDX_BITS);
				DUK_ASSERT(bidx != NO_BIDX_MARKER);
				duk_dup(ctx, bidx);
				break;
			}
			case PROP_TYPE_UNDEFINED: {
				duk_push_undefined(ctx);
				break;
			}
			case PROP_TYPE_BOOLEAN_TRUE: {
				duk_push_true(ctx);
				break;
			}
			case PROP_TYPE_BOOLEAN_FALSE: {
				duk_push_false(ctx);
				break;
			}
			default: {
				/* exhaustive */
				DUK_NEVER_HERE();
			}
			}

			duk_def_prop_stridx(ctx, i, stridx, prop_flags);
		}

		/* native function properties */
		num = duk_bd_decode(bd, NUM_FUNC_PROPS_BITS);
		DUK_DDDPRINT("built-in object %d, %d function valued properties", i, num);
		for (j = 0; j < num; j++) {
			int stridx;
			int natidx;
			int c_nargs;
			int c_length;
			duk_c_function c_func;
			duk_hnativefunction *h_func;

			stridx = duk_bd_decode(bd, STRIDX_BITS);
			natidx = duk_bd_decode(bd, NATIDX_BITS);

			c_length = duk_bd_decode(bd, LENGTH_PROP_BITS);
			c_nargs = c_length;
			if (duk_bd_decode_flag(bd)) {
				c_nargs = duk_bd_decode(bd, NARGS_BITS);
				if (c_nargs == NARGS_VARARGS_MARKER) {
					c_nargs = DUK_VARARGS;
				}
			}

			c_func = duk_builtin_native_functions[natidx];

			DUK_DDDPRINT("built-in %d, function-valued property %d, stridx %d, natidx %d, length %d, nargs %d",
			             i, j, stridx, natidx, c_length, (c_nargs == DUK_VARARGS ? -1 : c_nargs));


			/* [ (builtin objects) ] */

			duk_push_new_c_function(ctx, c_func, c_nargs);
			h_func = duk_require_hnativefunction(ctx, -1);
			h_func = h_func;  /* suppress warning (not referenced now) */

			/* Currently all built-in native functions are strict.
			 * This doesn't matter for many functions, but e.g.
			 * String.prototype.charAt (and other string functions)
			 * rely on being strict so that their 'this' binding is
			 * not automatically coerced.
			 */
			DUK_HOBJECT_SET_STRICT((duk_hobject *) h_func);

			/* [ (builtin objects) func ] */

			duk_push_int(ctx, c_length);
			duk_def_prop_stridx(ctx, -2, DUK_HEAP_STRIDX_LENGTH, DUK_PROPDESC_FLAGS_NONE);

			/* FIXME: other properties of function instances; 'name', 'arguments', 'caller'. */

			DUK_DDPRINT("built-in object %d, function property %d -> %!T", i, j, duk_get_tval(ctx, -1));

			/* [ (builtin objects) func ] */

			/*
			 *  The default property attributes are correct for all
			 *  function valued properties of built-in objects now.
			 */

			duk_def_prop_stridx(ctx, i, stridx, DUK_PROPDESC_FLAGS_WC);

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
	 */

	duk_get_prop_stridx(ctx, DUK_BIDX_DATE_PROTOTYPE, DUK_HEAP_STRIDX_TO_UTC_STRING);
	duk_def_prop_stridx(ctx, DUK_BIDX_DATE_PROTOTYPE, DUK_HEAP_STRIDX_TO_GMT_STRING, DUK_PROPDESC_FLAGS_WC);

	h = duk_require_hobject(ctx, DUK_BIDX_DOUBLE_ERROR);
	DUK_ASSERT(h != NULL);
	DUK_HOBJECT_CLEAR_EXTENSIBLE(h);

	/*
	 *  Since built-ins are not often extended, compact them.
	 */

	DUK_DDPRINT("compact built-ins");
	for (i = 0; i < DUK_NUM_BUILTINS; i++) {
		duk_hobject_compact_props(thr, thr->builtins[i]);
	}

	DUK_DPRINT("INITBUILTINS END");

#ifdef DUK_USE_DDEBUG
	for (i = 0; i < DUK_NUM_BUILTINS; i++) {
		DUK_DDPRINT("built-in object %d after initialization and compacting: %!@iO", i, thr->builtins[i]);
	}
#endif
	
#ifdef DUK_USE_DDDEBUG
	for (i = 0; i < DUK_NUM_BUILTINS; i++) {
		DUK_DDDPRINT("built-in object %d after initialization and compacting", i);
		DUK_DEBUG_DUMP_HOBJECT(thr->builtins[i]);
	}
#endif

	/*
	 *  Pop built-ins from stack: they are now INCREF'd and
	 *  reachable from the builtins[] array.
	 */

	duk_pop_n(ctx, DUK_NUM_BUILTINS);
	DUK_ASSERT(duk_get_top(ctx) == 0);
}

void duk_hthread_copy_builtin_objects(duk_hthread *thr_from, duk_hthread *thr_to) {
	int i;

	for (i = 0; i < DUK_NUM_BUILTINS; i++) {
		thr_to->builtins[i] = thr_from->builtins[i];
		DUK_HOBJECT_INCREF(thr, thr_to->builtins[i]);  /* side effect free */
	}
}

