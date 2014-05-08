/*
 *  Custom formatter for debug printing, allowing Duktape specific data
 *  structures (such as tagged values and heap objects) to be printed with
 *  a nice format string.  Because debug printing should not affect execution
 *  state, formatting here must be independent of execution (see implications
 *  below) and must not allocate memory.
 *
 *  Custom format tags begin with a '%!' to safely distinguish them from
 *  standard format tags.  The following conversions are supported:
 *
 *     %!T    tagged value (duk_tval *)
 *     %!O    heap object (duk_heaphdr *)
 *     %!I    decoded bytecode instruction
 *     %!C    bytecode instruction opcode name
 *
 *  Everything is serialized in a JSON-like manner.  The default depth is one
 *  level, internal prototype is not followed, and internal properties are not
 *  serialized.  The following modifiers change this behavior:
 *
 *     @      print pointers
 *     #      print binary representations (where applicable)
 *     d      deep traversal of own properties (not prototype)
 *     p      follow prototype chain (useless without 'd')
 *     i      include internal properties (other than prototype)
 *     x      hexdump buffers
 *     h      heavy formatting
 *
 *  For instance, the following serializes objects recursively, but does not
 *  follow the prototype chain nor print internal properties: "%!dO".
 *
 *  Notes:
 *
 *    * Standard snprintf return value semantics seem to vary.  This
 *      implementation returns the number of bytes it actually wrote
 *      (excluding the null terminator).  If retval == buffer size,
 *      output was truncated (except for corner cases).
 *
 *    * Output format is intentionally different from Ecmascript
 *      formatting requirements, as formatting here serves debugging
 *      of internals.
 *
 *    * Depth checking (and updating) is done in each type printer
 *      separately, to allow them to call each other freely.
 *
 *    * Some pathological structures might take ages to print (e.g.
 *      self recursion with 100 properties pointing to the object
 *      itself).  To guard against these, each printer also checks
 *      whether the output buffer is full; if so, early exit.
 *
 *    * Reference loops are detected using a loop stack.
 */

#include "duk_internal.h"

#ifdef DUK_USE_DEBUG

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* list of conversion specifiers that terminate a format tag;
 * this is unfortunately guesswork.
 */
#define DUK__ALLOWED_STANDARD_SPECIFIERS  "diouxXeEfFgGaAcsCSpnm"

/* maximum length of standard format tag that we support */
#define DUK__MAX_FORMAT_TAG_LENGTH  32

/* heapobj recursion depth when deep printing is selected */
#define DUK__DEEP_DEPTH_LIMIT  8

/* maximum recursion depth for loop detection stacks */
#define DUK__LOOP_STACK_DEPTH  256

/* must match bytecode defines now; build autogenerate? */
static const char *duk__bc_optab[] = {
	"LDREG",    "STREG",    "LDCONST",  "LDINT",    "LDINTX",   "MPUTOBJ",  "MPUTOBJI", "MPUTARR",  "MPUTARRI", "NEW",
	"NEWI",     "REGEXP", 	"CSREG",    "CSREGI",   "GETVAR",   "PUTVAR",   "DECLVAR",  "DELVAR",   "CSVAR",    "CSVARI",
	"CLOSURE",  "GETPROP", 	"PUTPROP",  "DELPROP",  "CSPROP",   "CSPROPI",  "ADD",      "SUB",      "MUL",      "DIV",
	"MOD",      "BAND",     "BOR",      "BXOR",     "BASL",     "BLSR", 	"BASR",     "BNOT", 	"LNOT",     "EQ",
	"NEQ",      "SEQ",      "SNEQ",     "GT",       "GE",       "LT",       "LE",       "IF", 	"INSTOF",   "IN",
	"JUMP",     "RETURN",   "CALL",     "CALLI",    "LABEL",    "ENDLABEL", "BREAK",    "CONTINUE", "TRYCATCH", "UNUSED59",
	"UNUSED60", "EXTRA",    "DEBUG",    "INVALID",
};

static const char *duk__bc_extraoptab[] = {
	"NOP", "LDTHIS", "LDUNDEF", "LDNULL", "LDTRUE", "LDFALSE", "NEWOBJ", "NEWARR", "SETALEN", "TYPEOF",
	"TYPEOFID", "TONUM", "INITENUM", "NEXTENUM", "INITSET", "INITSETI", "INITGET", "INITGETI", "ENDTRY", "ENDCATCH",
	"ENDFIN", "THROW", "INVLHS", "UNM", "UNP", "INC", "DEC", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",

	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",

	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",

	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",

	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX", "XXX",

	"XXX", "XXX", "XXX", "XXX", "XXX", "XXX",
};

typedef struct duk__dprint_state duk__dprint_state;
struct duk__dprint_state {
	duk_fixedbuffer *fb;

	/* loop_stack_index could be perhaps be replaced by 'depth', but it's nice
	 * to not couple these two mechanisms unnecessarily.
	 */
	duk_hobject *loop_stack[DUK__LOOP_STACK_DEPTH];
	int loop_stack_index;
	int loop_stack_limit;

	int depth;
	int depth_limit;

	int pointer;
	int heavy;
	int binary;
	int follow_proto;
	int internal;
	int hexdump;
};

/* helpers */
static void duk__print_hstring(duk__dprint_state *st, duk_hstring *k, int quotes);
static void duk__print_hobject(duk__dprint_state *st, duk_hobject *h);
static void duk__print_hbuffer(duk__dprint_state *st, duk_hbuffer *h);
static void duk__print_tval(duk__dprint_state *st, duk_tval *tv);
static void duk__print_instr(duk__dprint_state *st, duk_instr ins);
static void duk__print_heaphdr(duk__dprint_state *st, duk_heaphdr *h);
static void duk__print_shared_heaphdr(duk__dprint_state *st, duk_heaphdr *h);
static void duk__print_shared_heaphdr_string(duk__dprint_state *st, duk_heaphdr_string *h);

static void duk__print_shared_heaphdr(duk__dprint_state *st, duk_heaphdr *h) {
	duk_fixedbuffer *fb = st->fb;

	if (st->heavy) {
		duk_fb_sprintf(fb, "(%p)", (void *) h);
	}

	if (!h) {
		return;
	}

	if (st->binary) {
		duk_size_t i;
		duk_fb_put_byte(fb, (duk_uint8_t) '[');
		for (i = 0; i < (duk_size_t) sizeof(*h); i++) {
			duk_fb_sprintf(fb, "%02x", (int) ((unsigned char *)h)[i]);
		}
		duk_fb_put_byte(fb, (duk_uint8_t) ']');
	}

#ifdef DUK_USE_REFERENCE_COUNTING  /* currently implicitly also DUK_USE_DOUBLE_LINKED_HEAP */
	if (st->heavy) {
		duk_fb_sprintf(fb, "[h_next=%p,h_prev=%p,h_refcount=%u,h_flags=%08x,type=%d,reachable=%d,temproot=%d,finalizable=%d,finalized=%d]",
		               DUK_HEAPHDR_GET_NEXT(h),
		               DUK_HEAPHDR_GET_PREV(h),
		               DUK_HEAPHDR_GET_REFCOUNT(h),
		               DUK_HEAPHDR_GET_FLAGS(h),
		               DUK_HEAPHDR_GET_TYPE(h),
		               DUK_HEAPHDR_HAS_REACHABLE(h),
		               DUK_HEAPHDR_HAS_TEMPROOT(h),
		               DUK_HEAPHDR_HAS_FINALIZABLE(h),
		               DUK_HEAPHDR_HAS_FINALIZED(h));
	}
#else
	if (st->heavy) {
		duk_fb_sprintf(fb, "[h_next=%p,h_flags=%08x,type=%d,reachable=%d,temproot=%d,finalizable=%d,finalized=%d]",
		               DUK_HEAPHDR_GET_NEXT(h),
	        	       DUK_HEAPHDR_GET_FLAGS(h),
		               DUK_HEAPHDR_GET_TYPE(h),
		               DUK_HEAPHDR_HAS_REACHABLE(h),
	        	       DUK_HEAPHDR_HAS_TEMPROOT(h),
		               DUK_HEAPHDR_HAS_FINALIZABLE(h),
		               DUK_HEAPHDR_HAS_FINALIZED(h));
	}
#endif
}

static void duk__print_shared_heaphdr_string(duk__dprint_state *st, duk_heaphdr_string *h) {
	duk_fixedbuffer *fb = st->fb;

	if (st->heavy) {
		duk_fb_sprintf(fb, "(%p)", (void *) h);
	}

	if (!h) {
		return;
	}

	if (st->binary) {
		duk_size_t i;
		duk_fb_put_byte(fb, (duk_uint8_t) '[');
		for (i = 0; i < (duk_size_t) sizeof(*h); i++) {
			duk_fb_sprintf(fb, "%02x", (int) ((unsigned char *)h)[i]);
		}
		duk_fb_put_byte(fb, (duk_uint8_t) ']');
	}

#ifdef DUK_USE_REFERENCE_COUNTING
	if (st->heavy) {
		duk_fb_sprintf(fb, "[h_refcount=%u,h_flags=%08x,type=%d,reachable=%d,temproot=%d,finalizable=%d,finalized=%d]",
		               DUK_HEAPHDR_GET_REFCOUNT((duk_heaphdr *) h),
		               DUK_HEAPHDR_GET_FLAGS((duk_heaphdr *) h),
		               DUK_HEAPHDR_GET_TYPE((duk_heaphdr *) h),
		               DUK_HEAPHDR_HAS_REACHABLE((duk_heaphdr *) h),
		               DUK_HEAPHDR_HAS_TEMPROOT((duk_heaphdr *) h),
		               DUK_HEAPHDR_HAS_FINALIZABLE((duk_heaphdr *) h),
		               DUK_HEAPHDR_HAS_FINALIZED((duk_heaphdr *) h));
	}
#else
	if (st->heavy) {
		duk_fb_sprintf(fb, "[h_flags=%08x,type=%d,reachable=%d,temproot=%d,finalizable=%d,finalized=%d]",
	        	       DUK_HEAPHDR_GET_FLAGS((duk_heaphdr *) h),
		               DUK_HEAPHDR_GET_TYPE((duk_heaphdr *) h),
		               DUK_HEAPHDR_HAS_REACHABLE((duk_heaphdr *) h),
	        	       DUK_HEAPHDR_HAS_TEMPROOT((duk_heaphdr *) h),
		               DUK_HEAPHDR_HAS_FINALIZABLE((duk_heaphdr *) h),
		               DUK_HEAPHDR_HAS_FINALIZED((duk_heaphdr *) h));
	}
#endif
}

static void duk__print_hstring(duk__dprint_state *st, duk_hstring *h, int quotes) {
	duk_fixedbuffer *fb = st->fb;
	duk_uint8_t *p;
	duk_uint8_t *p_end;

	/* terminal type: no depth check */

	if (duk_fb_is_full(fb)) {
		return;
	}

	duk__print_shared_heaphdr_string(st, &h->hdr);

	if (!h) {
		duk_fb_put_cstring(fb, "NULL");
		return;
	}

	p = DUK_HSTRING_GET_DATA(h);
	p_end = p + DUK_HSTRING_GET_BYTELEN(h);

	if (p_end > p && p[0] == '_') {
		/* if property key begins with underscore, encode it with
		 * forced quotes (e.g. "_foo") to distinguish it from encoded
		 * internal properties (e.g. \xffbar -> _bar).
		 */
		quotes = 1;
	}

	if (quotes) {
		duk_fb_put_byte(fb, (duk_uint8_t) '"');
	}
	while (p < p_end) {
		duk_uint8_t ch = *p++;

		/* two special escapes: '\' and '"', other printables as is */
		if (ch == '\\') {
			duk_fb_sprintf(fb, "\\\\");
		} else if (ch == '"') {
			duk_fb_sprintf(fb, "\\\"");
		} else if (ch >= 0x20 && ch <= 0x7e) {
			duk_fb_put_byte(fb, ch);
		} else if (ch == 0xff && !quotes) {
			/* encode \xffbar as _bar if no quotes are applied, this is for
			 * readable internal keys.
			 */
			duk_fb_put_byte(fb, (duk_uint8_t) '_');
		} else {
			duk_fb_sprintf(fb, "\\x%02x", (int) ch);
		}
	}
	if (quotes) {
		duk_fb_put_byte(fb, (duk_uint8_t) '"');
	}
#ifdef DUK_USE_REFERENCE_COUNTING
	/* XXX: limit to quoted strings only, to save keys from being cluttered? */
	duk_fb_sprintf(fb, "/%d", DUK_HEAPHDR_GET_REFCOUNT(&h->hdr));
#endif
}

#ifdef DUK__COMMA
#undef DUK__COMMA
#endif
#define DUK__COMMA()  do { \
		if (first) { \
			first = 0; \
		} else { \
			duk_fb_put_byte(fb, (duk_uint8_t) ','); \
		} \
	} while (0)

static void duk__print_hobject(duk__dprint_state *st, duk_hobject *h) {
	duk_fixedbuffer *fb = st->fb;
	duk_uint_fast32_t i;
	duk_tval *tv;
	duk_hstring *key;
	int first = 1;
	char *brace1 = "{";
	char *brace2 = "}";
	int pushed_loopstack = 0;

	if (duk_fb_is_full(fb)) {
		return;
	}

	duk__print_shared_heaphdr(st, &h->hdr);

	if (h && DUK_HOBJECT_HAS_ARRAY_PART(h)) {
		brace1 = "[";
		brace2 = "]";
	}

	if (!h) {
		duk_fb_put_cstring(fb, "NULL");
		goto finished;
	}

	if (st->depth >= st->depth_limit) {
		if (DUK_HOBJECT_IS_COMPILEDFUNCTION(h)) {
			duk_fb_sprintf(fb, "%sobject/compiledfunction %p%s", brace1, (void *) h, brace2);
		} else if (DUK_HOBJECT_IS_NATIVEFUNCTION(h)) {
			duk_fb_sprintf(fb, "%sobject/nativefunction %p%s", brace1, (void *) h, brace2);
		} else if (DUK_HOBJECT_IS_THREAD(h)) {
			duk_fb_sprintf(fb, "%sobject/thread %p%s", brace1, (void *) h, brace2);
		} else {
			duk_fb_sprintf(fb, "%sobject %p%s", brace1, (void *) h, brace2);  /* may be NULL */
		}
		return;
	}

	for (i = 0; i < (duk_uint_fast32_t) st->loop_stack_index; i++) {
		if (st->loop_stack[i] == h) {
			duk_fb_sprintf(fb, "%sLOOP:%p%s", brace1, (void *) h, brace2);
			return;
		}
	}

	/* after this, return paths should 'goto finished' for decrement */
	st->depth++;

	if (st->loop_stack_index >= st->loop_stack_limit) {
		duk_fb_sprintf(fb, "%sOUT-OF-LOOP-STACK%s", brace1, brace2);
		goto finished;
	}
	st->loop_stack[st->loop_stack_index++] = h;
	pushed_loopstack = 1;

	/*
	 *  Notation: double underscore used for internal properties which are not
	 *  stored in the property allocation (e.g. '__valstack').
	 */

	duk_fb_put_cstring(fb, brace1);

	if (h->p) {
		duk_uint32_t a_limit;

		a_limit = h->a_size;
		if (st->internal) {
			/* dump all allocated entries, unused entries print as 'unused',
			 * note that these may extend beyond current 'length' and look
			 * a bit funny.
			 */
		} else {
			/* leave out trailing 'unused' elements */
			while (a_limit > 0) {
				tv = DUK_HOBJECT_A_GET_VALUE_PTR(h, a_limit - 1);
				if (!DUK_TVAL_IS_UNDEFINED_UNUSED(tv)) {
					break;
				}
				a_limit--;
			}
		}

		for (i = 0; i < a_limit; i++) {
			tv = DUK_HOBJECT_A_GET_VALUE_PTR(h, i);
			DUK__COMMA();
			duk__print_tval(st, tv);
		}
		for (i = 0; i < h->e_used; i++) {
			key = DUK_HOBJECT_E_GET_KEY(h, i);
			if (!key) {
				continue;
			}
			if (!st->internal &&
			    DUK_HSTRING_GET_BYTELEN(key) > 0 &&
			    DUK_HSTRING_GET_DATA(key)[0] == 0xff) {
				/* FIXME: cleanup to use DUK_HSTRING_FLAG_INTERNAL? */
				continue;
			}
			DUK__COMMA();
			duk__print_hstring(st, key, 0);
			duk_fb_put_byte(fb, (duk_uint8_t) ':');
			if (DUK_HOBJECT_E_SLOT_IS_ACCESSOR(h, i)) {
				duk_fb_sprintf(fb, "[get:%p,set:%p]",
				               DUK_HOBJECT_E_GET_VALUE(h, i).a.get,
				               DUK_HOBJECT_E_GET_VALUE(h, i).a.set);
			} else {
				tv = &DUK_HOBJECT_E_GET_VALUE(h, i).v;
				duk__print_tval(st, tv);
			}
			if (st->heavy) {
				duk_fb_sprintf(fb, "<%02x>", (int) DUK_HOBJECT_E_GET_FLAGS(h, i));
			}
		}
	}
	if (st->internal) {
		if (DUK_HOBJECT_HAS_EXTENSIBLE(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__extensible:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_CONSTRUCTABLE(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__constructable:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_BOUND(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__bound:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_COMPILEDFUNCTION(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__compiledfunction:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_NATIVEFUNCTION(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__nativefunction:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_THREAD(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__thread:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_ARRAY_PART(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__array_part:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_STRICT(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__strict:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_NEWENV(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__newenv:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_NAMEBINDING(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__namebinding:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_CREATEARGS(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__createargs:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_ENVRECCLOSED(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__envrecclosed:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_EXOTIC_ARRAY(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__special_array:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_EXOTIC_STRINGOBJ(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__special_stringobj:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_EXOTIC_ARGUMENTS(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__special_arguments:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_EXOTIC_DUKFUNC(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__special_dukfunc:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_EXOTIC_BUFFEROBJ(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__special_bufferobj:true");
		} else {
			;
		}
		if (DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(h)) {
			DUK__COMMA(); duk_fb_sprintf(fb, "__special_proxyobj:true");
		} else {
			;
		}
	}
	if (st->internal && DUK_HOBJECT_IS_COMPILEDFUNCTION(h)) {
		duk_hcompiledfunction *f = (duk_hcompiledfunction *) h;
		DUK__COMMA(); duk_fb_put_cstring(fb, "__data:"); duk__print_hbuffer(st, f->data);
		DUK__COMMA(); duk_fb_sprintf(fb, "__nregs:%d", f->nregs);
		DUK__COMMA(); duk_fb_sprintf(fb, "__nargs:%d", f->nargs);
	} else if (st->internal && DUK_HOBJECT_IS_NATIVEFUNCTION(h)) {
		duk_hnativefunction *f = (duk_hnativefunction *) h;
#if 0  /* FIXME: no portable way to print function pointers */
		DUK__COMMA(); duk_fb_sprintf(fb, "__func:%p", (void *) f->func);
#endif
		DUK__COMMA(); duk_fb_sprintf(fb, "__nargs:%d", f->nargs);

	} else if (st->internal && DUK_HOBJECT_IS_THREAD(h)) {
		duk_hthread *t = (duk_hthread *) h;
		DUK__COMMA(); duk_fb_sprintf(fb, "__strict:%d", t->strict);
		DUK__COMMA(); duk_fb_sprintf(fb, "__state:%d", t->state);
		DUK__COMMA(); duk_fb_sprintf(fb, "__unused1:%d", t->unused1);
		DUK__COMMA(); duk_fb_sprintf(fb, "__unused2:%d", t->unused2);
		DUK__COMMA(); duk_fb_sprintf(fb, "__valstack_max:%d", t->valstack_max);
		DUK__COMMA(); duk_fb_sprintf(fb, "__callstack_max:%d", t->callstack_max);
		DUK__COMMA(); duk_fb_sprintf(fb, "__catchstack_max:%d", t->catchstack_max);
		DUK__COMMA(); duk_fb_sprintf(fb, "__valstack:%p", (void *) t->valstack);
		DUK__COMMA(); duk_fb_sprintf(fb, "__valstack_end:%p/%d", (void *) t->valstack_end, (int) (t->valstack_end - t->valstack));
		DUK__COMMA(); duk_fb_sprintf(fb, "__valstack_bottom:%p/%d", (void *) t->valstack_bottom, (int) (t->valstack_bottom - t->valstack));
		DUK__COMMA(); duk_fb_sprintf(fb, "__valstack_top:%p/%d", (void *) t->valstack_top, (int) (t->valstack_top - t->valstack));
		DUK__COMMA(); duk_fb_sprintf(fb, "__catchstack:%p", (void *) t->catchstack);
		DUK__COMMA(); duk_fb_sprintf(fb, "__catchstack_size:%d", t->catchstack_size);
		DUK__COMMA(); duk_fb_sprintf(fb, "__catchstack_top:%d", t->catchstack_top);
		DUK__COMMA(); duk_fb_sprintf(fb, "__resumer:"); duk__print_hobject(st, (duk_hobject *) t->resumer);
		/* XXX: print built-ins array? */

	}
#ifdef DUK_USE_REFERENCE_COUNTING
	if (st->internal) {
		DUK__COMMA(); duk_fb_sprintf(fb, "__refcount:%d", DUK_HEAPHDR_GET_REFCOUNT((duk_heaphdr *) h));
	}
#endif
	if (st->internal) {
		DUK__COMMA(); duk_fb_sprintf(fb, "__class:%d", DUK_HOBJECT_GET_CLASS_NUMBER(h));
	}

	/* prototype should be last, for readability */
	if (st->follow_proto && h->prototype) {
		DUK__COMMA(); duk_fb_put_cstring(fb, "__prototype:"); duk__print_hobject(st, h->prototype);
	}

	duk_fb_put_cstring(fb, brace2);

	if (st->heavy && h->h_size > 0) {
		duk_fb_put_byte(fb, (duk_uint8_t) '<');
		for (i = 0; i < h->h_size; i++) {
			duk_uint32_t h_idx = DUK_HOBJECT_H_GET_INDEX(h, i);
			if (i > 0) {
				duk_fb_put_byte(fb, (duk_uint8_t) ',');
			}
			if (h_idx == DUK_HOBJECT_HASHIDX_UNUSED) {
				duk_fb_sprintf(fb, "u");
			} else if (h_idx == DUK_HOBJECT_HASHIDX_DELETED) {
				duk_fb_sprintf(fb, "d");
			} else {
				duk_fb_sprintf(fb, "%d", (int) h_idx);
			}
		}
		duk_fb_put_byte(fb, (duk_uint8_t) '>');
	}

 finished:
	st->depth--;
	if (pushed_loopstack) {
		st->loop_stack_index--;
		st->loop_stack[st->loop_stack_index] = NULL;
	}
}

#undef DUK__COMMA

static void duk__print_hbuffer(duk__dprint_state *st, duk_hbuffer *h) {
	duk_fixedbuffer *fb = st->fb;
	size_t i, n;
	duk_uint8_t *p;

	if (duk_fb_is_full(fb)) {
		return;
	}

	/* terminal type: no depth check */

	if (!h) {
		duk_fb_put_cstring(fb, "NULL");
		return;
	}

	if (DUK_HBUFFER_HAS_DYNAMIC(h)) {
		duk_hbuffer_dynamic *g = (duk_hbuffer_dynamic *) h;
		duk_fb_sprintf(fb, "buffer:dynamic:%p:%d:%d",
		               g->curr_alloc, g->size, g->usable_size);
	} else {
		duk_fb_sprintf(fb, "buffer:fixed:%d", DUK_HBUFFER_GET_SIZE(h));
	}

#ifdef DUK_USE_REFERENCE_COUNTING
	duk_fb_sprintf(fb, "/%d", DUK_HEAPHDR_GET_REFCOUNT(&h->hdr));
#endif

	if (st->hexdump) {
		duk_fb_sprintf(fb, "=[");
		n = DUK_HBUFFER_GET_SIZE(h);
		p = (duk_uint8_t *) DUK_HBUFFER_GET_DATA_PTR(h);
		for (i = 0; i < n; i++) {
			duk_fb_sprintf(fb, "%02x", (int) p[i]);
		}
		duk_fb_sprintf(fb, "]");
	}
}

static void duk__print_heaphdr(duk__dprint_state *st, duk_heaphdr *h) {
	duk_fixedbuffer *fb = st->fb;

	if (duk_fb_is_full(fb)) {
		return;
	}

	if (!h) {
		duk_fb_put_cstring(fb, "NULL");
		return;
	}

	switch (DUK_HEAPHDR_GET_TYPE(h)) {
	case DUK_HTYPE_STRING:
		duk__print_hstring(st, (duk_hstring *) h, 1);
		break;
	case DUK_HTYPE_OBJECT:
		duk__print_hobject(st, (duk_hobject *) h);
		break;
	case DUK_HTYPE_BUFFER:
		duk__print_hbuffer(st, (duk_hbuffer *) h);
		break;
	default:
		duk_fb_sprintf(fb, "[unknown htype %d]", DUK_HEAPHDR_GET_TYPE(h));
		break;
	}
}

static void duk__print_tval(duk__dprint_state *st, duk_tval *tv) {
	duk_fixedbuffer *fb = st->fb;

	if (duk_fb_is_full(fb)) {
		return;
	}

	/* depth check is done when printing an actual type */

	if (st->heavy) {
		duk_fb_sprintf(fb, "(%p)", (void *) tv);
	}

	if (!tv) {
		duk_fb_put_cstring(fb, "NULL");
		return;
	}

	if (st->binary) {
		duk_size_t i;
		duk_fb_put_byte(fb, (duk_uint8_t) '[');
		for (i = 0; i < (duk_size_t) sizeof(*tv); i++) {
			duk_fb_sprintf(fb, "%02x", (int) ((unsigned char *)tv)[i]);
		}
		duk_fb_put_byte(fb, (duk_uint8_t) ']');
	}

	if (st->heavy) {
		duk_fb_put_byte(fb, (duk_uint8_t) '<');
	}
	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED: {
		if (DUK_TVAL_IS_UNDEFINED_UNUSED(tv)) {
			duk_fb_put_cstring(fb, "unused");
		} else {
			duk_fb_put_cstring(fb, "undefined");
		}
		break;
	}
	case DUK_TAG_NULL: {
		duk_fb_put_cstring(fb, "null");
		break;
	}
	case DUK_TAG_BOOLEAN: {
		duk_fb_put_cstring(fb, DUK_TVAL_GET_BOOLEAN(tv) ? "true" : "false");
		break;
	}
	case DUK_TAG_STRING: {
		/* Note: string is a terminal heap object, so no depth check here */
		duk__print_hstring(st, DUK_TVAL_GET_STRING(tv), 1);
		break;
	}
	case DUK_TAG_OBJECT: {
		duk__print_hobject(st, DUK_TVAL_GET_OBJECT(tv));
		break;
	}
	case DUK_TAG_BUFFER: {
		duk__print_hbuffer(st, DUK_TVAL_GET_BUFFER(tv));
		break;
	}
	case DUK_TAG_POINTER: {
		duk_fb_sprintf(fb, "pointer:%p", DUK_TVAL_GET_POINTER(tv));
		break;
	}
	default: {
		/* IEEE double is approximately 16 decimal digits; print a couple extra */
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		duk_fb_sprintf(fb, "%.18g", DUK_TVAL_GET_NUMBER(tv));
		break;
	}
	}
	if (st->heavy) {
		duk_fb_put_byte(fb, (duk_uint8_t) '>');
	}
}

static void duk__print_instr(duk__dprint_state *st, duk_instr ins) {
	duk_fixedbuffer *fb = st->fb;
	int op;
	const char *op_name;
	const char *extraop_name;

	op = DUK_DEC_OP(ins);
	op_name = duk__bc_optab[op];

	/* FIXME: option to fix opcode length so it lines up nicely */

	if (op == DUK_OP_EXTRA) {
		extraop_name = duk__bc_extraoptab[DUK_DEC_A(ins)];

		duk_fb_sprintf(fb, "%s %d, %d",
		               extraop_name, DUK_DEC_B(ins), DUK_DEC_C(ins));
	} else if (op == DUK_OP_JUMP) {
		int diff1 = DUK_DEC_ABC(ins) - DUK_BC_JUMP_BIAS;  /* from next pc */
		int diff2 = diff1 + 1;                            /* from curr pc */

		duk_fb_sprintf(fb, "%s %d (to pc%c%d)",
		               op_name, diff1, (diff2 >= 0 ? '+' : '-'), (diff2 >= 0 ? diff2 : -diff2));
	} else {
		duk_fb_sprintf(fb, "%s %d, %d, %d",
		               op_name, DUK_DEC_A(ins), DUK_DEC_B(ins), DUK_DEC_C(ins));
	}
}

static void duk__print_opcode(duk__dprint_state *st, int opcode) {
	duk_fixedbuffer *fb = st->fb;

	if (opcode < DUK_BC_OP_MIN || opcode > DUK_BC_OP_MAX) {
		duk_fb_sprintf(fb, "?(%d)", opcode);
	} else {
		duk_fb_sprintf(fb, "%s", duk__bc_optab[opcode]);
	}
}

int duk_debug_vsnprintf(char *str, size_t size, const char *format, va_list ap) {
	duk_fixedbuffer fb;
	const char *p = format;
	const char *p_end = p + DUK_STRLEN(format);
	int retval;
	
	DUK_MEMZERO(&fb, sizeof(fb));
	fb.buffer = (duk_uint8_t *) str;
	fb.length = size;
	fb.offset = 0;
	fb.truncated = 0;

	while (p < p_end) {
		char ch = *p++;
		const char *p_begfmt = NULL;
		int got_exclamation = 0;
		duk__dprint_state st;

		if (ch != '%') {
			duk_fb_put_byte(&fb, (duk_uint8_t) ch);
			continue;
		}

		/*
		 *  Format tag parsing.  Since we don't understand all the
		 *  possible format tags allowed, we just scan for a terminating
		 *  specifier and keep track of relevant modifiers that we do
		 *  understand.  See man 3 printf.
		 */

		DUK_MEMZERO(&st, sizeof(st));
		st.fb = &fb;
		st.depth = 0;
		st.depth_limit = 1;
		st.loop_stack_index = 0;
		st.loop_stack_limit = DUK__LOOP_STACK_DEPTH;

		p_begfmt = p - 1;
		while (p < p_end) {
			ch = *p++;

			if (ch == '*') {
				/* unsupported: would consume multiple args */
				goto error;
			} else if (ch == '%') {
				duk_fb_put_byte(&fb, (duk_uint8_t) '%');
				break;
			} else if (ch == '!') {
				got_exclamation = 1;
			} else if (got_exclamation && ch == 'd') {
				st.depth_limit = DUK__DEEP_DEPTH_LIMIT;
			} else if (got_exclamation && ch == 'p') {
				st.follow_proto = 1;
			} else if (got_exclamation && ch == 'i') {
				st.internal = 1;
			} else if (got_exclamation && ch == 'x') {
				st.hexdump = 1;
			} else if (got_exclamation && ch == 'h') {
				st.heavy = 1;
			} else if (got_exclamation && ch == '@') {
				st.pointer = 1;
			} else if (got_exclamation && ch == '#') {
				st.binary = 1;
			} else if (got_exclamation && ch == 'T') {
				duk_tval *t = va_arg(ap, duk_tval *);
				if (st.pointer && !st.heavy) {
					duk_fb_sprintf(&fb, "(%p)", (void *) t);
				}
				duk__print_tval(&st, t);
				break;
			} else if (got_exclamation && ch == 'O') {
				duk_heaphdr *t = va_arg(ap, duk_heaphdr *);
				if (st.pointer && !st.heavy) {
					duk_fb_sprintf(&fb, "(%p)", (void *) t);
				}
				duk__print_heaphdr(&st, t);
				break;
			} else if (got_exclamation && ch == 'I') {
				duk_instr t = va_arg(ap, duk_instr);
				duk__print_instr(&st, t);
				break;
			} else if (got_exclamation && ch == 'C') {
				int t = va_arg(ap, int);
				duk__print_opcode(&st, t);
				break;
			} else if (!got_exclamation && strchr(DUK__ALLOWED_STANDARD_SPECIFIERS, (int) ch)) {
				char fmtbuf[DUK__MAX_FORMAT_TAG_LENGTH];
				duk_size_t fmtlen;

				DUK_ASSERT(p >= p_begfmt);
				fmtlen = (duk_size_t) (p - p_begfmt);
				if (fmtlen >= sizeof(fmtbuf)) {
					/* format is too large, abort */
					goto error;
				}
				DUK_MEMZERO(fmtbuf, sizeof(fmtbuf));
				DUK_MEMCPY(fmtbuf, p_begfmt, fmtlen);

				/* assume exactly 1 arg, which is why '*' is forbidden; arg size still
				 * depends on type though.
				 */

				/* FIXME: check size for other types.. actually it would be best to switch
				 * for supported standard formats and get args explicitly
				 */
				if (ch == 'f' || ch == 'g' || ch == 'e') {
					double arg;
					arg = va_arg(ap, double);
					duk_fb_sprintf(&fb, fmtbuf, arg);
				} else {
					void *arg;
					arg = va_arg(ap, void *);
					duk_fb_sprintf(&fb, fmtbuf, arg);
				}
				break;
			} else {
				/* ignore */
			}
		}
	}
	goto done;

 error:
	duk_fb_put_cstring(&fb, "FMTERR");
	/* fall through */

 done:
	retval = fb.offset;
	duk_fb_put_byte(&fb, (duk_uint8_t) 0);

	/* return total chars written excluding terminator */
	return retval;
}

int duk_debug_snprintf(char *str, size_t size, const char *format, ...) {
	int retval;
	va_list ap;
	va_start(ap, format);
	retval = duk_debug_vsnprintf(str, size, format, ap);
	va_end(ap);
	return retval;
}

/* Formatting function pointers is tricky: there is no standard pointer for
 * function pointers and the size of a function pointer may depend on the
 * specific pointer type.  This helper formats a function pointer based on
 * its memory layout to get something useful on most platforms.
 */
void duk_debug_format_funcptr(char *buf, int buf_size, unsigned char *fptr, int fptr_size) {
	int i;
	char *p = buf;
	char *p_end = buf + buf_size - 1;

	DUK_MEMZERO(buf, buf_size);

	for (i = 0; i < fptr_size; i++) {
		int left = p_end - p;
		unsigned char ch;
		if (left <= 0) {
			break;
		}

		/* Quite approximate but should be useful for little and big endian. */
#ifdef DUK_USE_INTEGER_BE
		ch = fptr[i];
#else
		ch = fptr[fptr_size - 1 - i];
#endif
		p += DUK_SNPRINTF(p, left, "%02x", (int) ch);
	}	
}

#endif  /* DUK_USE_DEBUG */

