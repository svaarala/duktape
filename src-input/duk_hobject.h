/*
 *  Heap object representation.
 *
 *  Heap objects are used for Ecmascript objects, arrays, and functions,
 *  but also for internal control like declarative and object environment
 *  records.  Compiled functions, native functions, and threads are also
 *  objects but with an extended C struct.
 *
 *  Objects provide the required Ecmascript semantics and exotic behaviors
 *  especially for property access.
 *
 *  Properties are stored in three conceptual parts:
 *
 *    1. A linear 'entry part' contains ordered key-value-attributes triples
 *       and is the main method of string properties.
 *
 *    2. An optional linear 'array part' is used for array objects to store a
 *       (dense) range of [0,N[ array indexed entries with default attributes
 *       (writable, enumerable, configurable).  If the array part would become
 *       sparse or non-default attributes are required, the array part is
 *       abandoned and moved to the 'entry part'.
 *
 *    3. An optional 'hash part' is used to optimize lookups of the entry
 *       part; it is used only for objects with sufficiently many properties
 *       and can be abandoned without loss of information.
 *
 *  These three conceptual parts are stored in a single memory allocated area.
 *  This minimizes memory allocation overhead but also means that all three
 *  parts are resized together, and makes property access a bit complicated.
 */

#ifndef DUK_HOBJECT_H_INCLUDED
#define DUK_HOBJECT_H_INCLUDED

/* Object flag.  There are currently 25 flag bits available.  Make sure
 * this stays in sync with debugger object inspection code.
 */

/* XXX: some flags are object subtype specific (e.g. common to all function
 * subtypes, duk_harray, etc) and could be reused for different subtypes.
 */
#define DUK_HOBJECT_FLAG_EXTENSIBLE            DUK_HEAPHDR_USER_FLAG(0)   /* object is extensible */
#define DUK_HOBJECT_FLAG_CONSTRUCTABLE         DUK_HEAPHDR_USER_FLAG(1)   /* object is constructable */
#define DUK_HOBJECT_FLAG_BOUNDFUNC             DUK_HEAPHDR_USER_FLAG(2)   /* object established using Function.prototype.bind() */
#define DUK_HOBJECT_FLAG_COMPFUNC              DUK_HEAPHDR_USER_FLAG(4)   /* object is a compiled function (duk_hcompfunc) */
#define DUK_HOBJECT_FLAG_NATFUNC               DUK_HEAPHDR_USER_FLAG(5)   /* object is a native function (duk_hnatfunc) */
#define DUK_HOBJECT_FLAG_BUFOBJ                DUK_HEAPHDR_USER_FLAG(6)   /* object is a buffer object (duk_hbufobj) (always exotic) */
#define DUK_HOBJECT_FLAG_THREAD                DUK_HEAPHDR_USER_FLAG(7)   /* object is a thread (duk_hthread) */
#define DUK_HOBJECT_FLAG_ARRAY_PART            DUK_HEAPHDR_USER_FLAG(8)   /* object has an array part (a_size may still be 0) */
#define DUK_HOBJECT_FLAG_STRICT                DUK_HEAPHDR_USER_FLAG(9)   /* function: function object is strict */
#define DUK_HOBJECT_FLAG_NOTAIL                DUK_HEAPHDR_USER_FLAG(10)  /* function: function must not be tail called */
#define DUK_HOBJECT_FLAG_NEWENV                DUK_HEAPHDR_USER_FLAG(11)  /* function: create new environment when called (see duk_hcompfunc) */
#define DUK_HOBJECT_FLAG_NAMEBINDING           DUK_HEAPHDR_USER_FLAG(12)  /* function: create binding for func name (function templates only, used for named function expressions) */
#define DUK_HOBJECT_FLAG_CREATEARGS            DUK_HEAPHDR_USER_FLAG(13)  /* function: create an arguments object on function call */
#define DUK_HOBJECT_FLAG_ENVRECCLOSED          DUK_HEAPHDR_USER_FLAG(14)  /* envrec: (declarative) record is closed */
#define DUK_HOBJECT_FLAG_EXOTIC_ARRAY          DUK_HEAPHDR_USER_FLAG(15)  /* 'Array' object, array length and index exotic behavior */
#define DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ      DUK_HEAPHDR_USER_FLAG(16)  /* 'String' object, array index exotic behavior */
#define DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS      DUK_HEAPHDR_USER_FLAG(17)  /* 'Arguments' object and has arguments exotic behavior (non-strict callee) */
#define DUK_HOBJECT_FLAG_EXOTIC_DUKFUNC        DUK_HEAPHDR_USER_FLAG(18)  /* Duktape/C (nativefunction) object, exotic 'length' */
#define DUK_HOBJECT_FLAG_EXOTIC_PROXYOBJ       DUK_HEAPHDR_USER_FLAG(19)  /* 'Proxy' object */

#define DUK_HOBJECT_FLAG_CLASS_BASE            DUK_HEAPHDR_USER_FLAG_NUMBER(20)
#define DUK_HOBJECT_FLAG_CLASS_BITS            5

#define DUK_HOBJECT_GET_CLASS_NUMBER(h)        \
	DUK_HEAPHDR_GET_FLAG_RANGE(&(h)->hdr, DUK_HOBJECT_FLAG_CLASS_BASE, DUK_HOBJECT_FLAG_CLASS_BITS)
#define DUK_HOBJECT_SET_CLASS_NUMBER(h,v)      \
	DUK_HEAPHDR_SET_FLAG_RANGE(&(h)->hdr, DUK_HOBJECT_FLAG_CLASS_BASE, DUK_HOBJECT_FLAG_CLASS_BITS, (v))

#define DUK_HOBJECT_GET_CLASS_MASK(h)          \
	(1UL << DUK_HEAPHDR_GET_FLAG_RANGE(&(h)->hdr, DUK_HOBJECT_FLAG_CLASS_BASE, DUK_HOBJECT_FLAG_CLASS_BITS))

/* Macro for creating flag initializer from a class number.
 * Unsigned type cast is needed to avoid warnings about coercing
 * a signed integer to an unsigned one; the largest class values
 * have the highest bit (bit 31) set which causes this.
 */
#define DUK_HOBJECT_CLASS_AS_FLAGS(v)          (((duk_uint_t) (v)) << DUK_HOBJECT_FLAG_CLASS_BASE)

/* E5 Section 8.6.2 + custom classes */
#define DUK_HOBJECT_CLASS_NONE                 0
#define DUK_HOBJECT_CLASS_ARGUMENTS            1
#define DUK_HOBJECT_CLASS_ARRAY                2
#define DUK_HOBJECT_CLASS_BOOLEAN              3
#define DUK_HOBJECT_CLASS_DATE                 4
#define DUK_HOBJECT_CLASS_ERROR                5
#define DUK_HOBJECT_CLASS_FUNCTION             6
#define DUK_HOBJECT_CLASS_JSON                 7
#define DUK_HOBJECT_CLASS_MATH                 8
#define DUK_HOBJECT_CLASS_NUMBER               9
#define DUK_HOBJECT_CLASS_OBJECT               10
#define DUK_HOBJECT_CLASS_REGEXP               11
#define DUK_HOBJECT_CLASS_STRING               12
#define DUK_HOBJECT_CLASS_GLOBAL               13
#define DUK_HOBJECT_CLASS_OBJENV               14  /* custom */
#define DUK_HOBJECT_CLASS_DECENV               15  /* custom */
#define DUK_HOBJECT_CLASS_POINTER              16  /* custom */
#define DUK_HOBJECT_CLASS_THREAD               17  /* custom; implies DUK_HOBJECT_IS_THREAD */
#define DUK_HOBJECT_CLASS_BUFOBJ_MIN           18
#define DUK_HOBJECT_CLASS_ARRAYBUFFER          18  /* implies DUK_HOBJECT_IS_BUFOBJ */
#define DUK_HOBJECT_CLASS_DATAVIEW             19
#define DUK_HOBJECT_CLASS_INT8ARRAY            20
#define DUK_HOBJECT_CLASS_UINT8ARRAY           21
#define DUK_HOBJECT_CLASS_UINT8CLAMPEDARRAY    22
#define DUK_HOBJECT_CLASS_INT16ARRAY           23
#define DUK_HOBJECT_CLASS_UINT16ARRAY          24
#define DUK_HOBJECT_CLASS_INT32ARRAY           25
#define DUK_HOBJECT_CLASS_UINT32ARRAY          26
#define DUK_HOBJECT_CLASS_FLOAT32ARRAY         27
#define DUK_HOBJECT_CLASS_FLOAT64ARRAY         28
#define DUK_HOBJECT_CLASS_BUFFER               29  /* custom; implies DUK_HOBJECT_IS_BUFOBJ */
#define DUK_HOBJECT_CLASS_BUFOBJ_MAX           29
#define DUK_HOBJECT_CLASS_MAX                  29

/* class masks */
#define DUK_HOBJECT_CMASK_ALL                  ((1UL << (DUK_HOBJECT_CLASS_MAX + 1)) - 1UL)
#define DUK_HOBJECT_CMASK_NONE                 (1UL << DUK_HOBJECT_CLASS_NONE)
#define DUK_HOBJECT_CMASK_ARGUMENTS            (1UL << DUK_HOBJECT_CLASS_ARGUMENTS)
#define DUK_HOBJECT_CMASK_ARRAY                (1UL << DUK_HOBJECT_CLASS_ARRAY)
#define DUK_HOBJECT_CMASK_BOOLEAN              (1UL << DUK_HOBJECT_CLASS_BOOLEAN)
#define DUK_HOBJECT_CMASK_DATE                 (1UL << DUK_HOBJECT_CLASS_DATE)
#define DUK_HOBJECT_CMASK_ERROR                (1UL << DUK_HOBJECT_CLASS_ERROR)
#define DUK_HOBJECT_CMASK_FUNCTION             (1UL << DUK_HOBJECT_CLASS_FUNCTION)
#define DUK_HOBJECT_CMASK_JSON                 (1UL << DUK_HOBJECT_CLASS_JSON)
#define DUK_HOBJECT_CMASK_MATH                 (1UL << DUK_HOBJECT_CLASS_MATH)
#define DUK_HOBJECT_CMASK_NUMBER               (1UL << DUK_HOBJECT_CLASS_NUMBER)
#define DUK_HOBJECT_CMASK_OBJECT               (1UL << DUK_HOBJECT_CLASS_OBJECT)
#define DUK_HOBJECT_CMASK_REGEXP               (1UL << DUK_HOBJECT_CLASS_REGEXP)
#define DUK_HOBJECT_CMASK_STRING               (1UL << DUK_HOBJECT_CLASS_STRING)
#define DUK_HOBJECT_CMASK_GLOBAL               (1UL << DUK_HOBJECT_CLASS_GLOBAL)
#define DUK_HOBJECT_CMASK_OBJENV               (1UL << DUK_HOBJECT_CLASS_OBJENV)
#define DUK_HOBJECT_CMASK_DECENV               (1UL << DUK_HOBJECT_CLASS_DECENV)
#define DUK_HOBJECT_CMASK_BUFFER               (1UL << DUK_HOBJECT_CLASS_BUFFER)
#define DUK_HOBJECT_CMASK_POINTER              (1UL << DUK_HOBJECT_CLASS_POINTER)
#define DUK_HOBJECT_CMASK_THREAD               (1UL << DUK_HOBJECT_CLASS_THREAD)
#define DUK_HOBJECT_CMASK_ARRAYBUFFER          (1UL << DUK_HOBJECT_CLASS_ARRAYBUFFER)
#define DUK_HOBJECT_CMASK_DATAVIEW             (1UL << DUK_HOBJECT_CLASS_DATAVIEW)
#define DUK_HOBJECT_CMASK_INT8ARRAY            (1UL << DUK_HOBJECT_CLASS_INT8ARRAY)
#define DUK_HOBJECT_CMASK_UINT8ARRAY           (1UL << DUK_HOBJECT_CLASS_UINT8ARRAY)
#define DUK_HOBJECT_CMASK_UINT8CLAMPEDARRAY    (1UL << DUK_HOBJECT_CLASS_UINT8CLAMPEDARRAY)
#define DUK_HOBJECT_CMASK_INT16ARRAY           (1UL << DUK_HOBJECT_CLASS_INT16ARRAY)
#define DUK_HOBJECT_CMASK_UINT16ARRAY          (1UL << DUK_HOBJECT_CLASS_UINT16ARRAY)
#define DUK_HOBJECT_CMASK_INT32ARRAY           (1UL << DUK_HOBJECT_CLASS_INT32ARRAY)
#define DUK_HOBJECT_CMASK_UINT32ARRAY          (1UL << DUK_HOBJECT_CLASS_UINT32ARRAY)
#define DUK_HOBJECT_CMASK_FLOAT32ARRAY         (1UL << DUK_HOBJECT_CLASS_FLOAT32ARRAY)
#define DUK_HOBJECT_CMASK_FLOAT64ARRAY         (1UL << DUK_HOBJECT_CLASS_FLOAT64ARRAY)

#define DUK_HOBJECT_CMASK_ALL_BUFOBJS \
	(DUK_HOBJECT_CMASK_BUFFER | \
	 DUK_HOBJECT_CMASK_ARRAYBUFFER | \
	 DUK_HOBJECT_CMASK_DATAVIEW | \
	 DUK_HOBJECT_CMASK_INT8ARRAY | \
	 DUK_HOBJECT_CMASK_UINT8ARRAY | \
	 DUK_HOBJECT_CMASK_UINT8CLAMPEDARRAY | \
	 DUK_HOBJECT_CMASK_INT16ARRAY | \
	 DUK_HOBJECT_CMASK_UINT16ARRAY | \
	 DUK_HOBJECT_CMASK_INT32ARRAY | \
	 DUK_HOBJECT_CMASK_UINT32ARRAY | \
	 DUK_HOBJECT_CMASK_FLOAT32ARRAY | \
	 DUK_HOBJECT_CMASK_FLOAT64ARRAY)

#define DUK_HOBJECT_IS_OBJENV(h)               (DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_OBJENV)
#define DUK_HOBJECT_IS_DECENV(h)               (DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_DECENV)
#define DUK_HOBJECT_IS_ENV(h)                  (DUK_HOBJECT_IS_OBJENV((h)) || DUK_HOBJECT_IS_DECENV((h)))
#define DUK_HOBJECT_IS_ARRAY(h)                DUK_HOBJECT_HAS_EXOTIC_ARRAY((h))  /* Rely on class Array <=> exotic Array */
#define DUK_HOBJECT_IS_BOUNDFUNC(h)            DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BOUNDFUNC)
#define DUK_HOBJECT_IS_COMPFUNC(h)             DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_COMPFUNC)
#define DUK_HOBJECT_IS_NATFUNC(h)              DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NATFUNC)
#define DUK_HOBJECT_IS_BUFOBJ(h)               DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BUFOBJ)
#define DUK_HOBJECT_IS_THREAD(h)               DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_THREAD)

#define DUK_HOBJECT_IS_NONBOUND_FUNCTION(h)    DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, \
                                                        DUK_HOBJECT_FLAG_COMPFUNC | \
                                                        DUK_HOBJECT_FLAG_NATFUNC)

#define DUK_HOBJECT_IS_FUNCTION(h)             DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, \
                                                        DUK_HOBJECT_FLAG_BOUNDFUNC | \
                                                        DUK_HOBJECT_FLAG_COMPFUNC | \
                                                        DUK_HOBJECT_FLAG_NATFUNC)

#define DUK_HOBJECT_IS_CALLABLE(h)             DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, \
                                                        DUK_HOBJECT_FLAG_BOUNDFUNC | \
                                                        DUK_HOBJECT_FLAG_COMPFUNC | \
                                                        DUK_HOBJECT_FLAG_NATFUNC)

/* object has any exotic behavior(s) */
#define DUK_HOBJECT_EXOTIC_BEHAVIOR_FLAGS      (DUK_HOBJECT_FLAG_EXOTIC_ARRAY | \
                                                DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS | \
                                                DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ | \
                                                DUK_HOBJECT_FLAG_EXOTIC_DUKFUNC | \
                                                DUK_HOBJECT_FLAG_BUFOBJ | \
                                                DUK_HOBJECT_FLAG_EXOTIC_PROXYOBJ)
#define DUK_HOBJECT_HAS_EXOTIC_BEHAVIOR(h)     DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_EXOTIC_BEHAVIOR_FLAGS)

/* object has any virtual properties (not counting Proxy behavior) */
#define DUK_HOBJECT_VIRTUAL_PROPERTY_FLAGS     (DUK_HOBJECT_FLAG_EXOTIC_ARRAY | \
                                                DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ | \
                                                DUK_HOBJECT_FLAG_EXOTIC_DUKFUNC | \
                                                DUK_HOBJECT_FLAG_BUFOBJ)
#define DUK_HOBJECT_HAS_VIRTUAL_PROPERTIES(h)  DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_VIRTUAL_PROPERTY_FLAGS)

#define DUK_HOBJECT_HAS_EXTENSIBLE(h)          DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXTENSIBLE)
#define DUK_HOBJECT_HAS_CONSTRUCTABLE(h)       DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CONSTRUCTABLE)
#define DUK_HOBJECT_HAS_BOUNDFUNC(h)           DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BOUNDFUNC)
#define DUK_HOBJECT_HAS_COMPFUNC(h)            DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_COMPFUNC)
#define DUK_HOBJECT_HAS_NATFUNC(h)             DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NATFUNC)
#define DUK_HOBJECT_HAS_BUFOBJ(h)              DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BUFOBJ)
#define DUK_HOBJECT_HAS_THREAD(h)              DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_THREAD)
#define DUK_HOBJECT_HAS_ARRAY_PART(h)          DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_ARRAY_PART)
#define DUK_HOBJECT_HAS_STRICT(h)              DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_STRICT)
#define DUK_HOBJECT_HAS_NOTAIL(h)              DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NOTAIL)
#define DUK_HOBJECT_HAS_NEWENV(h)              DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NEWENV)
#define DUK_HOBJECT_HAS_NAMEBINDING(h)         DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NAMEBINDING)
#define DUK_HOBJECT_HAS_CREATEARGS(h)          DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CREATEARGS)
#define DUK_HOBJECT_HAS_ENVRECCLOSED(h)        DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_ENVRECCLOSED)
#define DUK_HOBJECT_HAS_EXOTIC_ARRAY(h)        DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_ARRAY)
#define DUK_HOBJECT_HAS_EXOTIC_STRINGOBJ(h)    DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ)
#define DUK_HOBJECT_HAS_EXOTIC_ARGUMENTS(h)    DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS)
#define DUK_HOBJECT_HAS_EXOTIC_DUKFUNC(h)      DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_DUKFUNC)
#define DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(h)     DUK_HEAPHDR_CHECK_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_PROXYOBJ)

#define DUK_HOBJECT_SET_EXTENSIBLE(h)          DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXTENSIBLE)
#define DUK_HOBJECT_SET_CONSTRUCTABLE(h)       DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CONSTRUCTABLE)
#define DUK_HOBJECT_SET_BOUNDFUNC(h)           DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BOUNDFUNC)
#define DUK_HOBJECT_SET_COMPFUNC(h)            DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_COMPFUNC)
#define DUK_HOBJECT_SET_NATFUNC(h)             DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NATFUNC)
#define DUK_HOBJECT_SET_BUFOBJ(h)              DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BUFOBJ)
#define DUK_HOBJECT_SET_THREAD(h)              DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_THREAD)
#define DUK_HOBJECT_SET_ARRAY_PART(h)          DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_ARRAY_PART)
#define DUK_HOBJECT_SET_STRICT(h)              DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_STRICT)
#define DUK_HOBJECT_SET_NOTAIL(h)              DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NOTAIL)
#define DUK_HOBJECT_SET_NEWENV(h)              DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NEWENV)
#define DUK_HOBJECT_SET_NAMEBINDING(h)         DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NAMEBINDING)
#define DUK_HOBJECT_SET_CREATEARGS(h)          DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CREATEARGS)
#define DUK_HOBJECT_SET_ENVRECCLOSED(h)        DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_ENVRECCLOSED)
#define DUK_HOBJECT_SET_EXOTIC_ARRAY(h)        DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_ARRAY)
#define DUK_HOBJECT_SET_EXOTIC_STRINGOBJ(h)    DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ)
#define DUK_HOBJECT_SET_EXOTIC_ARGUMENTS(h)    DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS)
#define DUK_HOBJECT_SET_EXOTIC_DUKFUNC(h)      DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_DUKFUNC)
#define DUK_HOBJECT_SET_EXOTIC_PROXYOBJ(h)     DUK_HEAPHDR_SET_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_PROXYOBJ)

#define DUK_HOBJECT_CLEAR_EXTENSIBLE(h)        DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXTENSIBLE)
#define DUK_HOBJECT_CLEAR_CONSTRUCTABLE(h)     DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CONSTRUCTABLE)
#define DUK_HOBJECT_CLEAR_BOUNDFUNC(h)         DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BOUNDFUNC)
#define DUK_HOBJECT_CLEAR_COMPFUNC(h)          DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_COMPFUNC)
#define DUK_HOBJECT_CLEAR_NATFUNC(h)           DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NATFUNC)
#define DUK_HOBJECT_CLEAR_BUFOBJ(h)            DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_BUFOBJ)
#define DUK_HOBJECT_CLEAR_THREAD(h)            DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_THREAD)
#define DUK_HOBJECT_CLEAR_ARRAY_PART(h)        DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_ARRAY_PART)
#define DUK_HOBJECT_CLEAR_STRICT(h)            DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_STRICT)
#define DUK_HOBJECT_CLEAR_NOTAIL(h)            DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NOTAIL)
#define DUK_HOBJECT_CLEAR_NEWENV(h)            DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NEWENV)
#define DUK_HOBJECT_CLEAR_NAMEBINDING(h)       DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_NAMEBINDING)
#define DUK_HOBJECT_CLEAR_CREATEARGS(h)        DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_CREATEARGS)
#define DUK_HOBJECT_CLEAR_ENVRECCLOSED(h)      DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_ENVRECCLOSED)
#define DUK_HOBJECT_CLEAR_EXOTIC_ARRAY(h)      DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_ARRAY)
#define DUK_HOBJECT_CLEAR_EXOTIC_STRINGOBJ(h)  DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ)
#define DUK_HOBJECT_CLEAR_EXOTIC_ARGUMENTS(h)  DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_ARGUMENTS)
#define DUK_HOBJECT_CLEAR_EXOTIC_DUKFUNC(h)    DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_DUKFUNC)
#define DUK_HOBJECT_CLEAR_EXOTIC_PROXYOBJ(h)   DUK_HEAPHDR_CLEAR_FLAG_BITS(&(h)->hdr, DUK_HOBJECT_FLAG_EXOTIC_PROXYOBJ)

/* flags used for property attributes in duk_propdesc and packed flags */
#define DUK_PROPDESC_FLAG_WRITABLE              (1 << 0)    /* E5 Section 8.6.1 */
#define DUK_PROPDESC_FLAG_ENUMERABLE            (1 << 1)    /* E5 Section 8.6.1 */
#define DUK_PROPDESC_FLAG_CONFIGURABLE          (1 << 2)    /* E5 Section 8.6.1 */
#define DUK_PROPDESC_FLAG_ACCESSOR              (1 << 3)    /* accessor */
#define DUK_PROPDESC_FLAG_VIRTUAL               (1 << 4)    /* property is virtual: used in duk_propdesc, never stored
                                                             * (used by e.g. buffer virtual properties)
                                                             */
#define DUK_PROPDESC_FLAGS_MASK                 (DUK_PROPDESC_FLAG_WRITABLE | \
                                                 DUK_PROPDESC_FLAG_ENUMERABLE | \
                                                 DUK_PROPDESC_FLAG_CONFIGURABLE | \
                                                 DUK_PROPDESC_FLAG_ACCESSOR)

/* additional flags which are passed in the same flags argument as property
 * flags but are not stored in object properties.
 */
#define DUK_PROPDESC_FLAG_NO_OVERWRITE          (1 << 4)    /* internal define property: skip write silently if exists */

/* convenience */
#define DUK_PROPDESC_FLAGS_NONE                 0
#define DUK_PROPDESC_FLAGS_W                    (DUK_PROPDESC_FLAG_WRITABLE)
#define DUK_PROPDESC_FLAGS_E                    (DUK_PROPDESC_FLAG_ENUMERABLE)
#define DUK_PROPDESC_FLAGS_C                    (DUK_PROPDESC_FLAG_CONFIGURABLE)
#define DUK_PROPDESC_FLAGS_WE                   (DUK_PROPDESC_FLAG_WRITABLE | DUK_PROPDESC_FLAG_ENUMERABLE)
#define DUK_PROPDESC_FLAGS_WC                   (DUK_PROPDESC_FLAG_WRITABLE | DUK_PROPDESC_FLAG_CONFIGURABLE)
#define DUK_PROPDESC_FLAGS_EC                   (DUK_PROPDESC_FLAG_ENUMERABLE | DUK_PROPDESC_FLAG_CONFIGURABLE)
#define DUK_PROPDESC_FLAGS_WEC                  (DUK_PROPDESC_FLAG_WRITABLE | \
                                                 DUK_PROPDESC_FLAG_ENUMERABLE | \
                                                 DUK_PROPDESC_FLAG_CONFIGURABLE)

/* flags for duk_hobject_get_own_propdesc() and variants */
#define DUK_GETDESC_FLAG_PUSH_VALUE          (1 << 0)  /* push value to stack */
#define DUK_GETDESC_FLAG_IGNORE_PROTOLOOP    (1 << 1)  /* don't throw for prototype loop */

/*
 *  Macro for object validity check
 *
 *  Assert for currently guaranteed relations between flags, for instance.
 */

#define DUK_ASSERT_HOBJECT_VALID(h) do { \
		DUK_ASSERT((h) != NULL); \
		DUK_ASSERT(!DUK_HOBJECT_IS_CALLABLE((h)) || \
		           DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_FUNCTION); \
		DUK_ASSERT(!DUK_HOBJECT_IS_BUFOBJ((h)) || \
		           (DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_BUFFER || \
		            DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_ARRAYBUFFER || \
		            DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_DATAVIEW || \
		            DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_INT8ARRAY || \
		            DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_UINT8ARRAY || \
		            DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_UINT8CLAMPEDARRAY || \
		            DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_INT16ARRAY || \
		            DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_UINT16ARRAY || \
		            DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_INT32ARRAY || \
		            DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_UINT32ARRAY || \
		            DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_FLOAT32ARRAY || \
		            DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_FLOAT64ARRAY)); \
		/* Object is an Array <=> object has exotic array behavior */ \
		DUK_ASSERT((DUK_HOBJECT_GET_CLASS_NUMBER((h)) == DUK_HOBJECT_CLASS_ARRAY && DUK_HOBJECT_HAS_EXOTIC_ARRAY((h))) || \
		           (DUK_HOBJECT_GET_CLASS_NUMBER((h)) != DUK_HOBJECT_CLASS_ARRAY && !DUK_HOBJECT_HAS_EXOTIC_ARRAY((h)))); \
	} while (0)

/*
 *  Macros to access the 'props' allocation.
 */

#if defined(DUK_USE_HEAPPTR16)
#define DUK_HOBJECT_GET_PROPS(heap,h) \
	((duk_uint8_t *) DUK_USE_HEAPPTR_DEC16((heap)->heap_udata, ((duk_heaphdr *) (h))->h_extra16))
#define DUK_HOBJECT_SET_PROPS(heap,h,x) do { \
		((duk_heaphdr *) (h))->h_extra16 = DUK_USE_HEAPPTR_ENC16((heap)->heap_udata, (void *) (x)); \
	} while (0)
#else
#define DUK_HOBJECT_GET_PROPS(heap,h) \
	((h)->props)
#define DUK_HOBJECT_SET_PROPS(heap,h,x) do { \
		(h)->props = (duk_uint8_t *) (x); \
	} while (0)
#endif

#if defined(DUK_USE_HOBJECT_LAYOUT_1)
/* LAYOUT 1 */
#define DUK_HOBJECT_E_GET_KEY_BASE(heap,h) \
	((duk_hstring **) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) \
	))
#define DUK_HOBJECT_E_GET_VALUE_BASE(heap,h) \
	((duk_propvalue *) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) + \
			DUK_HOBJECT_GET_ESIZE((h)) * sizeof(duk_hstring *) \
	))
#define DUK_HOBJECT_E_GET_FLAGS_BASE(heap,h) \
	((duk_uint8_t *) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) + DUK_HOBJECT_GET_ESIZE((h)) * (sizeof(duk_hstring *) + sizeof(duk_propvalue)) \
	))
#define DUK_HOBJECT_A_GET_BASE(heap,h) \
	((duk_tval *) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) + \
			DUK_HOBJECT_GET_ESIZE((h)) * (sizeof(duk_hstring *) + sizeof(duk_propvalue) + sizeof(duk_uint8_t)) \
	))
#define DUK_HOBJECT_H_GET_BASE(heap,h) \
	((duk_uint32_t *) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) + \
			DUK_HOBJECT_GET_ESIZE((h)) * (sizeof(duk_hstring *) + sizeof(duk_propvalue) + sizeof(duk_uint8_t)) + \
			DUK_HOBJECT_GET_ASIZE((h)) * sizeof(duk_tval) \
	))
#define DUK_HOBJECT_P_COMPUTE_SIZE(n_ent,n_arr,n_hash) \
	( \
		(n_ent) * (sizeof(duk_hstring *) + sizeof(duk_propvalue) + sizeof(duk_uint8_t)) + \
		(n_arr) * sizeof(duk_tval) + \
		(n_hash) * sizeof(duk_uint32_t) \
	)
#define DUK_HOBJECT_P_SET_REALLOC_PTRS(p_base,set_e_k,set_e_pv,set_e_f,set_a,set_h,n_ent,n_arr,n_hash)  do { \
		(set_e_k) = (duk_hstring **) (void *) (p_base); \
		(set_e_pv) = (duk_propvalue *) (void *) ((set_e_k) + (n_ent)); \
		(set_e_f) = (duk_uint8_t *) (void *) ((set_e_pv) + (n_ent)); \
		(set_a) = (duk_tval *) (void *) ((set_e_f) + (n_ent)); \
		(set_h) = (duk_uint32_t *) (void *) ((set_a) + (n_arr)); \
	} while (0)
#elif defined(DUK_USE_HOBJECT_LAYOUT_2)
/* LAYOUT 2 */
#if (DUK_USE_ALIGN_BY == 4)
#define DUK_HOBJECT_E_FLAG_PADDING(e_sz) ((4 - (e_sz)) & 0x03)
#elif (DUK_USE_ALIGN_BY == 8)
#define DUK_HOBJECT_E_FLAG_PADDING(e_sz) ((8 - (e_sz)) & 0x07)
#elif (DUK_USE_ALIGN_BY == 1)
#define DUK_HOBJECT_E_FLAG_PADDING(e_sz) 0
#else
#error invalid DUK_USE_ALIGN_BY
#endif
#define DUK_HOBJECT_E_GET_KEY_BASE(heap,h) \
	((duk_hstring **) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) + \
			DUK_HOBJECT_GET_ESIZE((h)) * sizeof(duk_propvalue) \
	))
#define DUK_HOBJECT_E_GET_VALUE_BASE(heap,h) \
	((duk_propvalue *) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) \
	))
#define DUK_HOBJECT_E_GET_FLAGS_BASE(heap,h) \
	((duk_uint8_t *) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) + DUK_HOBJECT_GET_ESIZE((h)) * (sizeof(duk_hstring *) + sizeof(duk_propvalue)) \
	))
#define DUK_HOBJECT_A_GET_BASE(heap,h) \
	((duk_tval *) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) + \
			DUK_HOBJECT_GET_ESIZE((h)) * (sizeof(duk_hstring *) + sizeof(duk_propvalue) + sizeof(duk_uint8_t)) + \
			DUK_HOBJECT_E_FLAG_PADDING(DUK_HOBJECT_GET_ESIZE((h))) \
	))
#define DUK_HOBJECT_H_GET_BASE(heap,h) \
	((duk_uint32_t *) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) + \
			DUK_HOBJECT_GET_ESIZE((h)) * (sizeof(duk_hstring *) + sizeof(duk_propvalue) + sizeof(duk_uint8_t)) + \
			DUK_HOBJECT_E_FLAG_PADDING(DUK_HOBJECT_GET_ESIZE((h))) + \
			DUK_HOBJECT_GET_ASIZE((h)) * sizeof(duk_tval) \
	))
#define DUK_HOBJECT_P_COMPUTE_SIZE(n_ent,n_arr,n_hash) \
	( \
		(n_ent) * (sizeof(duk_hstring *) + sizeof(duk_propvalue) + sizeof(duk_uint8_t)) + \
		DUK_HOBJECT_E_FLAG_PADDING((n_ent)) + \
		(n_arr) * sizeof(duk_tval) + \
		(n_hash) * sizeof(duk_uint32_t) \
	)
#define DUK_HOBJECT_P_SET_REALLOC_PTRS(p_base,set_e_k,set_e_pv,set_e_f,set_a,set_h,n_ent,n_arr,n_hash)  do { \
		(set_e_pv) = (duk_propvalue *) (void *) (p_base); \
		(set_e_k) = (duk_hstring **) (void *) ((set_e_pv) + (n_ent)); \
		(set_e_f) = (duk_uint8_t *) (void *) ((set_e_k) + (n_ent)); \
		(set_a) = (duk_tval *) (void *) (((duk_uint8_t *) (set_e_f)) + \
		                                 sizeof(duk_uint8_t) * (n_ent) + \
		                                 DUK_HOBJECT_E_FLAG_PADDING((n_ent))); \
		(set_h) = (duk_uint32_t *) (void *) ((set_a) + (n_arr)); \
	} while (0)
#elif defined(DUK_USE_HOBJECT_LAYOUT_3)
/* LAYOUT 3 */
#define DUK_HOBJECT_E_GET_KEY_BASE(heap,h) \
	((duk_hstring **) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) + \
			DUK_HOBJECT_GET_ESIZE((h)) * sizeof(duk_propvalue) + \
			DUK_HOBJECT_GET_ASIZE((h)) * sizeof(duk_tval) \
	))
#define DUK_HOBJECT_E_GET_VALUE_BASE(heap,h) \
	((duk_propvalue *) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) \
	))
#define DUK_HOBJECT_E_GET_FLAGS_BASE(heap,h) \
	((duk_uint8_t *) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) + \
			DUK_HOBJECT_GET_ESIZE((h)) * (sizeof(duk_propvalue) + sizeof(duk_hstring *)) + \
			DUK_HOBJECT_GET_ASIZE((h)) * sizeof(duk_tval) + \
			DUK_HOBJECT_GET_HSIZE((h)) * sizeof(duk_uint32_t) \
	))
#define DUK_HOBJECT_A_GET_BASE(heap,h) \
	((duk_tval *) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) + \
			DUK_HOBJECT_GET_ESIZE((h)) * sizeof(duk_propvalue) \
	))
#define DUK_HOBJECT_H_GET_BASE(heap,h) \
	((duk_uint32_t *) (void *) ( \
		DUK_HOBJECT_GET_PROPS((heap), (h)) + \
			DUK_HOBJECT_GET_ESIZE((h)) * (sizeof(duk_propvalue) + sizeof(duk_hstring *)) + \
			DUK_HOBJECT_GET_ASIZE((h)) * sizeof(duk_tval) \
	))
#define DUK_HOBJECT_P_COMPUTE_SIZE(n_ent,n_arr,n_hash) \
	( \
		(n_ent) * (sizeof(duk_propvalue) + sizeof(duk_hstring *) + sizeof(duk_uint8_t)) + \
		(n_arr) * sizeof(duk_tval) + \
		(n_hash) * sizeof(duk_uint32_t) \
	)
#define DUK_HOBJECT_P_SET_REALLOC_PTRS(p_base,set_e_k,set_e_pv,set_e_f,set_a,set_h,n_ent,n_arr,n_hash)  do { \
		(set_e_pv) = (duk_propvalue *) (void *) (p_base); \
		(set_a) = (duk_tval *) (void *) ((set_e_pv) + (n_ent)); \
		(set_e_k) = (duk_hstring **) (void *) ((set_a) + (n_arr)); \
		(set_h) = (duk_uint32_t *) (void *) ((set_e_k) + (n_ent)); \
		(set_e_f) = (duk_uint8_t *) (void *) ((set_h) + (n_hash)); \
	} while (0)
#else
#error invalid hobject layout defines
#endif  /* hobject property layout */

#define DUK_HOBJECT_P_ALLOC_SIZE(h) \
	DUK_HOBJECT_P_COMPUTE_SIZE(DUK_HOBJECT_GET_ESIZE((h)), DUK_HOBJECT_GET_ASIZE((h)), DUK_HOBJECT_GET_HSIZE((h)))

#define DUK_HOBJECT_E_GET_KEY(heap,h,i)              (DUK_HOBJECT_E_GET_KEY_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_E_GET_KEY_PTR(heap,h,i)          (&DUK_HOBJECT_E_GET_KEY_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_E_GET_VALUE(heap,h,i)            (DUK_HOBJECT_E_GET_VALUE_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_E_GET_VALUE_PTR(heap,h,i)        (&DUK_HOBJECT_E_GET_VALUE_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_E_GET_VALUE_TVAL(heap,h,i)       (DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).v)
#define DUK_HOBJECT_E_GET_VALUE_TVAL_PTR(heap,h,i)   (&DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).v)
#define DUK_HOBJECT_E_GET_VALUE_GETTER(heap,h,i)     (DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).a.get)
#define DUK_HOBJECT_E_GET_VALUE_GETTER_PTR(heap,h,i) (&DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).a.get)
#define DUK_HOBJECT_E_GET_VALUE_SETTER(heap,h,i)     (DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).a.set)
#define DUK_HOBJECT_E_GET_VALUE_SETTER_PTR(heap,h,i) (&DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).a.set)
#define DUK_HOBJECT_E_GET_FLAGS(heap,h,i)            (DUK_HOBJECT_E_GET_FLAGS_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_E_GET_FLAGS_PTR(heap,h,i)        (&DUK_HOBJECT_E_GET_FLAGS_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_A_GET_VALUE(heap,h,i)            (DUK_HOBJECT_A_GET_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_A_GET_VALUE_PTR(heap,h,i)        (&DUK_HOBJECT_A_GET_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_H_GET_INDEX(heap,h,i)            (DUK_HOBJECT_H_GET_BASE((heap), (h))[(i)])
#define DUK_HOBJECT_H_GET_INDEX_PTR(heap,h,i)        (&DUK_HOBJECT_H_GET_BASE((heap), (h))[(i)])

#define DUK_HOBJECT_E_SET_KEY(heap,h,i,k)  do { \
		DUK_HOBJECT_E_GET_KEY((heap), (h), (i)) = (k); \
	} while (0)
#define DUK_HOBJECT_E_SET_VALUE(heap,h,i,v)  do { \
		DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)) = (v); \
	} while (0)
#define DUK_HOBJECT_E_SET_VALUE_TVAL(heap,h,i,v)  do { \
		DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).v = (v); \
	} while (0)
#define DUK_HOBJECT_E_SET_VALUE_GETTER(heap,h,i,v)  do { \
		DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).a.get = (v); \
	} while (0)
#define DUK_HOBJECT_E_SET_VALUE_SETTER(heap,h,i,v)  do { \
		DUK_HOBJECT_E_GET_VALUE((heap), (h), (i)).a.set = (v); \
	} while (0)
#define DUK_HOBJECT_E_SET_FLAGS(heap,h,i,f)  do { \
		DUK_HOBJECT_E_GET_FLAGS((heap), (h), (i)) = (duk_uint8_t) (f); \
	} while (0)
#define DUK_HOBJECT_A_SET_VALUE(heap,h,i,v)  do { \
		DUK_HOBJECT_A_GET_VALUE((heap), (h), (i)) = (v); \
	} while (0)
#define DUK_HOBJECT_A_SET_VALUE_TVAL(heap,h,i,v) \
	DUK_HOBJECT_A_SET_VALUE((heap), (h), (i), (v))  /* alias for above */
#define DUK_HOBJECT_H_SET_INDEX(heap,h,i,v)  do { \
		DUK_HOBJECT_H_GET_INDEX((heap), (h), (i)) = (v); \
	} while (0)

#define DUK_HOBJECT_E_SET_FLAG_BITS(heap,h,i,mask)  do { \
		DUK_HOBJECT_E_GET_FLAGS_BASE((heap), (h))[(i)] |= (mask); \
	} while (0)

#define DUK_HOBJECT_E_CLEAR_FLAG_BITS(heap,h,i,mask)  do { \
		DUK_HOBJECT_E_GET_FLAGS_BASE((heap), (h))[(i)] &= ~(mask); \
	} while (0)

#define DUK_HOBJECT_E_SLOT_IS_WRITABLE(heap,h,i)     ((DUK_HOBJECT_E_GET_FLAGS((heap), (h), (i)) & DUK_PROPDESC_FLAG_WRITABLE) != 0)
#define DUK_HOBJECT_E_SLOT_IS_ENUMERABLE(heap,h,i)   ((DUK_HOBJECT_E_GET_FLAGS((heap), (h), (i)) & DUK_PROPDESC_FLAG_ENUMERABLE) != 0)
#define DUK_HOBJECT_E_SLOT_IS_CONFIGURABLE(heap,h,i) ((DUK_HOBJECT_E_GET_FLAGS((heap), (h), (i)) & DUK_PROPDESC_FLAG_CONFIGURABLE) != 0)
#define DUK_HOBJECT_E_SLOT_IS_ACCESSOR(heap,h,i)     ((DUK_HOBJECT_E_GET_FLAGS((heap), (h), (i)) & DUK_PROPDESC_FLAG_ACCESSOR) != 0)

#define DUK_HOBJECT_E_SLOT_SET_WRITABLE(heap,h,i)        DUK_HOBJECT_E_SET_FLAG_BITS((heap), (h), (i),DUK_PROPDESC_FLAG_WRITABLE)
#define DUK_HOBJECT_E_SLOT_SET_ENUMERABLE(heap,h,i)      DUK_HOBJECT_E_SET_FLAG_BITS((heap), (h), (i),DUK_PROPDESC_FLAG_ENUMERABLE)
#define DUK_HOBJECT_E_SLOT_SET_CONFIGURABLE(heap,h,i)    DUK_HOBJECT_E_SET_FLAG_BITS((heap), (h), (i),DUK_PROPDESC_FLAG_CONFIGURABLE)
#define DUK_HOBJECT_E_SLOT_SET_ACCESSOR(heap,h,i)        DUK_HOBJECT_E_SET_FLAG_BITS((heap), (h), (i),DUK_PROPDESC_FLAG_ACCESSOR)

#define DUK_HOBJECT_E_SLOT_CLEAR_WRITABLE(heap,h,i)      DUK_HOBJECT_E_CLEAR_FLAG_BITS((heap), (h), (i),DUK_PROPDESC_FLAG_WRITABLE)
#define DUK_HOBJECT_E_SLOT_CLEAR_ENUMERABLE(heap,h,i)    DUK_HOBJECT_E_CLEAR_FLAG_BITS((heap), (h), (i),DUK_PROPDESC_FLAG_ENUMERABLE)
#define DUK_HOBJECT_E_SLOT_CLEAR_CONFIGURABLE(heap,h,i)  DUK_HOBJECT_E_CLEAR_FLAG_BITS((heap), (h), (i),DUK_PROPDESC_FLAG_CONFIGURABLE)
#define DUK_HOBJECT_E_SLOT_CLEAR_ACCESSOR(heap,h,i)      DUK_HOBJECT_E_CLEAR_FLAG_BITS((heap), (h), (i),DUK_PROPDESC_FLAG_ACCESSOR)

#define DUK_PROPDESC_IS_WRITABLE(p)             (((p)->flags & DUK_PROPDESC_FLAG_WRITABLE) != 0)
#define DUK_PROPDESC_IS_ENUMERABLE(p)           (((p)->flags & DUK_PROPDESC_FLAG_ENUMERABLE) != 0)
#define DUK_PROPDESC_IS_CONFIGURABLE(p)         (((p)->flags & DUK_PROPDESC_FLAG_CONFIGURABLE) != 0)
#define DUK_PROPDESC_IS_ACCESSOR(p)             (((p)->flags & DUK_PROPDESC_FLAG_ACCESSOR) != 0)

#define DUK_HOBJECT_HASHIDX_UNUSED              0xffffffffUL
#define DUK_HOBJECT_HASHIDX_DELETED             0xfffffffeUL

/*
 *  Macros for accessing size fields
 */

#if defined(DUK_USE_OBJSIZES16)
#define DUK_HOBJECT_GET_ESIZE(h) ((h)->e_size16)
#define DUK_HOBJECT_SET_ESIZE(h,v) do { (h)->e_size16 = (v); } while (0)
#define DUK_HOBJECT_GET_ENEXT(h) ((h)->e_next16)
#define DUK_HOBJECT_SET_ENEXT(h,v) do { (h)->e_next16 = (v); } while (0)
#define DUK_HOBJECT_POSTINC_ENEXT(h) ((h)->e_next16++)
#define DUK_HOBJECT_GET_ASIZE(h) ((h)->a_size16)
#define DUK_HOBJECT_SET_ASIZE(h,v) do { (h)->a_size16 = (v); } while (0)
#if defined(DUK_USE_HOBJECT_HASH_PART)
#define DUK_HOBJECT_GET_HSIZE(h) ((h)->h_size16)
#define DUK_HOBJECT_SET_HSIZE(h,v) do { (h)->h_size16 = (v); } while (0)
#else
#define DUK_HOBJECT_GET_HSIZE(h) 0
#define DUK_HOBJECT_SET_HSIZE(h,v) do { DUK_ASSERT((v) == 0); } while (0)
#endif
#else
#define DUK_HOBJECT_GET_ESIZE(h) ((h)->e_size)
#define DUK_HOBJECT_SET_ESIZE(h,v) do { (h)->e_size = (v); } while (0)
#define DUK_HOBJECT_GET_ENEXT(h) ((h)->e_next)
#define DUK_HOBJECT_SET_ENEXT(h,v) do { (h)->e_next = (v); } while (0)
#define DUK_HOBJECT_POSTINC_ENEXT(h) ((h)->e_next++)
#define DUK_HOBJECT_GET_ASIZE(h) ((h)->a_size)
#define DUK_HOBJECT_SET_ASIZE(h,v) do { (h)->a_size = (v); } while (0)
#if defined(DUK_USE_HOBJECT_HASH_PART)
#define DUK_HOBJECT_GET_HSIZE(h) ((h)->h_size)
#define DUK_HOBJECT_SET_HSIZE(h,v) do { (h)->h_size = (v); } while (0)
#else
#define DUK_HOBJECT_GET_HSIZE(h) 0
#define DUK_HOBJECT_SET_HSIZE(h,v) do { DUK_ASSERT((v) == 0); } while (0)
#endif
#endif

/*
 *  Misc
 */

/* Maximum prototype traversal depth.  Sanity limit which handles e.g.
 * prototype loops (even complex ones like 1->2->3->4->2->3->4->2->3->4).
 */
#define DUK_HOBJECT_PROTOTYPE_CHAIN_SANITY      10000L

/* Maximum traversal depth for "bound function" chains. */
#define DUK_HOBJECT_BOUND_CHAIN_SANITY          10000L

/*
 *  Ecmascript [[Class]]
 */

/* range check not necessary because all 4-bit values are mapped */
#define DUK_HOBJECT_CLASS_NUMBER_TO_STRIDX(n)  duk_class_number_to_stridx[(n)]

#define DUK_HOBJECT_GET_CLASS_STRING(heap,h)          \
	DUK_HEAP_GET_STRING( \
		(heap), \
		DUK_HOBJECT_CLASS_NUMBER_TO_STRIDX(DUK_HOBJECT_GET_CLASS_NUMBER((h))) \
	)

/*
 *  Macros for property handling
 */

#if defined(DUK_USE_HEAPPTR16)
#define DUK_HOBJECT_GET_PROTOTYPE(heap,h) \
	((duk_hobject *) DUK_USE_HEAPPTR_DEC16((heap)->heap_udata, (h)->prototype16))
#define DUK_HOBJECT_SET_PROTOTYPE(heap,h,x) do { \
		(h)->prototype16 = DUK_USE_HEAPPTR_ENC16((heap)->heap_udata, (void *) (x)); \
	} while (0)
#else
#define DUK_HOBJECT_GET_PROTOTYPE(heap,h) \
	((h)->prototype)
#define DUK_HOBJECT_SET_PROTOTYPE(heap,h,x) do { \
		(h)->prototype = (x); \
	} while (0)
#endif

/* note: this updates refcounts */
#define DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr,h,p)       duk_hobject_set_prototype_updref((thr), (h), (p))

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
#define DUK_HOBJECT_MAX_PROPERTIES       0x0000ffffUL
#else
#define DUK_HOBJECT_MAX_PROPERTIES       0x7fffffffUL   /* 2**31-1 ~= 2G properties */
#endif

/* higher value conserves memory; also note that linear scan is cache friendly */
#define DUK_HOBJECT_E_USE_HASH_LIMIT     32

/* hash size relative to entries size: for value X, approx. hash_prime(e_size + e_size / X) */
#define DUK_HOBJECT_H_SIZE_DIVISOR       4  /* hash size approx. 1.25 times entries size */

/* if new_size < L * old_size, resize without abandon check; L = 3-bit fixed point, e.g. 9 -> 9/8 = 112.5% */
#define DUK_HOBJECT_A_FAST_RESIZE_LIMIT  9  /* 112.5%, i.e. new size less than 12.5% higher -> fast resize */

/* if density < L, abandon array part, L = 3-bit fixed point, e.g. 2 -> 2/8 = 25% */
/* limit is quite low: one array entry is 8 bytes, one normal entry is 4+1+8+4 = 17 bytes (with hash entry) */
#define DUK_HOBJECT_A_ABANDON_LIMIT      2  /* 25%, i.e. less than 25% used -> abandon */

/* internal align target for props allocation, must be 2*n for some n */
#if (DUK_USE_ALIGN_BY == 4)
#define DUK_HOBJECT_ALIGN_TARGET         4
#elif (DUK_USE_ALIGN_BY == 8)
#define DUK_HOBJECT_ALIGN_TARGET         8
#elif (DUK_USE_ALIGN_BY == 1)
#define DUK_HOBJECT_ALIGN_TARGET         1
#else
#error invalid DUK_USE_ALIGN_BY
#endif

/* controls for minimum entry part growth */
#define DUK_HOBJECT_E_MIN_GROW_ADD       16
#define DUK_HOBJECT_E_MIN_GROW_DIVISOR   8  /* 2^3 -> 1/8 = 12.5% min growth */

/* controls for minimum array part growth */
#define DUK_HOBJECT_A_MIN_GROW_ADD       16
#define DUK_HOBJECT_A_MIN_GROW_DIVISOR   8  /* 2^3 -> 1/8 = 12.5% min growth */

/* probe sequence */
#define DUK_HOBJECT_HASH_INITIAL(hash,h_size)  ((hash) % (h_size))
#define DUK_HOBJECT_HASH_PROBE_STEP(hash)      DUK_UTIL_GET_HASH_PROBE_STEP((hash))

/*
 *  PC-to-line constants
 */

#define DUK_PC2LINE_SKIP    64

/* maximum length for a SKIP-1 diffstream: 35 bits per entry, rounded up to bytes */
#define DUK_PC2LINE_MAX_DIFF_LENGTH    (((DUK_PC2LINE_SKIP - 1) * 35 + 7) / 8)

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

struct duk_propdesc {
	/* read-only values 'lifted' for ease of use */
	duk_small_int_t flags;
	duk_hobject *get;
	duk_hobject *set;

	/* for updating (all are set to < 0 for virtual properties) */
	duk_int_t e_idx;  /* prop index in 'entry part', < 0 if not there */
	duk_int_t h_idx;  /* prop index in 'hash part', < 0 if not there */
	duk_int_t a_idx;  /* prop index in 'array part', < 0 if not there */
};

struct duk_hobject {
	duk_heaphdr hdr;

	/*
	 *  'props' contains {key,value,flags} entries, optional array entries, and
	 *  an optional hash lookup table for non-array entries in a single 'sliced'
	 *  allocation.  There are several layout options, which differ slightly in
	 *  generated code size/speed and alignment/padding; duk_features.h selects
	 *  the layout used.
	 *
	 *  Layout 1 (DUK_USE_HOBJECT_LAYOUT_1):
	 *
	 *    e_size * sizeof(duk_hstring *)         bytes of   entry keys (e_next gc reachable)
	 *    e_size * sizeof(duk_propvalue)         bytes of   entry values (e_next gc reachable)
	 *    e_size * sizeof(duk_uint8_t)           bytes of   entry flags (e_next gc reachable)
	 *    a_size * sizeof(duk_tval)              bytes of   (opt) array values (plain only) (all gc reachable)
	 *    h_size * sizeof(duk_uint32_t)          bytes of   (opt) hash indexes to entries (e_size),
	 *                                                      0xffffffffUL = unused, 0xfffffffeUL = deleted
	 *
	 *  Layout 2 (DUK_USE_HOBJECT_LAYOUT_2):
	 *
	 *    e_size * sizeof(duk_propvalue)         bytes of   entry values (e_next gc reachable)
	 *    e_size * sizeof(duk_hstring *)         bytes of   entry keys (e_next gc reachable)
	 *    e_size * sizeof(duk_uint8_t) + pad     bytes of   entry flags (e_next gc reachable)
	 *    a_size * sizeof(duk_tval)              bytes of   (opt) array values (plain only) (all gc reachable)
	 *    h_size * sizeof(duk_uint32_t)          bytes of   (opt) hash indexes to entries (e_size),
	 *                                                      0xffffffffUL = unused, 0xfffffffeUL = deleted
	 *
	 *  Layout 3 (DUK_USE_HOBJECT_LAYOUT_3):
	 *
	 *    e_size * sizeof(duk_propvalue)         bytes of   entry values (e_next gc reachable)
	 *    a_size * sizeof(duk_tval)              bytes of   (opt) array values (plain only) (all gc reachable)
	 *    e_size * sizeof(duk_hstring *)         bytes of   entry keys (e_next gc reachable)
	 *    h_size * sizeof(duk_uint32_t)          bytes of   (opt) hash indexes to entries (e_size),
	 *                                                      0xffffffffUL = unused, 0xfffffffeUL = deleted
	 *    e_size * sizeof(duk_uint8_t)           bytes of   entry flags (e_next gc reachable)
	 *
	 *  In layout 1, the 'e_next' count is rounded to 4 or 8 on platforms
	 *  requiring 4 or 8 byte alignment.  This ensures proper alignment
	 *  for the entries, at the cost of memory footprint.  However, it's
	 *  probably preferable to use another layout on such platforms instead.
	 *
	 *  In layout 2, the key and value parts are swapped to avoid padding
	 *  the key array on platforms requiring alignment by 8.  The flags part
	 *  is padded to get alignment for array entries.  The 'e_next' count does
	 *  not need to be rounded as in layout 1.
	 *
	 *  In layout 3, entry values and array values are always aligned properly,
	 *  and assuming pointers are at most 8 bytes, so are the entry keys.  Hash
	 *  indices will be properly aligned (assuming pointers are at least 4 bytes).
	 *  Finally, flags don't need additional alignment.  This layout provides
	 *  compact allocations without padding (even on platforms with alignment
	 *  requirements) at the cost of a bit slower lookups.
	 *
	 *  Objects with few keys don't have a hash index; keys are looked up linearly,
	 *  which is cache efficient because the keys are consecutive.  Larger objects
	 *  have a hash index part which contains integer indexes to the entries part.
	 *
	 *  A single allocation reduces memory allocation overhead but requires more
	 *  work when any part needs to be resized.  A sliced allocation for entries
	 *  makes linear key matching faster on most platforms (more locality) and
	 *  skimps on flags size (which would be followed by 3 bytes of padding in
	 *  most architectures if entries were placed in a struct).
	 *
	 *  'props' also contains internal properties distinguished with a non-BMP
	 *  prefix.  Often used properties should be placed early in 'props' whenever
	 *  possible to make accessing them as fast a possible.
	 */

#if defined(DUK_USE_HEAPPTR16)
	/* Located in duk_heaphdr h_extra16.  Subclasses of duk_hobject (like
	 * duk_hcompfunc) are not free to use h_extra16 for this reason.
	 */
#else
	duk_uint8_t *props;
#endif

	/* prototype: the only internal property lifted outside 'e' as it is so central */
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t prototype16;
#else
	duk_hobject *prototype;
#endif

#if defined(DUK_USE_OBJSIZES16)
	duk_uint16_t e_size16;
	duk_uint16_t e_next16;
	duk_uint16_t a_size16;
#if defined(DUK_USE_HOBJECT_HASH_PART)
	duk_uint16_t h_size16;
#endif
#else
	duk_uint32_t e_size;  /* entry part size */
	duk_uint32_t e_next;  /* index for next new key ([0,e_next[ are gc reachable) */
	duk_uint32_t a_size;  /* array part size (entirely gc reachable) */
#if defined(DUK_USE_HOBJECT_HASH_PART)
	duk_uint32_t h_size;  /* hash part size or 0 if unused */
#endif
#endif
};

/*
 *  Exposed data
 */

#if !defined(DUK_SINGLE_FILE)
DUK_INTERNAL_DECL duk_uint8_t duk_class_number_to_stridx[32];
#endif  /* !DUK_SINGLE_FILE */

/*
 *  Prototypes
 */

/* alloc and init */
DUK_INTERNAL_DECL duk_hobject *duk_hobject_alloc(duk_heap *heap, duk_uint_t hobject_flags);
#if 0  /* unused */
DUK_INTERNAL_DECL duk_hobject *duk_hobject_alloc_checked(duk_hthread *thr, duk_uint_t hobject_flags);
#endif
DUK_INTERNAL_DECL duk_harray *duk_harray_alloc(duk_heap *heap, duk_uint_t hobject_flags);
DUK_INTERNAL_DECL duk_hcompfunc *duk_hcompfunc_alloc(duk_heap *heap, duk_uint_t hobject_flags);
DUK_INTERNAL_DECL duk_hnatfunc *duk_hnatfunc_alloc(duk_heap *heap, duk_uint_t hobject_flags);
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
DUK_INTERNAL_DECL duk_hbufobj *duk_hbufobj_alloc(duk_heap *heap, duk_uint_t hobject_flags);
#endif
DUK_INTERNAL_DECL duk_hthread *duk_hthread_alloc(duk_heap *heap, duk_uint_t hobject_flags);

/* resize */
DUK_INTERNAL_DECL void duk_hobject_realloc_props(duk_hthread *thr,
                                                 duk_hobject *obj,
                                                 duk_uint32_t new_e_size,
                                                 duk_uint32_t new_a_size,
                                                 duk_uint32_t new_h_size,
                                                 duk_bool_t abandon_array);

/* low-level property functions */
DUK_INTERNAL_DECL void duk_hobject_find_existing_entry(duk_heap *heap, duk_hobject *obj, duk_hstring *key, duk_int_t *e_idx, duk_int_t *h_idx);
DUK_INTERNAL_DECL duk_tval *duk_hobject_find_existing_entry_tval_ptr(duk_heap *heap, duk_hobject *obj, duk_hstring *key);
DUK_INTERNAL_DECL duk_tval *duk_hobject_find_existing_entry_tval_ptr_and_attrs(duk_heap *heap, duk_hobject *obj, duk_hstring *key, duk_int_t *out_attrs);
DUK_INTERNAL_DECL duk_tval *duk_hobject_find_existing_array_entry_tval_ptr(duk_heap *heap, duk_hobject *obj, duk_uarridx_t i);
DUK_INTERNAL_DECL duk_bool_t duk_hobject_get_own_propdesc(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_propdesc *out_desc, duk_small_uint_t flags);
DUK_INTERNAL_DECL duk_bool_t duk_hobject_get_propdesc(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_propdesc *out_desc, duk_small_uint_t flags);

/* XXX: when optimizing for guaranteed property slots, use a guaranteed
 * slot for internal value; this call can then access it directly.
 */
#define duk_hobject_get_internal_value_tval_ptr(heap,obj) \
	duk_hobject_find_existing_entry_tval_ptr((heap), (obj), DUK_HEAP_STRING_INT_VALUE((heap)))

/* core property functions */
DUK_INTERNAL_DECL duk_bool_t duk_hobject_getprop(duk_hthread *thr, duk_tval *tv_obj, duk_tval *tv_key);
DUK_INTERNAL_DECL duk_bool_t duk_hobject_putprop(duk_hthread *thr, duk_tval *tv_obj, duk_tval *tv_key, duk_tval *tv_val, duk_bool_t throw_flag);
DUK_INTERNAL_DECL duk_bool_t duk_hobject_delprop(duk_hthread *thr, duk_tval *tv_obj, duk_tval *tv_key, duk_bool_t throw_flag);
DUK_INTERNAL_DECL duk_bool_t duk_hobject_hasprop(duk_hthread *thr, duk_tval *tv_obj, duk_tval *tv_key);

/* internal property functions */
#define DUK_DELPROP_FLAG_THROW  (1 << 0)
#define DUK_DELPROP_FLAG_FORCE  (1 << 1)
DUK_INTERNAL_DECL duk_bool_t duk_hobject_delprop_raw(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_small_uint_t flags);
DUK_INTERNAL_DECL duk_bool_t duk_hobject_hasprop_raw(duk_hthread *thr, duk_hobject *obj, duk_hstring *key);
DUK_INTERNAL_DECL void duk_hobject_define_property_internal(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_small_uint_t flags);
DUK_INTERNAL_DECL void duk_hobject_define_property_internal_arridx(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t arr_idx, duk_small_uint_t flags);
DUK_INTERNAL_DECL void duk_hobject_define_accessor_internal(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_hobject *getter, duk_hobject *setter, duk_small_uint_t propflags);
DUK_INTERNAL_DECL void duk_hobject_set_length(duk_hthread *thr, duk_hobject *obj, duk_uint32_t length);  /* XXX: duk_uarridx_t? */
DUK_INTERNAL_DECL void duk_hobject_set_length_zero(duk_hthread *thr, duk_hobject *obj);
DUK_INTERNAL_DECL duk_uint32_t duk_hobject_get_length(duk_hthread *thr, duk_hobject *obj);  /* XXX: duk_uarridx_t? */

/* helpers for defineProperty() and defineProperties() */
DUK_INTERNAL_DECL
void duk_hobject_prepare_property_descriptor(duk_context *ctx,
                                             duk_idx_t idx_in,
                                             duk_uint_t *out_defprop_flags,
                                             duk_idx_t *out_idx_value,
                                             duk_hobject **out_getter,
                                             duk_hobject **out_setter);
DUK_INTERNAL_DECL
void duk_hobject_define_property_helper(duk_context *ctx,
                                        duk_uint_t defprop_flags,
                                        duk_hobject *obj,
                                        duk_hstring *key,
                                        duk_idx_t idx_value,
                                        duk_hobject *get,
                                        duk_hobject *set);

/* Object built-in methods */
DUK_INTERNAL_DECL duk_ret_t duk_hobject_object_get_own_property_descriptor(duk_context *ctx);
DUK_INTERNAL_DECL void duk_hobject_object_seal_freeze_helper(duk_hthread *thr, duk_hobject *obj, duk_bool_t is_freeze);
DUK_INTERNAL_DECL duk_bool_t duk_hobject_object_is_sealed_frozen_helper(duk_hthread *thr, duk_hobject *obj, duk_bool_t is_frozen);
DUK_INTERNAL_DECL duk_bool_t duk_hobject_object_ownprop_helper(duk_context *ctx, duk_small_uint_t required_desc_flags);

/* internal properties */
DUK_INTERNAL_DECL duk_bool_t duk_hobject_get_internal_value(duk_heap *heap, duk_hobject *obj, duk_tval *tv);
DUK_INTERNAL_DECL duk_hstring *duk_hobject_get_internal_value_string(duk_heap *heap, duk_hobject *obj);

/* hobject management functions */
DUK_INTERNAL_DECL void duk_hobject_compact_props(duk_hthread *thr, duk_hobject *obj);

/* ES6 proxy */
#if defined(DUK_USE_ES6_PROXY)
DUK_INTERNAL_DECL duk_bool_t duk_hobject_proxy_check(duk_hthread *thr, duk_hobject *obj, duk_hobject **out_target, duk_hobject **out_handler);
DUK_INTERNAL_DECL duk_hobject *duk_hobject_resolve_proxy_target(duk_hthread *thr, duk_hobject *obj);
#endif

/* enumeration */
DUK_INTERNAL_DECL void duk_hobject_enumerator_create(duk_context *ctx, duk_small_uint_t enum_flags);
DUK_INTERNAL_DECL duk_ret_t duk_hobject_get_enumerated_keys(duk_context *ctx, duk_small_uint_t enum_flags);
DUK_INTERNAL_DECL duk_bool_t duk_hobject_enumerator_next(duk_context *ctx, duk_bool_t get_value);

/* macros */
DUK_INTERNAL_DECL void duk_hobject_set_prototype_updref(duk_hthread *thr, duk_hobject *h, duk_hobject *p);

/* finalization */
#if defined(DUK_USE_FINALIZER_SUPPORT)
DUK_INTERNAL_DECL void duk_hobject_run_finalizer(duk_hthread *thr, duk_hobject *obj);
#endif

/* pc2line */
#if defined(DUK_USE_PC2LINE)
DUK_INTERNAL_DECL void duk_hobject_pc2line_pack(duk_hthread *thr, duk_compiler_instr *instrs, duk_uint_fast32_t length);
DUK_INTERNAL_DECL duk_uint_fast32_t duk_hobject_pc2line_query(duk_context *ctx, duk_idx_t idx_func, duk_uint_fast32_t pc);
#endif

/* misc */
DUK_INTERNAL_DECL duk_bool_t duk_hobject_prototype_chain_contains(duk_hthread *thr, duk_hobject *h, duk_hobject *p, duk_bool_t ignore_loop);

#endif  /* DUK_HOBJECT_H_INCLUDED */
