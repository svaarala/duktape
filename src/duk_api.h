/*
 *  Duktape public API.
 *
 *  Stack indexes are relative to the current frame (call stack entry).
 *  API calls can only access the current frame (between bottom and top
 *  of current frame).
 *              
 *     +-------+----+----+----+----+
 *     | . . . |  0 |  1 |  2 |  3 |   top = 4
 *     |       | -4 | -3 | -2 | -1 |
 *     +-------+----+----+----+----+
 *             :                   :
 *     bottom -'
 *     of frame
 *
 *  DUK_INVALID_INDEX (defined as the minimum integer) represents an
 *  invalid index / no index in calls where that is appropriate.  If
 *  used elsewhere, it never maps to a valid stack entry.
 *
 *  Any API definition may potentially be a macro, so callers should
 *  never assume otherwise, e.g. by using API calls as function pointers.
 */

#ifndef DUK_API_H_INCLUDED
#define DUK_API_H_INCLUDED

#include <limits.h>  /* INT_MIN */
#include <stdarg.h>  /* va_list etc */
#include <stdlib.h>
#include <stddef.h>

/*
 *  Typedefs; avoid all dependencies on internal types
 *
 *  (duk_context *) currently maps directly to (duk_hthread *).
 */

struct duk_memory_functions;

typedef void duk_context;
typedef struct duk_memory_functions duk_memory_functions;

typedef int (*duk_c_function)(duk_context *ctx);
typedef void *(*duk_alloc_function) (void *udata, size_t size);
typedef void *(*duk_realloc_function) (void *udata, void *ptr, size_t size);
typedef void (*duk_free_function) (void *udata, void *ptr);
typedef void (*duk_fatal_function) (duk_context *ctx, int code);
typedef void (*duk_decode_char_function) (void *udata, int codepoint);
typedef int (*duk_map_char_function) (void *udata, int codepoint);
typedef int (*duk_safe_call_function) (duk_context *ctx);

struct duk_memory_functions {
	duk_alloc_function alloc;
	duk_realloc_function realloc;
	duk_free_function free;
	void *udata;
};

/*
 *  Constants
 */

/* Used to represent invalid index; if caller uses this without checking,
 * this index will almost certainly map to a non-existent stack entry.
 */
#define  DUK_INVALID_INDEX     INT_MIN 

/* Indicates that a native function does not have a fixed number of args,
 * and the argument stack should not be capped/extended at all.
 */
#define  DUK_VARARGS           (-1)

/* Value types, used by e.g. duk_get_type() */
#define  DUK_TYPE_NONE         0    /* no value, e.g. invalid index */
#define  DUK_TYPE_UNDEFINED    1    /* Ecmascript undefined */
#define  DUK_TYPE_NULL         2    /* Ecmascript null */
#define  DUK_TYPE_BOOLEAN      3    /* Ecmascript boolean: 0 or 1 */
#define  DUK_TYPE_NUMBER       4    /* Ecmascript number: double */
#define  DUK_TYPE_STRING       5    /* Ecmascript string: CESU-8 / extended UTF-8 encoded */
#define  DUK_TYPE_OBJECT       6    /* Ecmascript object: includes objects, arrays, functions, threads */
#define  DUK_TYPE_BUFFER       7    /* fixed or dynamic, garbage collected byte buffer */
#define  DUK_TYPE_POINTER      8    /* raw void pointer */

/* Value mask types, used by e.g. duk_get_type_mask() */
#define  DUK_TYPE_MASK_NONE        (1 << DUK_TYPE_NONE)
#define  DUK_TYPE_MASK_UNDEFINED   (1 << DUK_TYPE_UNDEFINED)
#define  DUK_TYPE_MASK_NULL        (1 << DUK_TYPE_NULL)
#define  DUK_TYPE_MASK_BOOLEAN     (1 << DUK_TYPE_BOOLEAN)
#define  DUK_TYPE_MASK_NUMBER      (1 << DUK_TYPE_NUMBER)
#define  DUK_TYPE_MASK_STRING      (1 << DUK_TYPE_STRING)
#define  DUK_TYPE_MASK_OBJECT      (1 << DUK_TYPE_OBJECT)
#define  DUK_TYPE_MASK_BUFFER      (1 << DUK_TYPE_BUFFER)
#define  DUK_TYPE_MASK_POINTER     (1 << DUK_TYPE_POINTER)

/* FIXME: these are now used by duk_get_multiple() and duk_push_multiple(),
 * but the type chararacters are not very logical and don't cover all the
 * current types.
 */
#define  DUK_TYPECHAR_UNDEFINED  'u'
#define  DUK_TYPECHAR_NULL       'n'
#define  DUK_TYPECHAR_BOOLEAN    'b'
#define  DUK_TYPECHAR_NUMBER     'd'
#define  DUK_TYPECHAR_INTEGER    'i'
#define  DUK_TYPECHAR_STRING     's'
#define  DUK_TYPECHAR_LSTRING    'l'
/* FIXME: buffer */
#define  DUK_TYPECHAR_POINTER    'p'
#define  DUK_TYPECHAR_SKIP       '-'

/* Coercion hints */
#define  DUK_HINT_NONE         0    /* prefer number, unless coercion input is a Date, in which case prefer string (E5 Section 8.12.8) */
#define  DUK_HINT_STRING       1    /* prefer string */
#define  DUK_HINT_NUMBER       2    /* prefer number */

/* Enumeration flags for duk_enum() */
#define  DUK_ENUM_INCLUDE_NONENUMERABLE    (1 << 0)    /* enumerate non-numerable properties in addition to enumerable */
#define  DUK_ENUM_INCLUDE_INTERNAL         (1 << 1)    /* enumerate internal properties (regardless of enumerability) */
#define  DUK_ENUM_OWN_PROPERTIES_ONLY      (1 << 2)    /* don't walk prototype chain, only check own properties */
#define  DUK_ENUM_ARRAY_INDICES_ONLY       (1 << 3)    /* only enumerate array indices */
#define  DUK_ENUM_SORT_ARRAY_INDICES       (1 << 4)    /* sort array indices, use with DUK_ENUM_ARRAY_INDICES_ONLY */

/* Internal error codes */
#define  DUK_ERR_UNIMPLEMENTED_ERROR  50   /* UnimplementedError */
#define  DUK_ERR_UNSUPPORTED_ERROR    51   /* UnsupportedError */
#define  DUK_ERR_INTERNAL_ERROR       52   /* InternalError */
#define  DUK_ERR_ALLOC_ERROR          53   /* AllocError */
#define  DUK_ERR_ASSERTION_ERROR      54   /* AssertionError */
#define  DUK_ERR_API_ERROR            55   /* APIError */
#define  DUK_ERR_UNCAUGHT_ERROR       56   /* UncaughtError */

/* Ecmascript E5 specification error codes */
#define  DUK_ERR_ERROR                100  /* Error */
#define  DUK_ERR_EVAL_ERROR           101  /* EvalError */
#define  DUK_ERR_RANGE_ERROR          102  /* RangeError */
#define  DUK_ERR_REFERENCE_ERROR      103  /* ReferenceError */
#define  DUK_ERR_SYNTAX_ERROR         104  /* SyntaxError */
#define  DUK_ERR_TYPE_ERROR           105  /* TypeError */
#define  DUK_ERR_URI_ERROR            106  /* URIError */

/* Return codes for C functions */
#define  DUK_RET_UNIMPLEMENTED_ERROR  (-DUK_ERR_UNIMPLEMENTED_ERROR)
#define  DUK_RET_UNSUPPORTED_ERROR    (-DUK_ERR_UNSUPPORTED_ERROR)
#define  DUK_RET_INTERNAL_ERROR       (-DUK_ERR_INTERNAL_ERROR)
#define  DUK_RET_ALLOC_ERROR          (-DUK_ERR_ALLOC_ERROR)
#define  DUK_RET_ASSERTION_ERROR      (-DUK_ERR_ASSERTION_ERROR)
#define  DUK_RET_API_ERROR            (-DUK_ERR_API_ERROR)
#define  DUK_RET_UNCAUGHT_ERROR       (-DUK_ERR_UNCAUGHT_ERROR)
#define  DUK_RET_ERROR                (-DUK_ERR_ERROR)
#define  DUK_RET_EVAL_ERROR           (-DUK_ERR_EVAL_ERROR)
#define  DUK_RET_RANGE_ERROR          (-DUK_ERR_RANGE_ERROR)
#define  DUK_RET_REFERENCE_ERROR      (-DUK_ERR_REFERENCE_ERROR)
#define  DUK_RET_SYNTAX_ERROR         (-DUK_ERR_SYNTAX_ERROR)
#define  DUK_RET_TYPE_ERROR           (-DUK_ERR_TYPE_ERROR)
#define  DUK_RET_URI_ERROR            (-DUK_ERR_URI_ERROR)

/*
 *  Context management
 */

duk_context *duk_create_heap(duk_alloc_function alloc_func,
                             duk_realloc_function realloc_func,
                             duk_free_function free_func,
                             void *alloc_udata,
                             duk_fatal_function fatal_handler);
void duk_destroy_heap(duk_context *ctx);

/*
 *  Memory management
 *
 *  Raw functions have no side effects (cannot trigger GC).
 */

void *duk_alloc_raw(duk_context *ctx, size_t size);
void duk_free_raw(duk_context *ctx, void *ptr);
void *duk_realloc_raw(duk_context *ctx, void *ptr, size_t size);
void *duk_alloc(duk_context *ctx, size_t size);
void duk_free(duk_context *ctx, void *ptr);
void *duk_realloc(duk_context *ctx, void *ptr, size_t size);
void duk_get_memory_functions(duk_context *ctx, duk_memory_functions *out_funcs);

/*
 *  Error handling
 */

void duk_throw(duk_context *ctx);                                       /* throw value on top of stack */
void duk_error(duk_context *ctx, int err_code, const char *fmt, ...);   /* push error with msg and throw it */
void duk_fatal(duk_context *ctx, int err_code);                         /* call fatal error handler */

/*
 *  Other state related functions
 */

int duk_is_strict_call(duk_context *ctx);                               /* current activation is strict */
int duk_is_constructor_call(duk_context *ctx);                          /* current activation is a constructor call ('new Foo()') */

/*
 *  Stack management
 */

int duk_normalize_index(duk_context *ctx, int index);                   /* normalize index relative to bottom of current frame; return DUK_INVALID_INDEX (<0) if invalid index */
int duk_require_normalize_index(duk_context *ctx, int index);           /* normalize index, raise error if invalid */
int duk_is_valid_index(duk_context *ctx, int index);                    /* validate index, return non-zero if valid, zero if invalid */
void duk_require_valid_index(duk_context *ctx, int index);              /* validate index, raise error if invalid */

int duk_get_top(duk_context *ctx);                                      /* get stack top (>= 0) */
void duk_set_top(duk_context *ctx, int index);                          /* set stack top (negative values are normalized); pad with undefined or chop */
int duk_get_top_index(duk_context *ctx);                                /* get top index or DUK_INVALID_INDEX if stack is empty */
int duk_require_top_index(duk_context *ctx);                            /* same as duk_get_top_index() but throw error if
                                                                         * index is invalid.
                                                                         */

int duk_check_stack(duk_context *ctx, unsigned int extra);              /* check that stack has at least 'extra' free items,
                                                                         * growing (but not shrinking) if necessary (internal
                                                                         * extra is automatically added, so 'extra' should
                                                                         * reflect caller's need for stack space).
                                                                         * return zero on error, non-zero on success
                                                                         */
void duk_require_stack(duk_context *ctx, unsigned int extra);           /* same as duk_check_stack(), but raise error if
                                                                         * fails to grow.
                                                                         */

int duk_check_stack_top(duk_context *ctx, unsigned int top);            /* like duk_check_stack(), but ensures there is space in
                                                                         * the valstack up to 'top', plus internal extra.
                                                                         */
void duk_require_stack_top(duk_context *ctx, unsigned int top);         /* like duk_check_stack_top(), but raise error if
                                                                         * fails to grow.
                                                                         */

/*
 *  Stack manipulation (other than push/pop)
 */

void duk_swap(duk_context *ctx, int index1, int index2);                /* swap value at two indices */
void duk_swap_top(duk_context *ctx, int index);                         /* swap top and value at index */
void duk_dup(duk_context *ctx, int from_index);                         /* push a duplicate of value at index onto stack */
void duk_dup_top(duk_context *ctx);                                     /* push a duplicate of top onto stack */
void duk_insert(duk_context *ctx, int to_index);                        /* insert element at top to specified index (and pop top) */
void duk_replace(duk_context *ctx, int to_index);                       /* replace value at index with top (and pop top) */
void duk_remove(duk_context *ctx, int index);                           /* remove element at index (and shift) */
void duk_xmove(duk_context *from_ctx, duk_context *to_ctx, unsigned int count);  /* move 'count' value between tops of two contexts of the same heap */

/* XXX: slice operations */

/*
 *  Push operations
 *
 *  Push functions return the absolute (relative to bottom of frame)
 *  position of the pushed value for convenience.
 *
 *  Note: duk_dup() is technically a push.
 */

void duk_push_undefined(duk_context *ctx);
void duk_push_null(duk_context *ctx);
void duk_push_boolean(duk_context *ctx, int val);
void duk_push_true(duk_context *ctx);
void duk_push_false(duk_context *ctx);
void duk_push_number(duk_context *ctx, double val);
void duk_push_nan(duk_context *ctx);
void duk_push_int(duk_context *ctx, int val);
const char *duk_push_string(duk_context *ctx, const char *str);                     /* may fail; returns interned string ptr */
const char *duk_push_lstring(duk_context *ctx, const char *str, size_t len);        /* may fail; returns interned string ptr */
void duk_push_pointer(duk_context *ctx, void *p);
const char *duk_push_sprintf(duk_context *ctx, const char *fmt, ...);               /* may fail */
const char *duk_push_vsprintf(duk_context *ctx, const char *fmt, va_list ap);       /* may fail */
void duk_push_multiple(duk_context *ctx, const char *types, ...);                   /* push multiple values, see duk_get_multiple */

void duk_push_this(duk_context *ctx);                                               /* push the current 'this' binding */
void duk_push_current_function(duk_context *ctx);                                   /* push the currently running (C) function */
void duk_push_current_thread(duk_context *ctx);                                     /* push the currently running thread */
void duk_push_global_object(duk_context *ctx);                                      /* push the global object */

int duk_push_new_object(duk_context *ctx);                                          /* returns positive index of pushed object (may fail) */
int duk_push_new_array(duk_context *ctx);                                           /* returns positive index of pushed array (may fail) */
int duk_push_new_thread(duk_context *ctx);                                          /* returns positive index of pushed thread (may fail) */
int duk_push_new_c_function(duk_context *ctx, duk_c_function func, int nargs);      /* returns positive index of pushed object (may fail); nargs == DUK_VARARGS creates a variable args function */
int duk_push_new_error_object(duk_context *ctx, int err_code, const char *fmt, ...);  /* returns positive index of pushed error */
void *duk_push_new_buffer(duk_context *ctx, size_t size, int dynamic);              /* returns pointer to buffer (may be NULL if size is 0; may fail) */
void *duk_push_new_fixed_buffer(duk_context *ctx, size_t size);
void *duk_push_new_dynamic_buffer(duk_context *ctx, size_t size);

/*
 *  Pop operations
 */

void duk_pop(duk_context *ctx);    /* pop top */
void duk_pop_n(duk_context *ctx, unsigned int count);
void duk_pop_2(duk_context *ctx);  /* shortcut */
void duk_pop_3(duk_context *ctx);  /* shortcut */

/*
 *  Type checks
 *
 *  duk_is_none(), which would indicate whether index it outside of stack,
 *  is not needed; duk_is_valid_index() gives the same information.
 */

int duk_get_type(duk_context *ctx, int index);                   /* returns one of DUK_TYPE_xxx, DUK_TYPE_NONE for invalid index */
int duk_get_type_mask(duk_context *ctx, int index);              /* returns one of DUK_TYPE_MASK_xxx, DUK_TYPE_MASK_NONE for invalid index */

/* FIXME: duk_match_type_mask(duk_context *ctx, int index, int mask) ? */

int duk_is_undefined(duk_context *ctx, int index);
int duk_is_null(duk_context *ctx, int index);
int duk_is_null_or_undefined(duk_context *ctx, int index);       /* useful in Ecmascript, similar to 'x == null' which also matches undefined */
int duk_is_boolean(duk_context *ctx, int index);
int duk_is_number(duk_context *ctx, int index);
int duk_is_nan(duk_context *ctx, int index);
int duk_is_string(duk_context *ctx, int index);
int duk_is_object(duk_context *ctx, int index);
int duk_is_buffer(duk_context *ctx, int index);
int duk_is_pointer(duk_context *ctx, int index);

int duk_is_array(duk_context *ctx, int index);                   /* implies: duk_is_object=true */
int duk_is_function(duk_context *ctx, int index);                /* implies: duk_is_object=true */
int duk_is_c_function(duk_context *ctx, int index);              /* implies: duk_is_function=duk_is_object=true */
int duk_is_ecmascript_function(duk_context *ctx, int index);     /* implies: duk_is_function=duk_is_object=true */
int duk_is_bound_function(duk_context *ctx, int index);          /* implies: duk_is_function=duk_is_object=true */
int duk_is_thread(duk_context *ctx, int index);                  /* implies: duk_is_object=true */

int duk_is_callable(duk_context *ctx, int index);                /* currently same as duk_is_function() */
int duk_is_dynamic(duk_context *ctx, int index);                 /* for buffer */

int duk_is_primitive(duk_context *ctx, int index);               /* E5 Section 9.1; anything but object is primitive */
int duk_is_object_coercible(duk_context *ctx, int index);        /* E5 Section 9.10 */

/*
 *  Get operations: no coercion, returns default value for invalid
 *  indices and invalid value types.
 *
 *  duk_get_undefined() and duk_get_null() would be pointless and
 *  are not included.
 */

int duk_get_boolean(duk_context *ctx, int index);                        /* default: false */
double duk_get_number(duk_context *ctx, int index);                      /* default: NaN */
int duk_get_int(duk_context *ctx, int index);                            /* default: 0, truncated using E5 Section 9.4 */
const char *duk_get_string(duk_context *ctx, int index);                 /* default: NULL */
const char *duk_get_lstring(duk_context *ctx, int index, size_t *out_len); /* default: NULL, out_len may be NULL */
void *duk_get_buffer(duk_context *ctx, int index, size_t *out_size);     /* default: NULL, out_size may be NULL */
void *duk_get_pointer(duk_context *ctx, int index);                      /* default: NULL */
void duk_get_multiple(duk_context *ctx, int start_index, const char *types, ...);
duk_c_function duk_get_c_function(duk_context *ctx, int index);
duk_context *duk_get_context(duk_context *ctx, int index);
size_t duk_get_length(duk_context *ctx, int index);                      /* string: char length,
                                                                          * object: 'length' property,
                                                                          * buffer: byte length
                                                                          */

/*
 *  Require operations: no coercion, throw error if index or type
 *  is incorrect.  No defaulting.
 */

void duk_require_undefined(duk_context *ctx, int index);
void duk_require_null(duk_context *ctx, int index);
int duk_require_boolean(duk_context *ctx, int index);
double duk_require_number(duk_context *ctx, int index);
int duk_require_int(duk_context *ctx, int index);                               /* returns truncated value (e.g. 1.5 -> 1) */
const char *duk_require_string(duk_context *ctx, int index);
const char *duk_require_lstring(duk_context *ctx, int index, size_t *out_len);  /* out_len may be NULL */
void *duk_require_buffer(duk_context *ctx, int index, size_t *out_size);
void *duk_require_pointer(duk_context *ctx, int index);
duk_c_function duk_require_c_function(duk_context *ctx, int index);
duk_context *duk_require_context(duk_context *ctx, int index);

/*
 *  Coercion operations: in-place coercion, return coerced value where
 *  applicable.  If index is invalid, throw error.  Some coercions may
 *  throw an expected error (e.g. from a toString() or valueOf() call)
 *  or an internal error (e.g. from out of memory).
 */

void duk_to_undefined(duk_context *ctx, int index);                      /* replace value with undefined */
void duk_to_null(duk_context *ctx, int index);                           /* replace value with null */
int duk_to_boolean(duk_context *ctx, int index);                         /* Ecmascript ToBoolean coercion, E5 Section 9.2 */
double duk_to_number(duk_context *ctx, int index);                       /* Ecmascript ToNumber coercion, E5 Section 9.3 */
int duk_to_int(duk_context *ctx, int index);                             /* Ecmascript ToInteger coercion, E5 Section 9.4 */
int duk_to_int32(duk_context *ctx, int index);                           /* Ecmascript ToInt32 coercion, E5 Section 9.5 */
unsigned int duk_to_uint32(duk_context *ctx, int index);                 /* Ecmascript ToUint32 coercion, E5 Section 9.6 */
unsigned int duk_to_uint16(duk_context *ctx, int index);                 /* Ecmascript ToUint16 coercion, E5 Section 9.7 */
const char *duk_to_string(duk_context *ctx, int index);                  /* Ecmascript ToString coercion, E5 Section 9.8 */
const char *duk_to_lstring(duk_context *ctx, int index, size_t *out_len);  /* out_len may be NULL */
void *duk_to_buffer(duk_context *ctx, int index, size_t *out_size);      /* coerces: buffer to itself, string to buffer, others: use ToString, then to buffer */
void *duk_to_pointer(duk_context *ctx, int index);                       /* coerces: pointer to itself, heap object to heap pointer (debug only), others to NULL */
void duk_to_object(duk_context *ctx, int index);                         /* Ecmascript ToObject coercion, E5 Section 9.9 (returns nothing) */
void duk_to_defaultvalue(duk_context *ctx, int index, int hint);         /* Ecmascript E5 Section 8.12.18, with optional hint */
void duk_to_primitive(duk_context *ctx, int index, int hint);            /* Ecmascript E5 Section 9.1, with optional hint */

/*
 *  Misc conversion
 */

void duk_base64_encode(duk_context *ctx, int index);
void duk_base64_decode(duk_context *ctx, int index);
void duk_hex_decode(duk_context *ctx, int index);
void duk_hex_encode(duk_context *ctx, int index);

/* XXX: duk_to_json in place, return const char *, would be
 * nice for printing.
 */
const char *duk_json_encode(duk_context *ctx, int index);
void duk_json_decode(duk_context *ctx, int index);

/*
 *  Buffer
 */

void *duk_resize_buffer(duk_context *ctx, int index, size_t new_size);              /* may fail (throw error) */
void duk_to_fixed_buffer(duk_context *ctx, int index);

/*
 *  Property access
 *
 *  The basic function assumes key is on stack.  The _string variant takes
 *  a C string as a property name, while the _index variant takes an array
 *  index as a property name (e.g. 123 is equivalent to the key "123").
 */

int duk_get_prop(duk_context *ctx, int obj_index);                                  /* [key] -> [val] */
int duk_get_prop_string(duk_context *ctx, int obj_index, const char *key);          /* [] -> [val] */
int duk_get_prop_index(duk_context *ctx, int obj_index, unsigned int arr_index);    /* [] -> [val] */
int duk_put_prop(duk_context *ctx, int obj_index);                                  /* [key val] -> [] */
int duk_put_prop_string(duk_context *ctx, int obj_index, const char *key);          /* [val] -> [] */
int duk_put_prop_index(duk_context *ctx, int obj_index, unsigned int arr_index);    /* [val] -> [] */
int duk_del_prop(duk_context *ctx, int obj_index);                                  /* [key] -> [] */
int duk_del_prop_string(duk_context *ctx, int obj_index, const char *key);          /* [] -> [] */
int duk_del_prop_index(duk_context *ctx, int obj_index, unsigned int arr_index);    /* [] -> [] */
int duk_has_prop(duk_context *ctx, int obj_index);                                  /* [key] -> [] */
int duk_has_prop_string(duk_context *ctx, int obj_index, const char *key);          /* [] -> [] */
int duk_has_prop_index(duk_context *ctx, int obj_index, unsigned int arr_index);    /* [] -> [] */

/*
 *  Variable access
 */

int duk_get_var(duk_context *ctx);                                 /* [varname] -> [val] */
int duk_put_var(duk_context *ctx);                                 /* [varname val] -> [res] */
int duk_del_var(duk_context *ctx);                                 /* [varname] -> [res] */
int duk_has_var(duk_context *ctx);                                 /* [varname] */

/*
 *  Object operations
 */

void duk_compact(duk_context *ctx, int obj_index);                 /* [] -> [] */
void duk_enum(duk_context *ctx, int obj_index, int enum_flags);    /* [] -> [enum] */
int duk_next(duk_context *ctx, int enum_index, int get_value);     /* if enum empty, [] -> [] and return 0.
                                                                    * else return non-zero and:
                                                                    *       [] -> [key]     (if get_value == 0)
                                                                    *       [] -> [key val] (if get_value != 0);
                                                                    * Note that getting the value may cause an error
                                                                    * (e.g. in case of a getter).
                                                                    */

/*
 *  String manipulation
 */

/* FIXME: decode and map string should probably have a slice syntax */
/* FIXME: char_code_at */

void duk_concat(duk_context *ctx, unsigned int count);              /* [val1 ... valN] -> [res], coerced and concatenated */
void duk_join(duk_context *ctx, unsigned int count);                /* [sep val1 ... valN] -> [res], coerced and joined */
void duk_decode_string(duk_context *ctx, int index, duk_decode_char_function callback, void *udata);
void duk_map_string(duk_context *ctx, int index, duk_map_char_function callback, void *udata);
void duk_substring(duk_context *ctx, size_t start_offset, size_t end_offset);
void duk_trim(duk_context *ctx, int index);                         /* trim using StrWhiteSpaceChar: WhiteSpace + LineTerminator,
                                                                     * matches String.prototype.trim(), global object parseInt()
                                                                     * and parseFloat().
                                                                     */

/*
 *  Ecmascript operators
 */

/* FIXME: what to include; there are plenty of operators, arithmetic, comparison, bit ops, etc */

int duk_equals(duk_context *ctx, int index1, int index2);           /* true if equals with Ecmascript '==' semantics; false if either index invalid */
int duk_strict_equals(duk_context *ctx, int index1, int index2);    /* true if equals with Ecmascript '===' semantics; false if either index invalid */

#if 0  /* FIXME: to be decided */
int duk_less_than(duk_context *ctx, int index1, int index2);        /* 'x < y' comparison, E5 Section 11.8.5 */
int duk_same_value(duk_context *ctx, int index1, int index2);       /* true with Ecmascript SameValue, E5 Section 9.12 (stricter than '===') */
#endif

/*
 *  Function (method) calls
 *
 *  If 'errhandler_index' is DUK_INVALID_INDEX, the current errhandler will be
 *  used.  If 'errhandler_index' points to an undefined value in the stack,
 *  a NULL errhandler will be used, replacing any existing errhandler.
 */

void duk_call(duk_context *ctx, int nargs);                                /* [func arg1 ... argN] -> [retval] */
void duk_call_method(duk_context *ctx, int nargs);                         /* [func this arg1 ... argN] -> [retval] */
void duk_call_prop(duk_context *ctx, int obj_index, int nargs);            /* [key arg1 ... argN] -> [retval] ('this' bound to obj) */
int duk_pcall(duk_context *ctx, int nargs, int errhandler_index);          /* [func arg1 ... argN] -> [retval] or [err] */
int duk_pcall_method(duk_context *ctx, int nargs, int errhandler_index);   /* [func this arg1 ... argN] -> [retval] or [err] */
int duk_pcall_prop(duk_context *ctx, int obj_index, int nargs, int errhandler_index);  /* [key arg1 ... argN] -> [retval] or [err] ('this' bound to obj) */
void duk_new(duk_context *ctx, int nargs);                                 /* [... constructor arg1 ... argN] -> [retval] */

/*
 *  Protected pure C function call.  Operates on the existing value stack
 *  frame and is not visible on the call stack.  This call is useful as it
 *  provides a place to catch errors, perform cleanup, and perhaps rethrow.
 *
 *  Because this call operates on the current value stack frame, stack
 *  behavior differs a bit from other call types.
 *
 *  The top 'nargs' elements of the stack top are identified as arguments to
 *  'func' establishing a "base index" for the return stack as:
 *
 *    (duk_get_top() - nargs)
 *
 *  When 'func' returns, it indicates with its return value the number of
 *  return values it has pushed on top of the stack; multiple or zero return
 *  values possible.  The stack is then manipulated so that there are exactly
 *  'nrets' values starting at the "base index" established before the call.
 * 
 *  Note that since 'func' has full access to the value stack, it may modify
 *  the stack below the indended arguments and even pop elements below the
 *  "base index" off the stack.  Such elements are restored with 'undefined'
 *  values before returning, to ensure that the stack is always in a
 *  consistent state upon returning.
 * 
 *  If an error occurs, the stack will still have 'nrets' values at "base
 *  index"; the first of such values is the error, and the remaining values
 *  are undefined.  If 'nrets' is zero, the error will not be present on the
 *  stack (the return stack top will equal the "base index"), so calling this
 *  function with nrets = 0 may not be very useful.
 * 
 *  Example: with nargs = 3, nrets = 2, 'func' rc = 4, pipe chars indicate
 *  logical boundaries:
 *
 *          .--- frame bottom
 *          v
 *    [ ... | ... | a b c ]            stack before calling 'func'
 * 
 *    [ ... | ... | a b | x y z w ]    stack after calling 'func', which has
 *                                     popped one argument and written four
 *                                     return values
 * 
 *    [ ... | ... | x y ]              stack after duk_safe_call() returns,
 *                                     2 (= nrets) first 'func' return values
 *                                     are left at "base index"
 * 
 *  Note: 'func' uses caller stack frame, so bottom-based references are
 *  dangerous within 'func'.
 */

int duk_safe_call(duk_context *ctx, duk_safe_call_function func, int nargs, int nrets, int errhandler_index);

/*
 *  Thread management
 */

/* There are currently no native functions to yield/resume, due to the internal
 * limitations on coroutine handling.  These will be added later.
 */

/*
 *  Compilation and evaluation
 */

/* FIXME: helper for protected eval, duk_peval()? */
/* FIXME: global program -> through compile flags? */

void duk_eval(duk_context *ctx, int flags);                /* [code] -> [retval] (may throw error) */
void duk_compile(duk_context *ctx, int flags);             /* [code] -> [func] (may throw error) */

#endif  /* DUK_API_H_INCLUDED */

