/*
 *  Object representation.
 *
 *  duk_hobjects are used for ECMAScript objects, arrays, functions, Proxies,
 *  etc, but also for internal control like declarative and object environment
 *  records.  Several object types have an extended C struct like duk_harray,
 *  duk_hcompfunc, etc.
 *
 *  Some duk_hobject types extend the base duk_hobject structure, e.g.
 *  duk_harray.  Such objects may have extra fields in their extended
 *  struct.  For example, duk_harray is used for Array and Arguments object
 *  instances and has a linear array for items which is used when default
 *  property attributes are sufficient and the items part is dense enough;
 *  otherwise values in 'items' are migrated to the 'entry part'.
 */

#if !defined(DUK_HOBJECT_H_INCLUDED)
#define DUK_HOBJECT_H_INCLUDED

/* Object flags.  Make sure this stays in sync with debugger object
 * inspection code.
 */

/* XXX: some flags are object subtype specific (e.g. common to all function
 * subtypes, duk_harray, etc) and could be reused for different subtypes.
 */
#define DUK_HOBJECT_FLAG_EXTENSIBLE    DUK_HEAPHDR_USER_FLAG(0) /* object is extensible */
#define DUK_HOBJECT_FLAG_CONSTRUCTABLE DUK_HEAPHDR_USER_FLAG(1) /* object is constructable */
#define DUK_HOBJECT_FLAG_CALLABLE      DUK_HEAPHDR_USER_FLAG(2) /* object is callable */
#define DUK_HOBJECT_FLAG_BOUNDFUNC     DUK_HEAPHDR_USER_FLAG(3) /* object is a bound function (duk_hboundfunc) */
#define DUK_HOBJECT_FLAG_COMPFUNC      DUK_HEAPHDR_USER_FLAG(4) /* object is a compiled function (duk_hcompfunc) */
#define DUK_HOBJECT_FLAG_NATFUNC       DUK_HEAPHDR_USER_FLAG(5) /* object is a native function (duk_hnatfunc) */
#define DUK_HOBJECT_FLAG_BUFOBJ        DUK_HEAPHDR_USER_FLAG(6) /* object is a buffer object (duk_hbufobj) (always exotic) */
#define DUK_HOBJECT_FLAG_FASTREFS \
	DUK_HEAPHDR_USER_FLAG(7) /* object has no fields needing DECREF/marking beyond base duk_hobject header */
#define DUK_HOBJECT_FLAG_ARRAY_ITEMS DUK_HEAPHDR_USER_FLAG(8) /* array has 'items' (= not abandoned) */
#define DUK_HOBJECT_FLAG_STRICT      DUK_HEAPHDR_USER_FLAG(9) /* function: function object is strict */
#define DUK_HOBJECT_FLAG_NOTAIL      DUK_HEAPHDR_USER_FLAG(10) /* function: function must not be tail called */
#define DUK_HOBJECT_FLAG_NEWENV      DUK_HEAPHDR_USER_FLAG(11) /* function: create new environment when called (see duk_hcompfunc) */
#define DUK_HOBJECT_FLAG_NAMEBINDING \
	DUK_HEAPHDR_USER_FLAG( \
	    12) /* function: create binding for func name (function templates only, used for named function expressions) */
#define DUK_HOBJECT_FLAG_CREATEARGS       DUK_HEAPHDR_USER_FLAG(13) /* function: create an arguments object on function call */
#define DUK_HOBJECT_FLAG_HAVE_FINALIZER   DUK_HEAPHDR_USER_FLAG(14) /* object has a callable (own) finalizer slot value */
#define DUK_HOBJECT_FLAG_EXOTIC_ARRAY     DUK_HEAPHDR_USER_FLAG(15) /* 'Array' object, array length and index exotic behavior */
#define DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ DUK_HEAPHDR_USER_FLAG(16) /* 'String' object, array index exotic behavior */
#define DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS \
	DUK_HEAPHDR_USER_FLAG(17) /* 'Arguments' object and has arguments exotic behavior (non-strict callee) */
#define DUK_HOBJECT_FLAG_EXOTIC_PROXYOBJ DUK_HEAPHDR_USER_FLAG(18) /* 'Proxy' object */
#define DUK_HOBJECT_FLAG_SPECIAL_CALL    DUK_HEAPHDR_USER_FLAG(19) /* special casing in call behavior, for .call(), .apply(), etc. */
#define DUK_HOBJECT_FLAG_UNUSED20        DUK_HEAPHDR_USER_FLAG(20)

#define DUK_HOBJECT_GET_HTYPE(h)      DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) (h))
#define DUK_HOBJECT_SET_HTYPE(h, val) DUK_HEAPHDR_SET_HTYPE((duk_heaphdr *) (h), (val))

#if defined(DUK_USE_64BIT_OPS)
#define DUK_HOBJECT_GET_HMASK(h) DUK_HEAPHDR_GET_HMASK((duk_heaphdr *) (h))
#endif

#define DUK_HOBJECT_IS_ANY_STRING(h) DUK_HEAPHDR_IS_ANY_STRING((duk_heaphdr *) (h))
#define DUK_HOBJECT_IS_ANY_BUFFER(h) DUK_HEAPHDR_IS_ANY_BUFFER((duk_heaphdr *) (h))
#define DUK_HOBJECT_IS_ANY_BUFOBJ(h) DUK_HEAPHDR_IS_ANY_BUFOBJ((duk_heaphdr *) (h))
#define DUK_HOBJECT_IS_ANY_OBJECT(h) DUK_HEAPHDR_IS_ANY_OBJECT((duk_heaphdr *) (h))

#define DUK_HOBJECT_IS_HARRAY(h) (DUK_HOBJECT_IS_ARRAY((h)) || DUK_HOBJECT_IS_ARGUMENTS((h)))

#define DUK_HOBJECT_IS_OBJENV(h) (DUK_HOBJECT_GET_HTYPE((h)) == DUK_HTYPE_OBJENV)
#define DUK_HOBJECT_IS_DECENV(h) (DUK_HOBJECT_GET_HTYPE((h)) == DUK_HTYPE_DECENV)
#define DUK_HOBJECT_IS_ENV(h)    (DUK_HOBJECT_IS_OBJENV((h)) || DUK_HOBJECT_IS_DECENV((h)))

#define DUK_HOBJECT_IS_ARRAY(h) DUK_HOBJECT_HAS_EXOTIC_ARRAY((h)) /* Rely on class Array <=> exotic Array => duk_harray */
#define DUK_HOBJECT_IS_ARGUMENTS(h) \
	(DUK_HOBJECT_GET_HTYPE((h)) == \
	 DUK_HTYPE_ARGUMENTS) /* Use htype; all Arguments objects don't have Arguments exotic behavior */
#define DUK_HOBJECT_IS_BOUNDFUNC(h) DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BOUNDFUNC)
#define DUK_HOBJECT_IS_COMPFUNC(h)  DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_COMPFUNC)
#define DUK_HOBJECT_IS_NATFUNC(h)   DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NATFUNC)
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
#define DUK_HOBJECT_IS_BUFOBJ(h) DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BUFOBJ)
#else
#define DUK_HOBJECT_IS_BUFOBJ(h) 0
#endif
#define DUK_HOBJECT_IS_THREAD(h) (DUK_HOBJECT_GET_HTYPE((h)) == DUK_HTYPE_THREAD)
#if defined(DUK_USE_ES6_PROXY)
#define DUK_HOBJECT_IS_PROXY(h) DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ((h))
#else
#define DUK_HOBJECT_IS_PROXY(h) 0
#endif

#define DUK_HOBJECT_IS_NONBOUND_FUNCTION(h) \
	DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_COMPFUNC | DUK_HOBJECT_FLAG_NATFUNC)

#define DUK_HOBJECT_IS_FUNCTION(h) \
	DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BOUNDFUNC | DUK_HOBJECT_FLAG_COMPFUNC | DUK_HOBJECT_FLAG_NATFUNC)

#define DUK_HOBJECT_IS_CALLABLE(h) DUK_HOBJECT_HAS_CALLABLE((h))

/* Object has any exotic behavior(s). */
#define DUK_HOBJECT_EXOTIC_BEHAVIOR_FLAGS \
	(DUK_HOBJECT_FLAG_EXOTIC_ARRAY | DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS | DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ | \
	 DUK_HOBJECT_FLAG_BUFOBJ | DUK_HOBJECT_FLAG_EXOTIC_PROXYOBJ)
#define DUK_HOBJECT_HAS_EXOTIC_BEHAVIOR(h) DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_EXOTIC_BEHAVIOR_FLAGS)

/* Object has any virtual properties (not counting Proxy behavior). */
#define DUK_HOBJECT_VIRTUAL_PROPERTY_FLAGS \
	(DUK_HOBJECT_FLAG_EXOTIC_ARRAY | DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ | DUK_HOBJECT_FLAG_BUFOBJ)
#define DUK_HOBJECT_HAS_VIRTUAL_PROPERTIES(h) DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_VIRTUAL_PROPERTY_FLAGS)

#define DUK_HOBJECT_HAS_EXTENSIBLE(h)    DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXTENSIBLE)
#define DUK_HOBJECT_HAS_CONSTRUCTABLE(h) DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CONSTRUCTABLE)
#define DUK_HOBJECT_HAS_CALLABLE(h)      DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CALLABLE)
#define DUK_HOBJECT_HAS_BOUNDFUNC(h)     DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BOUNDFUNC)
#define DUK_HOBJECT_HAS_COMPFUNC(h)      DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_COMPFUNC)
#define DUK_HOBJECT_HAS_NATFUNC(h)       DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NATFUNC)
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
#define DUK_HOBJECT_HAS_BUFOBJ(h) DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BUFOBJ)
#else
#define DUK_HOBJECT_HAS_BUFOBJ(h) 0
#endif
#define DUK_HOBJECT_HAS_FASTREFS(h)         DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_FASTREFS)
#define DUK_HOBJECT_HAS_ARRAY_ITEMS(h)      DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_ARRAY_ITEMS)
#define DUK_HOBJECT_HAS_STRICT(h)           DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_STRICT)
#define DUK_HOBJECT_HAS_NOTAIL(h)           DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NOTAIL)
#define DUK_HOBJECT_HAS_NEWENV(h)           DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NEWENV)
#define DUK_HOBJECT_HAS_NAMEBINDING(h)      DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NAMEBINDING)
#define DUK_HOBJECT_HAS_CREATEARGS(h)       DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CREATEARGS)
#define DUK_HOBJECT_HAS_HAVE_FINALIZER(h)   DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_HAVE_FINALIZER)
#define DUK_HOBJECT_HAS_EXOTIC_ARRAY(h)     DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_ARRAY)
#define DUK_HOBJECT_HAS_EXOTIC_STRINGOBJ(h) DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ)
#define DUK_HOBJECT_HAS_EXOTIC_ARGUMENTS(h) DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS)
#if defined(DUK_USE_ES6_PROXY)
#define DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(h) DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_PROXYOBJ)
#else
#define DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(h) 0
#endif
#define DUK_HOBJECT_HAS_SPECIAL_CALL(h) DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_SPECIAL_CALL)

#define DUK_HOBJECT_SET_EXTENSIBLE(h)    DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXTENSIBLE)
#define DUK_HOBJECT_SET_CONSTRUCTABLE(h) DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CONSTRUCTABLE)
#define DUK_HOBJECT_SET_CALLABLE(h)      DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CALLABLE)
#define DUK_HOBJECT_SET_BOUNDFUNC(h)     DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BOUNDFUNC)
#define DUK_HOBJECT_SET_COMPFUNC(h)      DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_COMPFUNC)
#define DUK_HOBJECT_SET_NATFUNC(h)       DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NATFUNC)
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
#define DUK_HOBJECT_SET_BUFOBJ(h) DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BUFOBJ)
#endif
#define DUK_HOBJECT_SET_FASTREFS(h)         DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_FASTREFS)
#define DUK_HOBJECT_SET_ARRAY_ITEMS(h)      DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_ARRAY_ITEMS)
#define DUK_HOBJECT_SET_STRICT(h)           DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_STRICT)
#define DUK_HOBJECT_SET_NOTAIL(h)           DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NOTAIL)
#define DUK_HOBJECT_SET_NEWENV(h)           DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NEWENV)
#define DUK_HOBJECT_SET_NAMEBINDING(h)      DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NAMEBINDING)
#define DUK_HOBJECT_SET_CREATEARGS(h)       DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CREATEARGS)
#define DUK_HOBJECT_SET_HAVE_FINALIZER(h)   DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_HAVE_FINALIZER)
#define DUK_HOBJECT_SET_EXOTIC_ARRAY(h)     DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_ARRAY)
#define DUK_HOBJECT_SET_EXOTIC_STRINGOBJ(h) DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ)
#define DUK_HOBJECT_SET_EXOTIC_ARGUMENTS(h) DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS)
#if defined(DUK_USE_ES6_PROXY)
#define DUK_HOBJECT_SET_EXOTIC_PROXYOBJ(h) DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_PROXYOBJ)
#endif
#define DUK_HOBJECT_SET_SPECIAL_CALL(h) DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_SPECIAL_CALL)

#define DUK_HOBJECT_CLEAR_EXTENSIBLE(h)    DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXTENSIBLE)
#define DUK_HOBJECT_CLEAR_CONSTRUCTABLE(h) DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CONSTRUCTABLE)
#define DUK_HOBJECT_CLEAR_CALLABLE(h)      DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CALLABLE)
#define DUK_HOBJECT_CLEAR_BOUNDFUNC(h)     DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BOUNDFUNC)
#define DUK_HOBJECT_CLEAR_COMPFUNC(h)      DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_COMPFUNC)
#define DUK_HOBJECT_CLEAR_NATFUNC(h)       DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NATFUNC)
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
#define DUK_HOBJECT_CLEAR_BUFOBJ(h) DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BUFOBJ)
#endif
#define DUK_HOBJECT_CLEAR_FASTREFS(h)         DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_FASTREFS)
#define DUK_HOBJECT_CLEAR_ARRAY_ITEMS(h)      DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_ARRAY_ITEMS)
#define DUK_HOBJECT_CLEAR_STRICT(h)           DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_STRICT)
#define DUK_HOBJECT_CLEAR_NOTAIL(h)           DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NOTAIL)
#define DUK_HOBJECT_CLEAR_NEWENV(h)           DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NEWENV)
#define DUK_HOBJECT_CLEAR_NAMEBINDING(h)      DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NAMEBINDING)
#define DUK_HOBJECT_CLEAR_CREATEARGS(h)       DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CREATEARGS)
#define DUK_HOBJECT_CLEAR_HAVE_FINALIZER(h)   DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_HAVE_FINALIZER)
#define DUK_HOBJECT_CLEAR_EXOTIC_ARRAY(h)     DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_ARRAY)
#define DUK_HOBJECT_CLEAR_EXOTIC_STRINGOBJ(h) DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ)
#define DUK_HOBJECT_CLEAR_EXOTIC_ARGUMENTS(h) DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS)
#if defined(DUK_USE_ES6_PROXY)
#define DUK_HOBJECT_CLEAR_EXOTIC_PROXYOBJ(h) DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_PROXYOBJ)
#endif
#define DUK_HOBJECT_CLEAR_SPECIAL_CALL(h) DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_SPECIAL_CALL)

/* Object can use FASTREFS <=> has no strong reference fields beyond
 * duk_hobject base header.
 */
#define DUK_HOBJECT_PROHIBITS_FASTREFS(h) \
	(DUK_HOBJECT_IS_COMPFUNC((h)) || DUK_HOBJECT_IS_DECENV((h)) || DUK_HOBJECT_IS_OBJENV((h)) || DUK_HOBJECT_IS_BUFOBJ((h)) || \
	 DUK_HOBJECT_IS_THREAD((h)) || DUK_HOBJECT_IS_PROXY((h)) || DUK_HOBJECT_IS_BOUNDFUNC((h)) || DUK_HOBJECT_IS_ARRAY((h)) || \
	 DUK_HOBJECT_IS_ARGUMENTS((h)))
#define DUK_HOBJECT_ALLOWS_FASTREFS(h) (!DUK_HOBJECT_PROHIBITS_FASTREFS((h)))

/* Flags used for property attributes in packed flags.  Must fit into 8 bits. */
#define DUK_PROPDESC_FLAG_WRITABLE     (1U << 0) /* E5 Section 8.6.1 */
#define DUK_PROPDESC_FLAG_ENUMERABLE   (1U << 1) /* E5 Section 8.6.1 */
#define DUK_PROPDESC_FLAG_CONFIGURABLE (1U << 2) /* E5 Section 8.6.1 */
#define DUK_PROPDESC_FLAG_ACCESSOR     (1U << 3) /* accessor */
#define DUK_PROPDESC_FLAG_VIRTUAL      (1U << 4) /* property is virtual: never stored (used by e.g. buffer virtual properties) */
#define DUK_PROPDESC_FLAGS_MASK \
	(DUK_PROPDESC_FLAG_WRITABLE | DUK_PROPDESC_FLAG_ENUMERABLE | DUK_PROPDESC_FLAG_CONFIGURABLE | DUK_PROPDESC_FLAG_ACCESSOR)

/* Additional flags which are passed in the same flags argument as property
 * flags but are not stored in object properties.
 */
#define DUK_PROPDESC_FLAG_NO_OVERWRITE (1U << 4) /* internal define property: skip write silently if exists */

/* Convenience defines for property attributes. */
#define DUK_PROPDESC_FLAGS_NONE 0
#define DUK_PROPDESC_FLAGS_W    (DUK_PROPDESC_FLAG_WRITABLE)
#define DUK_PROPDESC_FLAGS_E    (DUK_PROPDESC_FLAG_ENUMERABLE)
#define DUK_PROPDESC_FLAGS_C    (DUK_PROPDESC_FLAG_CONFIGURABLE)
#define DUK_PROPDESC_FLAGS_WE   (DUK_PROPDESC_FLAG_WRITABLE | DUK_PROPDESC_FLAG_ENUMERABLE)
#define DUK_PROPDESC_FLAGS_WC   (DUK_PROPDESC_FLAG_WRITABLE | DUK_PROPDESC_FLAG_CONFIGURABLE)
#define DUK_PROPDESC_FLAGS_EC   (DUK_PROPDESC_FLAG_ENUMERABLE | DUK_PROPDESC_FLAG_CONFIGURABLE)
#define DUK_PROPDESC_FLAGS_WEC  (DUK_PROPDESC_FLAG_WRITABLE | DUK_PROPDESC_FLAG_ENUMERABLE | DUK_PROPDESC_FLAG_CONFIGURABLE)

/*
 *  Macro for object validity check
 *
 *  Assert for currently guaranteed relations between flags, for instance.
 */

#if defined(DUK_USE_ASSERTIONS)
DUK_INTERNAL_DECL void duk_hobject_assert_valid(duk_heap *heap, duk_hobject *obj);
DUK_INTERNAL_DECL void duk_hobject_assert_compact(duk_heap *heap, duk_hobject *obj);
DUK_INTERNAL_DECL void duk_hobject_assert_key_absent(duk_heap *heap, duk_hobject *obj, duk_hstring *key);
#define DUK_HOBJECT_ASSERT_VALID(heap, h) \
	do { \
		duk_hobject_assert_valid((heap), (h)); \
	} while (0)
#define DUK_HOBJECT_ASSERT_COMPACT(heap, h) \
	do { \
		duk_hobject_assert_compact((heap), (h)); \
	} while (0)
#define DUK_HOBJECT_ASSERT_KEY_ABSENT(heap, h, k) \
	do { \
		duk_hobject_assert_key_absent((heap), (h), (k)); \
	} while (0)
#else
#define DUK_HOBJECT_ASSERT_VALID(heap, h) \
	do { \
	} while (0)
#define DUK_HOBJECT_ASSERT_COMPACT(heap, h) \
	do { \
	} while (0)
#define DUK_HOBJECT_ASSERT_KEY_ABSENT(heap, h, k) \
	do { \
	} while (0)
#endif

/*
 *  Macros to access the 'props' allocation.
 */

#if defined(DUK_USE_HEAPPTR16)
#define DUK_HOBJECT_GET_PROPS(heap, h) ((duk_uint8_t *) DUK_USE_HEAPPTR_DEC16((heap)->heap_udata, ((duk_heaphdr *) (h))->h_extra16))
#define DUK_HOBJECT_SET_PROPS(heap, h, x) \
	do { \
		((duk_heaphdr *) (h))->h_extra16 = DUK_USE_HEAPPTR_ENC16((heap)->heap_udata, (void *) (x)); \
	} while (0)
#define DUK_HOBJECT_GET_HASH(heap, h) ((duk_uint8_t *) DUK_USE_HEAPPTR_DEC16((heap)->heap_udata, (h)->hash16))
#define DUK_HOBJECT_SET_HASH(heap, h, x) \
	do { \
		(h)->hash16 = DUK_USE_HEAPPTR_ENC16((heap)->heap_udata, (void *) (x)); \
	} while (0)
#else
#define DUK_HOBJECT_GET_PROPS(heap, h) ((h)->props)
#define DUK_HOBJECT_SET_PROPS(heap, h, x) \
	do { \
		(h)->props = (duk_uint8_t *) (x); \
	} while (0)
#define DUK_HOBJECT_GET_HASH(heap, h) ((h)->hash)
#define DUK_HOBJECT_SET_HASH(heap, h, x) \
	do { \
		(h)->hash = (duk_uint32_t *) (x); \
	} while (0)

#endif

#define DUK_HOBJECT_E_GET_KEY_BASE(heap, h) \
	((duk_hstring **) (void *) (DUK_HOBJECT_GET_PROPS((heap), (h)) + DUK_HOBJECT_GET_ESIZE((h)) * sizeof(duk_propvalue)))
#define DUK_HOBJECT_E_GET_VALUE_BASE(heap, h) ((duk_propvalue *) (void *) (DUK_HOBJECT_GET_PROPS((heap), (h))))
#define DUK_HOBJECT_E_GET_FLAGS_BASE(heap, h) \
	((duk_uint8_t *) (void *) (DUK_HOBJECT_GET_PROPS((heap), (h)) + \
	                           DUK_HOBJECT_GET_ESIZE((h)) * (sizeof(duk_propvalue) + sizeof(duk_hstring *))))
#define DUK_HOBJECT_H_GET_BASE(heap, h)    ((duk_uint32_t *) (void *) (DUK_HOBJECT_GET_HASH((heap), (h))))
#define DUK_HOBJECT_P_COMPUTE_SIZE(n_ent)  ((n_ent) * (sizeof(duk_propvalue) + sizeof(duk_hstring *) + sizeof(duk_uint8_t)))
#define DUK_HOBJECT_H_COMPUTE_SIZE(n_hash) ((n_hash + 1) * (sizeof(duk_uint32_t)))

#define DUK_HOBJECT_E_GET_KEY(heap, h, i)              (DUK_HOBJECT_E_GET_KEY_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_E_GET_KEY_PTR(heap, h, i)          (&DUK_HOBJECT_E_GET_KEY_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_E_GET_VALUE(heap, h, i)            (DUK_HOBJECT_E_GET_VALUE_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_E_GET_VALUE_PTR(heap, h, i)        (&DUK_HOBJECT_E_GET_VALUE_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_E_GET_VALUE_TVAL(heap, h, i)       (DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).v)
#define DUK_HOBJECT_E_GET_VALUE_TVAL_PTR(heap, h, i)   (&DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).v)
#define DUK_HOBJECT_E_GET_VALUE_GETTER(heap, h, i)     (DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).a.get)
#define DUK_HOBJECT_E_GET_VALUE_GETTER_PTR(heap, h, i) (&DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).a.get)
#define DUK_HOBJECT_E_GET_VALUE_SETTER(heap, h, i)     (DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).a.set)
#define DUK_HOBJECT_E_GET_VALUE_SETTER_PTR(heap, h, i) (&DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).a.set)
#define DUK_HOBJECT_E_GET_FLAGS(heap, h, i)            (DUK_HOBJECT_E_GET_FLAGS_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_E_GET_FLAGS_PTR(heap, h, i)        (&DUK_HOBJECT_E_GET_FLAGS_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_H_GET_INDEX(heap, h, i)            (DUK_HOBJECT_H_GET_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_H_GET_INDEX_PTR(heap, h, i)        (&DUK_HOBJECT_H_GET_BASE((heap), (h))[(i)])

#define DUK_HOBJECT_E_SET_KEY(heap, h, i, k) \
	do { \
		DUK_HOBJECT_E_GET_KEY((heap), (h), (i)) = (k); \
	} while (0)
#define DUK_HOBJECT_E_SET_VALUE(heap, h, i, v) \
	do { \
		DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)) = (v); \
	} while (0)
#define DUK_HOBJECT_E_SET_VALUE_TVAL(heap, h, i, v) \
	do { \
		DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).v = (v); \
	} while (0)
#define DUK_HOBJECT_E_SET_VALUE_GETTER(heap, h, i, v) \
	do { \
		DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).a.get = (v); \
	} while (0)
#define DUK_HOBJECT_E_SET_VALUE_SETTER(heap, h, i, v) \
	do { \
		DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).a.set = (v); \
	} while (0)
#define DUK_HOBJECT_E_SET_FLAGS(heap, h, i, f) \
	do { \
		DUK_HOBJECT_E_GET_FLAGS((heap), (h), (i)) = (duk_uint8_t) (f); \
	} while (0)
#define DUK_HOBJECT_H_SET_INDEX(heap, h, i, v) \
	do { \
		DUK_HOBJECT_H_GET_INDEX((heap), (h), (i)) = (v); \
	} while (0)

#define DUK_HOBJECT_E_SET_FLAG_BITS(heap, h, i, mask) \
	do { \
		DUK_HOBJECT_E_GET_FLAGS_BASE((heap), (h))[(i)] |= (mask); \
	} while (0)

#define DUK_HOBJECT_E_CLEAR_FLAG_BITS(heap, h, i, mask) \
	do { \
		DUK_HOBJECT_E_GET_FLAGS_BASE((heap), (h))[(i)] &= ~(mask); \
	} while (0)

#define DUK_HOBJECT_E_SLOT_IS_WRITABLE(heap, h, i) ((DUK_HOBJECT_E_GET_FLAGS((heap), (h), (i)) & DUK_PROPDESC_FLAG_WRITABLE) != 0)
#define DUK_HOBJECT_E_SLOT_IS_ENUMERABLE(heap, h, i) \
	((DUK_HOBJECT_E_GET_FLAGS((heap), (h), (i)) & DUK_PROPDESC_FLAG_ENUMERABLE) != 0)
#define DUK_HOBJECT_E_SLOT_IS_CONFIGURABLE(heap, h, i) \
	((DUK_HOBJECT_E_GET_FLAGS((heap), (h), (i)) & DUK_PROPDESC_FLAG_CONFIGURABLE) != 0)
#define DUK_HOBJECT_E_SLOT_IS_ACCESSOR(heap, h, i) ((DUK_HOBJECT_E_GET_FLAGS((heap), (h), (i)) & DUK_PROPDESC_FLAG_ACCESSOR) != 0)

#define DUK_HOBJECT_E_SLOT_SET_WRITABLE(heap, h, i)   DUK_HOBJECT_E_SET_FLAG_BITS((heap), (h), (i), DUK_PROPDESC_FLAG_WRITABLE)
#define DUK_HOBJECT_E_SLOT_SET_ENUMERABLE(heap, h, i) DUK_HOBJECT_E_SET_FLAG_BITS((heap), (h), (i), DUK_PROPDESC_FLAG_ENUMERABLE)
#define DUK_HOBJECT_E_SLOT_SET_CONFIGURABLE(heap, h, i) \
	DUK_HOBJECT_E_SET_FLAG_BITS((heap), (h), (i), DUK_PROPDESC_FLAG_CONFIGURABLE)
#define DUK_HOBJECT_E_SLOT_SET_ACCESSOR(heap, h, i) DUK_HOBJECT_E_SET_FLAG_BITS((heap), (h), (i), DUK_PROPDESC_FLAG_ACCESSOR)

#define DUK_HOBJECT_E_SLOT_CLEAR_WRITABLE(heap, h, i) DUK_HOBJECT_E_CLEAR_FLAG_BITS((heap), (h), (i), DUK_PROPDESC_FLAG_WRITABLE)
#define DUK_HOBJECT_E_SLOT_CLEAR_ENUMERABLE(heap, h, i) \
	DUK_HOBJECT_E_CLEAR_FLAG_BITS((heap), (h), (i), DUK_PROPDESC_FLAG_ENUMERABLE)
#define DUK_HOBJECT_E_SLOT_CLEAR_CONFIGURABLE(heap, h, i) \
	DUK_HOBJECT_E_CLEAR_FLAG_BITS((heap), (h), (i), DUK_PROPDESC_FLAG_CONFIGURABLE)
#define DUK_HOBJECT_E_SLOT_CLEAR_ACCESSOR(heap, h, i) DUK_HOBJECT_E_CLEAR_FLAG_BITS((heap), (h), (i), DUK_PROPDESC_FLAG_ACCESSOR)

#define DUK_PROPDESC_IS_WRITABLE(p)     (((p)->flags & DUK_PROPDESC_FLAG_WRITABLE) != 0)
#define DUK_PROPDESC_IS_ENUMERABLE(p)   (((p)->flags & DUK_PROPDESC_FLAG_ENUMERABLE) != 0)
#define DUK_PROPDESC_IS_CONFIGURABLE(p) (((p)->flags & DUK_PROPDESC_FLAG_CONFIGURABLE) != 0)
#define DUK_PROPDESC_IS_ACCESSOR(p)     (((p)->flags & DUK_PROPDESC_FLAG_ACCESSOR) != 0)

#define DUK_HOBJECT_HASHIDX_UNUSED  0xffffffffUL
#define DUK_HOBJECT_HASHIDX_DELETED 0xfffffffeUL

/*
 *  Macros for accessing size fields
 */

#if defined(DUK_USE_OBJSIZES16)
#define DUK_HOBJECT_GET_ESIZE(h) ((h)->e_size16)
#define DUK_HOBJECT_SET_ESIZE(h, v) \
	do { \
		(h)->e_size16 = (v); \
	} while (0)
#define DUK_HOBJECT_GET_ENEXT(h) ((h)->e_next16)
#define DUK_HOBJECT_SET_ENEXT(h, v) \
	do { \
		(h)->e_next16 = (v); \
	} while (0)
#define DUK_HOBJECT_POSTINC_ENEXT(h) ((h)->e_next16++)
#else
#define DUK_HOBJECT_GET_ESIZE(h) ((h)->e_size)
#define DUK_HOBJECT_SET_ESIZE(h, v) \
	do { \
		(h)->e_size = (v); \
	} while (0)
#define DUK_HOBJECT_GET_ENEXT(h) ((h)->e_next)
#define DUK_HOBJECT_SET_ENEXT(h, v) \
	do { \
		(h)->e_next = (v); \
	} while (0)
#define DUK_HOBJECT_POSTINC_ENEXT(h) ((h)->e_next++)
#endif

/*
 *  Misc
 */

/* Maximum prototype traversal depth.  Sanity limit which handles e.g.
 * prototype loops (even complex ones like 1->2->3->4->2->3->4->2->3->4).
 */
#define DUK_HOBJECT_PROTOTYPE_CHAIN_SANITY 10000L

/* Valstack space allocated especially for proxy lookup which does a
 * recursive property lookup.
 */
#define DUK_HOBJECT_PROXY_VALSTACK_SPACE 20

/* Valstack space that suffices for all local calls, excluding any recursion
 * into ECMAScript or Duktape/C calls (Proxy, getters, etc).
 */
#define DUK_HOBJECT_PROP_VALSTACK_SPACE 10

/*
 *  ECMAScript [[Class]]
 */

/* Range check not necessary because all 6-bit values are mapped. */
#define DUK_HOBJECT_HTYPE_TO_STRIDX(n) duk_htype_to_stridx[(n)]

#define DUK_HOBJECT_GET_CLASS_STRING(heap, h) \
	DUK_HEAP_GET_STRING((heap), DUK_HOBJECT_CLASS_NUMBER_TO_STRIDX(DUK_HOBJECT_GET_CLASS_NUMBER((h))))

/*
 *  Macros for property handling
 */

#if defined(DUK_USE_HEAPPTR16)
#define DUK_HOBJECT_GET_PROTOTYPE(heap, h) ((duk_hobject *) DUK_USE_HEAPPTR_DEC16((heap)->heap_udata, (h)->prototype16))
#define DUK_HOBJECT_SET_PROTOTYPE(heap, h, x) \
	do { \
		(h)->prototype16 = DUK_USE_HEAPPTR_ENC16((heap)->heap_udata, (void *) (x)); \
	} while (0)
#else
#define DUK_HOBJECT_GET_PROTOTYPE(heap, h) ((h)->prototype)
#define DUK_HOBJECT_SET_PROTOTYPE(heap, h, x) \
	do { \
		(h)->prototype = (x); \
	} while (0)
#endif

/* Set initial prototype, assume NULL previous prototype, INCREF new value,
 * tolerate NULL.
 */
#define DUK_HOBJECT_SET_PROTOTYPE_INIT_INCREF(thr, h, proto) \
	do { \
		duk_hthread *duk__thr = (thr); \
		duk_hobject *duk__obj = (h); \
		duk_hobject *duk__proto = (proto); \
		DUK_UNREF(duk__thr); \
		DUK_ASSERT(DUK_HOBJECT_GET_PROTOTYPE(duk__thr->heap, duk__obj) == NULL); \
		DUK_HOBJECT_SET_PROTOTYPE(duk__thr->heap, duk__obj, duk__proto); \
		DUK_HOBJECT_INCREF_ALLOWNULL(duk__thr, duk__proto); \
	} while (0)

/*
 *  Finalizer check
 */

#if defined(DUK_USE_HEAPPTR16)
#define DUK_HOBJECT_HAS_FINALIZER_FAST(heap, h) duk_hobject_has_finalizer_fast_raw((heap), (h))
#else
#define DUK_HOBJECT_HAS_FINALIZER_FAST(heap, h) duk_hobject_has_finalizer_fast_raw((h))
#endif

/*
 *  Resizing and hash behavior
 */

/* Sanity limit on max number of properties (allocated, not necessarily used).
 * This is somewhat arbitrary, but if we're close to 2**32 properties some
 * algorithms will fail (e.g. hash size selection, next prime selection).
 * Also, we use negative array/entry table indices to indicate 'not found',
 * so anything above 0x80000000 will cause trouble now.
 */
#if defined(DUK_USE_OBJSIZES16)
#define DUK_HOBJECT_MAX_PROPERTIES 0x0000ffffUL
#else
#define DUK_HOBJECT_MAX_PROPERTIES 0x3fffffffUL /* 2**30-1 ~= 1G properties */
#endif

/*
 *  PC-to-line constants
 */

#define DUK_PC2LINE_SKIP 64

/* maximum length for a SKIP-1 diffstream: 35 bits per entry, rounded up to bytes */
#define DUK_PC2LINE_MAX_DIFF_LENGTH (((DUK_PC2LINE_SKIP - 1) * 35 + 7) / 8)

/*
 *  Struct defs
 */

struct duk_propaccessor {
	duk_hobject *get;
	duk_hobject *set;
};

union duk_propvalue {
	/* The get/set pointers could be 16-bit pointer compressed but it
	 * would make no difference on 32-bit platforms because duk_tval is
	 * 8 bytes or more anyway.
	 */
	duk_tval v;
	duk_propaccessor a;
};

struct duk_hobject {
	duk_heaphdr hdr;

	/* Property table: non-index keys, values, attributes. */
#if defined(DUK_USE_HEAPPTR16)
	/* Located in duk_heaphdr h_extra16.  Subclasses of duk_hobject (like
	 * duk_hcompfunc) are not free to use h_extra16 for this reason.
	 */
#else
	duk_uint8_t *props;
#endif

	/* Optional hash part: NULL if none.  Hash size is stored as the first
	 * entry of the hash part to avoid the field for small objects.
	 */
#if defined(DUK_USE_HOBJECT_HASH_PART)
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t hash16;
#else
	duk_uint32_t *hash;
#endif
#endif

	/* Index keys and their hash. */
	/*
	 * i_size propvalues
	 * i_size propkeys (uint32)
	 *
	 * hash for faster access.  "hash" is idx*2 which leaves gaps.
	 * Unused entries 0xffffffff.
	 */
	duk_uint8_t *idx_props;
	duk_uint32_t *idx_hash;
	duk_uint32_t i_size;
	duk_uint32_t i_next;

	/* Prototype: the only internal property lifted outside of entries
	 * as it is so central and most objects have a prototype pointer.
	 */
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t prototype16;
#else
	duk_hobject *prototype;
#endif

#if defined(DUK_USE_OBJSIZES16)
	duk_uint16_t e_size16;
	duk_uint16_t e_next16;
#else
	duk_uint32_t e_size; /* entry part size */
	duk_uint32_t e_next; /* index for next new key ([0,e_next[ are gc reachable, above is garbage) */
#endif
};

/*
 *  Exposed data
 */

#if !defined(DUK_SINGLE_FILE)
DUK_INTERNAL_DECL duk_uint8_t duk_htype_to_stridx[64];
#endif /* !DUK_SINGLE_FILE */

/*
 *  Prototypes
 */

/* alloc and init */
DUK_INTERNAL_DECL duk_hobject *duk_hobject_alloc_unchecked(duk_heap *heap, duk_uint_t hobject_flags);
DUK_INTERNAL_DECL duk_hobject *duk_hobject_alloc(duk_hthread *thr, duk_uint_t hobject_flags);
DUK_INTERNAL_DECL duk_harray *duk_harray_alloc(duk_hthread *thr, duk_uint_t hobject_flags);
DUK_INTERNAL_DECL duk_hcompfunc *duk_hcompfunc_alloc(duk_hthread *thr, duk_uint_t hobject_flags);
DUK_INTERNAL_DECL duk_hnatfunc *duk_hnatfunc_alloc(duk_hthread *thr, duk_uint_t hobject_flags);
DUK_INTERNAL_DECL duk_hboundfunc *duk_hboundfunc_alloc(duk_heap *heap, duk_uint_t hobject_flags);
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
DUK_INTERNAL_DECL duk_hbufobj *duk_hbufobj_alloc(duk_hthread *thr, duk_uint_t hobject_flags);
#endif
DUK_INTERNAL_DECL duk_hthread *duk_hthread_alloc_unchecked(duk_heap *heap, duk_uint_t hobject_flags);
DUK_INTERNAL_DECL duk_hthread *duk_hthread_alloc(duk_hthread *thr, duk_uint_t hobject_flags);
DUK_INTERNAL_DECL duk_hdecenv *duk_hdecenv_alloc(duk_hthread *thr, duk_uint_t hobject_flags);
DUK_INTERNAL_DECL duk_hobjenv *duk_hobjenv_alloc(duk_hthread *thr, duk_uint_t hobject_flags);
DUK_INTERNAL_DECL duk_hproxy *duk_hproxy_alloc(duk_hthread *thr, duk_uint_t hobject_flags);

/* resize */
DUK_INTERNAL_DECL void duk_hobject_realloc_strprops(duk_hthread *thr, duk_hobject *obj, duk_uint32_t new_e_size);
DUK_INTERNAL_DECL void duk_hobject_resize_entrypart(duk_hthread *thr, duk_hobject *obj, duk_uint32_t new_e_size);
DUK_INTERNAL_DECL void duk_hobject_realloc_idxprops(duk_hthread *thr, duk_hobject *obj, duk_uint32_t new_i_size);
#if 0 /*unused*/
DUK_INTERNAL_DECL void duk_hobject_resize_arraypart(duk_hthread *thr,
                                                    duk_hobject *obj,
                                                    duk_uint32_t new_a_size);
#endif
DUK_INTERNAL_DECL duk_int_t duk_hobject_alloc_strentry_checked(duk_hthread *thr, duk_hobject *obj, duk_hstring *key);
DUK_INTERNAL_DECL duk_int_t duk_hobject_alloc_idxentry_checked(duk_hthread *thr, duk_hobject *obj, duk_uint32_t key);

/* low-level property functions */
DUK_INTERNAL_DECL duk_bool_t
duk_hobject_find_entry(duk_heap *heap, duk_hobject *obj, duk_hstring *key, duk_int_t *e_idx, duk_int_t *h_idx);
DUK_INTERNAL_DECL duk_tval *duk_hobject_find_entry_tval_ptr(duk_heap *heap, duk_hobject *obj, duk_hstring *key);
DUK_INTERNAL_DECL duk_tval *duk_hobject_find_entry_tval_ptr_stridx(duk_heap *heap, duk_hobject *obj, duk_small_uint_t stridx);
DUK_INTERNAL_DECL duk_tval *duk_hobject_find_entry_tval_ptr_and_attrs(duk_heap *heap,
                                                                      duk_hobject *obj,
                                                                      duk_hstring *key,
                                                                      duk_uint_t *out_attrs);
DUK_INTERNAL_DECL duk_tval *duk_hobject_find_array_entry_tval_ptr(duk_heap *heap, duk_hobject *obj, duk_uarridx_t i);

/* internal property functions */
#define DUK_DELPROP_FLAG_THROW (1U << 0)
#define DUK_DELPROP_FLAG_FORCE (1U << 1)
DUK_INTERNAL_DECL duk_size_t duk_hobject_get_length(duk_hthread *thr, duk_hobject *obj);
#if defined(DUK_USE_HEAPPTR16)
DUK_INTERNAL_DECL duk_bool_t duk_hobject_has_finalizer_fast_raw(duk_heap *heap, duk_hobject *obj);
#else
DUK_INTERNAL_DECL duk_bool_t duk_hobject_has_finalizer_fast_raw(duk_hobject *obj);
#endif

/* Object built-in methods */
DUK_INTERNAL_DECL void duk_hobject_object_seal_freeze_helper(duk_hthread *thr, duk_hobject *obj, duk_bool_t is_freeze);
DUK_INTERNAL_DECL duk_bool_t duk_hobject_object_is_sealed_frozen_helper(duk_hthread *thr, duk_hobject *obj, duk_bool_t is_frozen);

/* internal properties */
DUK_INTERNAL_DECL duk_tval *duk_hobject_get_internal_value_tval_ptr(duk_heap *heap, duk_hobject *obj);
DUK_INTERNAL_DECL duk_hstring *duk_hobject_get_internal_value_string(duk_heap *heap, duk_hobject *obj);
DUK_INTERNAL_DECL duk_harray *duk_hobject_get_formals(duk_hthread *thr, duk_hobject *obj);
DUK_INTERNAL_DECL duk_hobject *duk_hobject_get_varmap(duk_hthread *thr, duk_hobject *obj);

/* hobject management functions */
DUK_INTERNAL_DECL void duk_hobject_compact_object(duk_hthread *thr, duk_hobject *obj);

/* Proxy */
DUK_INTERNAL_DECL duk_hobject *duk_hobject_resolve_proxy_target(duk_hobject *obj);
DUK_INTERNAL_DECL duk_hobject *duk_proxy_get_target_autothrow(duk_hthread *thr, duk_hproxy *h);
#if defined(DUK_USE_ES6_PROXY)
DUK_INTERNAL_DECL duk_bool_t duk_proxy_trap_check_strkey(duk_hthread *thr,
                                                         duk_hproxy *h,
                                                         duk_hstring *key,
                                                         duk_small_uint_t trap_stridx);
DUK_INTERNAL_DECL duk_bool_t duk_proxy_trap_check_idxkey(duk_hthread *thr,
                                                         duk_hproxy *h,
                                                         duk_uarridx_t idx,
                                                         duk_small_uint_t trap_stridx);
DUK_INTERNAL_DECL duk_bool_t duk_proxy_trap_check_nokey(duk_hthread *thr, duk_hproxy *h, duk_small_uint_t trap_stridx);
#endif

/* macros */
DUK_INTERNAL_DECL duk_hobject *duk_hobject_get_proto_raw(duk_heap *heap, duk_hobject *h);
DUK_INTERNAL_DECL void duk_hobject_set_proto_raw_updref(duk_hthread *thr, duk_hobject *h, duk_hobject *p);

/* pc2line */
#if defined(DUK_USE_PC2LINE)
DUK_INTERNAL_DECL void duk_hobject_pc2line_pack(duk_hthread *thr, duk_compiler_instr *instrs, duk_uint_fast32_t length);
DUK_INTERNAL_DECL duk_uint_fast32_t duk_hobject_pc2line_query(duk_hthread *thr, duk_idx_t idx_func, duk_uint_fast32_t pc);
#endif

/* misc */
DUK_INTERNAL_DECL duk_bool_t duk_hobject_prototype_chain_contains(duk_hthread *thr,
                                                                  duk_hobject *h,
                                                                  duk_hobject *p,
                                                                  duk_bool_t ignore_loop);
DUK_INTERNAL_DECL void duk_hobject_get_props_key_attr(duk_heap *heap,
                                                      duk_hobject *obj,
                                                      duk_propvalue **out_val_base,
                                                      duk_hstring ***out_key_base,
                                                      duk_uint8_t **out_attr_base);
DUK_INTERNAL_DECL duk_propvalue *duk_hobject_get_props(duk_heap *heap, duk_hobject *h);
DUK_INTERNAL_DECL duk_uint32_t duk_hobject_get_esize(duk_hobject *h);
DUK_INTERNAL_DECL duk_uint32_t duk_hobject_get_enext(duk_hobject *h);
DUK_INTERNAL_DECL duk_size_t duk_hobject_get_ebytes(duk_hobject *h);
DUK_INTERNAL_DECL duk_uint32_t duk_hobject_get_isize(duk_hobject *h);
DUK_INTERNAL_DECL duk_uint32_t duk_hobject_get_inext(duk_hobject *h);
DUK_INTERNAL_DECL duk_size_t duk_hobject_get_ibytes(duk_hobject *h);
DUK_INTERNAL_DECL duk_uint32_t duk_hobject_get_hsize(duk_heap *heap, duk_hobject *h);
DUK_INTERNAL_DECL duk_size_t duk_hobject_get_hbytes(duk_heap *heap, duk_hobject *h);
DUK_INTERNAL_DECL duk_uint32_t duk_hobject_get_asize(duk_hobject *h);
DUK_INTERNAL_DECL duk_size_t duk_hobject_get_abytes(duk_hobject *h);

#if !defined(DUK_USE_OBJECT_BUILTIN)
/* These declarations are needed when related built-in is disabled and
 * genbuiltins.py won't automatically emit the declerations.
 */
DUK_INTERNAL_DECL duk_ret_t duk_bi_object_prototype_to_string(duk_hthread *thr);
DUK_INTERNAL_DECL duk_ret_t duk_bi_function_prototype(duk_hthread *thr);
#endif

DUK_INTERNAL_DECL duk_uint32_t duk_hobject_get_min_grow_e(duk_uint32_t e_size);
DUK_INTERNAL_DECL duk_uint32_t duk_hobject_get_min_grow_i(duk_uint32_t i_size);
DUK_INTERNAL_DECL duk_uint32_t duk_hobject_get_min_grow_a(duk_uint32_t a_size);

DUK_INTERNAL_DECL duk_uint32_t duk_harray_count_used_items(duk_heap *heap, duk_harray *a);

DUK_INTERNAL_DECL void duk_hobject_abandon_array_items(duk_hthread *thr, duk_hobject *obj);

DUK_INTERNAL_DECL void duk_harray_grow_items_for_size(duk_hthread *thr, duk_hobject *obj, duk_uint32_t new_items_min_length);
DUK_INTERNAL_DECL duk_tval *duk_harray_append_reserve_items(duk_hthread *thr,
                                                            duk_harray *a,
                                                            duk_uarridx_t start_idx,
                                                            duk_uint32_t count);

DUK_INTERNAL_DECL duk_bool_t duk_hobject_lookup_strprop_index(duk_hthread *thr,
                                                              duk_hobject *obj,
                                                              duk_hstring *key,
                                                              duk_uint_fast32_t *out_idx);
DUK_INTERNAL_DECL duk_bool_t duk_hobject_lookup_strprop_indices(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_hstring *key,
                                                                duk_uint_fast32_t *out_idx,
                                                                duk_int_fast32_t *out_hashidx);
DUK_INTERNAL_DECL duk_bool_t duk_hobject_lookup_strprop_val_attrs(duk_hthread *thr,
                                                                  duk_hobject *obj,
                                                                  duk_hstring *key,
                                                                  duk_propvalue **out_valptr,
                                                                  duk_uint8_t *out_attrs);

DUK_INTERNAL_DECL duk_bool_t duk_hobject_lookup_idxprop_index(duk_hthread *thr,
                                                              duk_hobject *obj,
                                                              duk_uarridx_t idx,
                                                              duk_uint_fast32_t *out_idx);
DUK_INTERNAL_DECL duk_bool_t duk_hobject_lookup_idxprop_indices(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_uarridx_t idx,
                                                                duk_uint_fast32_t *out_idx,
                                                                duk_int_fast32_t *out_hashidx);
DUK_INTERNAL_DECL duk_bool_t duk_hobject_lookup_idxprop_val_attrs(duk_hthread *thr,
                                                                  duk_hobject *obj,
                                                                  duk_uarridx_t idx,
                                                                  duk_propvalue **out_valptr,
                                                                  duk_uint8_t *out_attrs);

DUK_INTERNAL_DECL duk_hobject *duk_hobject_lookup_strprop_known_hobject(duk_hthread *thr, duk_hobject *obj, duk_hstring *key);
DUK_INTERNAL_DECL duk_tval *duk_hobject_lookup_strprop_data_tvalptr(duk_hthread *thr, duk_hobject *obj, duk_hstring *key);
DUK_INTERNAL_DECL duk_hstring *duk_hobject_lookup_intvalue_hstring(duk_hthread *thr, duk_hobject *obj);

DUK_INTERNAL_DECL duk_uint32_t duk_hobject_compute_uarridx_hash(duk_uarridx_t idx);

DUK_INTERNAL_DECL void duk_hobject_start_critical(duk_hthread *thr,
                                                  duk_small_uint_t *prev_ms_base_flags,
                                                  duk_small_uint_t flags_to_set,
                                                  duk_bool_t *prev_error_not_allowed);
DUK_INTERNAL_DECL void duk_hobject_end_critical(duk_hthread *thr,
                                                duk_small_uint_t *prev_ms_base_flags,
                                                duk_bool_t *prev_error_not_allowed);

DUK_INTERNAL_DECL duk_tval *duk_hobject_obtain_arridx_slot(duk_hthread *thr, duk_uint32_t arr_idx, duk_hobject *obj);

DUK_INTERNAL_DECL duk_uint32_t duk_to_array_length_checked(duk_hthread *thr, duk_tval *tv);
DUK_INTERNAL_DECL duk_bool_t duk_harray_put_array_length_u32_smaller(duk_hthread *thr,
                                                                     duk_hobject *obj,
                                                                     duk_uint32_t old_len,
                                                                     duk_uint32_t new_len,
                                                                     duk_bool_t force_flag);
DUK_INTERNAL_DECL duk_bool_t duk_harray_put_array_length_u32(duk_hthread *thr,
                                                             duk_hobject *obj,
                                                             duk_uint32_t new_len,
                                                             duk_bool_t force_flag);
DUK_INTERNAL_DECL duk_bool_t duk_harray_put_array_length_top(duk_hthread *thr, duk_hobject *obj, duk_bool_t force_flag);

#endif /* DUK_HOBJECT_H_INCLUDED */
