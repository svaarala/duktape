/*
 *  Duktape public API.
 *
 *  See the API reference for documentation on call semantics.
 */

#ifndef DUKTAPE_H_INCLUDED
#define DUKTAPE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  Feature detection needed by this public header
 *
 *  DUK_API_NORETURN: macro for declaring a 'noreturn' function.
 *  Unfortunately the noreturn declaration may appear in various
 *  places of a function declaration, so the solution is to wrap
 *  the entire declaration inside the macro.
 *
 *  http://gcc.gnu.org/onlinedocs/gcc-4.3.2//gcc/Function-Attributes.html
 *  http://clang.llvm.org/docs/LanguageExtensions.html
 */

#if defined(__GNUC__)
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
/* Convenience, e.g. gcc 4.5.1 == 40501; http://stackoverflow.com/questions/6031819/emulating-gccs-builtin-unreachable */
#define DUK_API_GCC_VERSION  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#error cannot figure out gcc version
#endif
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define DUK_API_VARIADIC_MACROS
#else
#undef DUK_API_VARIADIC_MACROS
#endif

#if defined(DUK_API_GCC_VERSION) && (DUK_API_GCC_VERSION >= 20500)
/* since gcc-2.5 */
#define DUK_API_NORETURN(decl)  decl __attribute__((noreturn))
#elif defined(__clang__)
/* syntax same as gcc */
#define DUK_API_NORETURN(decl)  decl __attribute__((noreturn))
#elif defined(_MSC_VER)
/* http://msdn.microsoft.com/en-us/library/aa235362(VS.60).aspx */
#define DUK_API_NORETURN(decl)  __declspec(noreturn) decl
#else
/* Don't know how to declare a noreturn function, so don't do it; this
 * may cause some spurious compilation warnings (e.g. "variable used
 * uninitialized").
 */
#define DUK_API_NORETURN(decl)  decl
#endif

/*
 *  Includes
 */

#include <limits.h>  /* INT_MIN */
#include <stdarg.h>  /* va_list etc */
#include <stdlib.h>
#include <stddef.h>

/*
 *  Typedefs; avoid all dependencies on internal types
 *
 *  (duk_context *) currently maps directly to internal type (duk_hthread *).
 *  Internal typedefs have a '_t' suffix, exposed typedefs don't.  This is to
 *  reduce clutter in user code.
 */

/* FIXME: proper detection */
typedef int duk_idx;
typedef int duk_ret;

struct duk_memory_functions;

typedef void duk_context;
typedef struct duk_memory_functions duk_memory_functions;

typedef duk_ret (*duk_c_function)(duk_context *ctx);
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

/* Duktape version, (major * 10000) + (minor * 100) + patch.  Allows C code
 * to #ifdef against Duktape API version.  The same value is also available
 * to Ecmascript code in Duktape.version.
 */
#define DUK_VERSION                       900

/* Used to represent invalid index; if caller uses this without checking,
 * this index will map to a non-existent stack entry.  Also used in some
 * API calls as a marker to denote "no value".
 */
#define DUK_INVALID_INDEX                 INT_MIN 

/* Indicates that a native function does not have a fixed number of args,
 * and the argument stack should not be capped/extended at all.
 */
#define DUK_VARARGS                       (-1)

/* Value types, used by e.g. duk_get_type() */
#define DUK_TYPE_NONE                     0    /* no value, e.g. invalid index */
#define DUK_TYPE_UNDEFINED                1    /* Ecmascript undefined */
#define DUK_TYPE_NULL                     2    /* Ecmascript null */
#define DUK_TYPE_BOOLEAN                  3    /* Ecmascript boolean: 0 or 1 */
#define DUK_TYPE_NUMBER                   4    /* Ecmascript number: double */
#define DUK_TYPE_STRING                   5    /* Ecmascript string: CESU-8 / extended UTF-8 encoded */
#define DUK_TYPE_OBJECT                   6    /* Ecmascript object: includes objects, arrays, functions, threads */
#define DUK_TYPE_BUFFER                   7    /* fixed or dynamic, garbage collected byte buffer */
#define DUK_TYPE_POINTER                  8    /* raw void pointer */

/* Value mask types, used by e.g. duk_get_type_mask() */
#define DUK_TYPE_MASK_NONE                (1 << DUK_TYPE_NONE)
#define DUK_TYPE_MASK_UNDEFINED           (1 << DUK_TYPE_UNDEFINED)
#define DUK_TYPE_MASK_NULL                (1 << DUK_TYPE_NULL)
#define DUK_TYPE_MASK_BOOLEAN             (1 << DUK_TYPE_BOOLEAN)
#define DUK_TYPE_MASK_NUMBER              (1 << DUK_TYPE_NUMBER)
#define DUK_TYPE_MASK_STRING              (1 << DUK_TYPE_STRING)
#define DUK_TYPE_MASK_OBJECT              (1 << DUK_TYPE_OBJECT)
#define DUK_TYPE_MASK_BUFFER              (1 << DUK_TYPE_BUFFER)
#define DUK_TYPE_MASK_POINTER             (1 << DUK_TYPE_POINTER)

/* Coercion hints */
#define DUK_HINT_NONE                     0    /* prefer number, unless input is a Date, in which
                                                * case prefer string (E5 Section 8.12.8)
                                                */
#define DUK_HINT_STRING                   1    /* prefer string */
#define DUK_HINT_NUMBER                   2    /* prefer number */

/* Enumeration flags for duk_enum() */
#define DUK_ENUM_INCLUDE_NONENUMERABLE    (1 << 0)    /* enumerate non-numerable properties in addition to enumerable */
#define DUK_ENUM_INCLUDE_INTERNAL         (1 << 1)    /* enumerate internal properties (regardless of enumerability) */
#define DUK_ENUM_OWN_PROPERTIES_ONLY      (1 << 2)    /* don't walk prototype chain, only check own properties */
#define DUK_ENUM_ARRAY_INDICES_ONLY       (1 << 3)    /* only enumerate array indices */
#define DUK_ENUM_SORT_ARRAY_INDICES       (1 << 4)    /* sort array indices, use with DUK_ENUM_ARRAY_INDICES_ONLY */

/* Compilation flags for duk_compile() */
#define DUK_COMPILE_EVAL                  (1 << 0)    /* compile eval code (instead of program) */
#define DUK_COMPILE_FUNCTION              (1 << 1)    /* compile function code (instead of program) */
#define DUK_COMPILE_STRICT                (1 << 2)    /* use strict (outer) context for program, eval, or function */

/* Duktape specific error codes */
#define DUK_ERR_UNIMPLEMENTED_ERROR       50   /* UnimplementedError */
#define DUK_ERR_UNSUPPORTED_ERROR         51   /* UnsupportedError */
#define DUK_ERR_INTERNAL_ERROR            52   /* InternalError */
#define DUK_ERR_ALLOC_ERROR               53   /* AllocError */
#define DUK_ERR_ASSERTION_ERROR           54   /* AssertionError */
#define DUK_ERR_API_ERROR                 55   /* APIError */
#define DUK_ERR_UNCAUGHT_ERROR            56   /* UncaughtError */

/* Ecmascript E5 specification error codes */
#define DUK_ERR_ERROR                     100  /* Error */
#define DUK_ERR_EVAL_ERROR                101  /* EvalError */
#define DUK_ERR_RANGE_ERROR               102  /* RangeError */
#define DUK_ERR_REFERENCE_ERROR           103  /* ReferenceError */
#define DUK_ERR_SYNTAX_ERROR              104  /* SyntaxError */
#define DUK_ERR_TYPE_ERROR                105  /* TypeError */
#define DUK_ERR_URI_ERROR                 106  /* URIError */

/* Return codes for C functions (shortcut for throwing an error) */
#define DUK_RET_UNIMPLEMENTED_ERROR       (-DUK_ERR_UNIMPLEMENTED_ERROR)
#define DUK_RET_UNSUPPORTED_ERROR         (-DUK_ERR_UNSUPPORTED_ERROR)
#define DUK_RET_INTERNAL_ERROR            (-DUK_ERR_INTERNAL_ERROR)
#define DUK_RET_ALLOC_ERROR               (-DUK_ERR_ALLOC_ERROR)
#define DUK_RET_ASSERTION_ERROR           (-DUK_ERR_ASSERTION_ERROR)
#define DUK_RET_API_ERROR                 (-DUK_ERR_API_ERROR)
#define DUK_RET_UNCAUGHT_ERROR            (-DUK_ERR_UNCAUGHT_ERROR)
#define DUK_RET_ERROR                     (-DUK_ERR_ERROR)
#define DUK_RET_EVAL_ERROR                (-DUK_ERR_EVAL_ERROR)
#define DUK_RET_RANGE_ERROR               (-DUK_ERR_RANGE_ERROR)
#define DUK_RET_REFERENCE_ERROR           (-DUK_ERR_REFERENCE_ERROR)
#define DUK_RET_SYNTAX_ERROR              (-DUK_ERR_SYNTAX_ERROR)
#define DUK_RET_TYPE_ERROR                (-DUK_ERR_TYPE_ERROR)
#define DUK_RET_URI_ERROR                 (-DUK_ERR_URI_ERROR)

/* Return codes for protected calls (duk_safe_call(), duk_pcall()). */
#define DUK_EXEC_SUCCESS                  0
#define DUK_EXEC_ERROR                    1
/* FIXME: these codes will be refined later (separate code for a fatal API error,
 * distinct from normal error).  These must now match internal DUK_ERR_EXEC_xxx
 * defines.  The internal codes should be removed.
 */

/*
 *  If no variadic macros, __FILE__ and __LINE__ are passed through globals
 *  which is ugly and not thread safe.
 */

#ifndef DUK_API_VARIADIC_MACROS
extern const char *duk_api_global_filename;
extern int duk_api_global_line;
#endif

/*
 *  Context management
 */

duk_context *duk_create_heap(duk_alloc_function alloc_func,
                             duk_realloc_function realloc_func,
                             duk_free_function free_func,
                             void *alloc_udata,
                             duk_fatal_function fatal_handler);
void duk_destroy_heap(duk_context *ctx);

#define duk_create_heap_default()  (duk_create_heap(NULL, NULL, NULL, NULL, NULL))

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
void duk_gc(duk_context *ctx, int flags);

/*
 *  Error handling
 */

DUK_API_NORETURN(void duk_throw(duk_context *ctx));

DUK_API_NORETURN(void duk_error_raw(duk_context *ctx, int err_code, const char *filename, int line, const char *fmt, ...));
#ifdef DUK_API_VARIADIC_MACROS
#define duk_error(ctx,err_code,...)  \
	duk_error_raw((ctx),(err_code),__FILE__,__LINE__,__VA_ARGS__)
#else
DUK_API_NORETURN(void duk_error_stash(duk_context *ctx, int err_code, const char *fmt, ...));
#define duk_error  \
	duk_api_global_filename = __FILE__, \
	duk_api_global_line = __LINE__, \
	duk_error_stash  /* arguments follow */
#endif

DUK_API_NORETURN(void duk_fatal(duk_context *ctx, int err_code));

/*
 *  Other state related functions
 */

int duk_is_strict_call(duk_context *ctx);
int duk_is_constructor_call(duk_context *ctx);
int duk_get_magic(duk_context *ctx);

/*
 *  Stack management
 */

int duk_normalize_index(duk_context *ctx, int index);
int duk_require_normalize_index(duk_context *ctx, int index);
int duk_is_valid_index(duk_context *ctx, int index);
void duk_require_valid_index(duk_context *ctx, int index);

int duk_get_top(duk_context *ctx);
void duk_set_top(duk_context *ctx, int index);
int duk_get_top_index(duk_context *ctx);
int duk_require_top_index(duk_context *ctx);

int duk_check_stack(duk_context *ctx, unsigned int extra);
void duk_require_stack(duk_context *ctx, unsigned int extra);
int duk_check_stack_top(duk_context *ctx, unsigned int top);
void duk_require_stack_top(duk_context *ctx, unsigned int top);

/*
 *  Stack manipulation (other than push/pop)
 */

void duk_swap(duk_context *ctx, int index1, int index2);
void duk_swap_top(duk_context *ctx, int index);
void duk_dup(duk_context *ctx, int from_index);
void duk_dup_top(duk_context *ctx);
void duk_insert(duk_context *ctx, int to_index);
void duk_replace(duk_context *ctx, int to_index);
void duk_remove(duk_context *ctx, int index);
void duk_xmove(duk_context *from_ctx, duk_context *to_ctx, unsigned int count);  /* FIXME: undocumented */

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
const char *duk_push_string(duk_context *ctx, const char *str);
const char *duk_push_string_file(duk_context *ctx, const char *path);
const char *duk_push_lstring(duk_context *ctx, const char *str, size_t len);
void duk_push_pointer(duk_context *ctx, void *p);
const char *duk_push_sprintf(duk_context *ctx, const char *fmt, ...);
const char *duk_push_vsprintf(duk_context *ctx, const char *fmt, va_list ap);

void duk_push_this(duk_context *ctx);
void duk_push_current_function(duk_context *ctx);
void duk_push_current_thread(duk_context *ctx);
void duk_push_global_object(duk_context *ctx);

int duk_push_object(duk_context *ctx);
int duk_push_array(duk_context *ctx);
int duk_push_thread(duk_context *ctx);
int duk_push_c_function(duk_context *ctx, duk_c_function func, int nargs);

int duk_push_error_object_raw(duk_context *ctx, int err_code, const char *filename, int line, const char *fmt, ...);
#ifdef DUK_API_VARIADIC_MACROS
#define duk_push_error_object(ctx,err_code,...)  \
	duk_push_error_object_raw((ctx),(err_code),__FILE__,__LINE__,__VA_ARGS__)
#else
int duk_push_error_object_stash(duk_context *ctx, int err_code, const char *fmt, ...);
#define duk_push_error_object  \
	duk_api_global_filename = __FILE__, \
	duk_api_global_line = __LINE__, \
	duk_push_error_object_stash  /* arguments follow */
#endif

void *duk_push_buffer(duk_context *ctx, size_t size, int dynamic);
void *duk_push_fixed_buffer(duk_context *ctx, size_t size);
void *duk_push_dynamic_buffer(duk_context *ctx, size_t size);

/*
 *  Pop operations
 */

void duk_pop(duk_context *ctx);
void duk_pop_n(duk_context *ctx, unsigned int count);
void duk_pop_2(duk_context *ctx);
void duk_pop_3(duk_context *ctx);

/*
 *  Type checks
 *
 *  duk_is_none(), which would indicate whether index it outside of stack,
 *  is not needed; duk_is_valid_index() gives the same information.
 */

int duk_get_type(duk_context *ctx, int index);
int duk_check_type(duk_context *ctx, int index, int type);
int duk_get_type_mask(duk_context *ctx, int index);
int duk_check_type_mask(duk_context *ctx, int index, int mask);

int duk_is_undefined(duk_context *ctx, int index);
int duk_is_null(duk_context *ctx, int index);
int duk_is_null_or_undefined(duk_context *ctx, int index);
int duk_is_boolean(duk_context *ctx, int index);
int duk_is_number(duk_context *ctx, int index);
int duk_is_nan(duk_context *ctx, int index);
int duk_is_string(duk_context *ctx, int index);
int duk_is_object(duk_context *ctx, int index);
int duk_is_buffer(duk_context *ctx, int index);
int duk_is_pointer(duk_context *ctx, int index);

int duk_is_array(duk_context *ctx, int index);
int duk_is_function(duk_context *ctx, int index);
int duk_is_c_function(duk_context *ctx, int index);
int duk_is_ecmascript_function(duk_context *ctx, int index);
int duk_is_bound_function(duk_context *ctx, int index);
int duk_is_thread(duk_context *ctx, int index);

int duk_is_callable(duk_context *ctx, int index);
int duk_is_dynamic(duk_context *ctx, int index);
int duk_is_fixed(duk_context *ctx, int index);

int duk_is_primitive(duk_context *ctx, int index);
int duk_is_object_coercible(duk_context *ctx, int index);

/*
 *  Get operations: no coercion, returns default value for invalid
 *  indices and invalid value types.
 *
 *  duk_get_undefined() and duk_get_null() would be pointless and
 *  are not included.
 */

int duk_get_boolean(duk_context *ctx, int index);
double duk_get_number(duk_context *ctx, int index);
int duk_get_int(duk_context *ctx, int index);
const char *duk_get_string(duk_context *ctx, int index);
const char *duk_get_lstring(duk_context *ctx, int index, size_t *out_len);
void *duk_get_buffer(duk_context *ctx, int index, size_t *out_size);
void *duk_get_pointer(duk_context *ctx, int index);
duk_c_function duk_get_c_function(duk_context *ctx, int index);
duk_context *duk_get_context(duk_context *ctx, int index);
size_t duk_get_length(duk_context *ctx, int index);

/*
 *  Require operations: no coercion, throw error if index or type
 *  is incorrect.  No defaulting.
 */

void duk_require_undefined(duk_context *ctx, int index);
void duk_require_null(duk_context *ctx, int index);
int duk_require_boolean(duk_context *ctx, int index);
double duk_require_number(duk_context *ctx, int index);
int duk_require_int(duk_context *ctx, int index);
const char *duk_require_string(duk_context *ctx, int index);
const char *duk_require_lstring(duk_context *ctx, int index, size_t *out_len);
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

void duk_to_undefined(duk_context *ctx, int index);
void duk_to_null(duk_context *ctx, int index);
int duk_to_boolean(duk_context *ctx, int index);
double duk_to_number(duk_context *ctx, int index);
int duk_to_int(duk_context *ctx, int index);
int duk_to_int32(duk_context *ctx, int index);
unsigned int duk_to_uint32(duk_context *ctx, int index);
unsigned int duk_to_uint16(duk_context *ctx, int index);
const char *duk_to_string(duk_context *ctx, int index);
const char *duk_to_lstring(duk_context *ctx, int index, size_t *out_len);
void *duk_to_buffer(duk_context *ctx, int index, size_t *out_size);
void *duk_to_pointer(duk_context *ctx, int index);
void duk_to_object(duk_context *ctx, int index);
void duk_to_defaultvalue(duk_context *ctx, int index, int hint);
void duk_to_primitive(duk_context *ctx, int index, int hint);

/*
 *  Misc conversion
 */

const char *duk_base64_encode(duk_context *ctx, int index);
void duk_base64_decode(duk_context *ctx, int index);
const char *duk_hex_encode(duk_context *ctx, int index);
void duk_hex_decode(duk_context *ctx, int index);
const char *duk_json_encode(duk_context *ctx, int index);
void duk_json_decode(duk_context *ctx, int index);

/*
 *  Buffer
 */

void *duk_resize_buffer(duk_context *ctx, int index, size_t new_size);
void duk_to_fixed_buffer(duk_context *ctx, int index);

/*
 *  Property access
 *
 *  The basic function assumes key is on stack.  The _string variant takes
 *  a C string as a property name, while the _index variant takes an array
 *  index as a property name (e.g. 123 is equivalent to the key "123").
 */

int duk_get_prop(duk_context *ctx, int obj_index);
int duk_get_prop_string(duk_context *ctx, int obj_index, const char *key);
int duk_get_prop_index(duk_context *ctx, int obj_index, unsigned int arr_index);
int duk_put_prop(duk_context *ctx, int obj_index);
int duk_put_prop_string(duk_context *ctx, int obj_index, const char *key);
int duk_put_prop_index(duk_context *ctx, int obj_index, unsigned int arr_index);
int duk_del_prop(duk_context *ctx, int obj_index);
int duk_del_prop_string(duk_context *ctx, int obj_index, const char *key);
int duk_del_prop_index(duk_context *ctx, int obj_index, unsigned int arr_index);
int duk_has_prop(duk_context *ctx, int obj_index);
int duk_has_prop_string(duk_context *ctx, int obj_index, const char *key);
int duk_has_prop_index(duk_context *ctx, int obj_index, unsigned int arr_index);

/*
 *  Variable access
 */

/* FIXME: incomplete, not usable now */
void duk_get_var(duk_context *ctx);
void duk_put_var(duk_context *ctx);
int duk_del_var(duk_context *ctx);
int duk_has_var(duk_context *ctx);

/*
 *  Object operations
 */

void duk_compact(duk_context *ctx, int obj_index);
void duk_enum(duk_context *ctx, int obj_index, int enum_flags);
int duk_next(duk_context *ctx, int enum_index, int get_value);

/*
 *  String manipulation
 */

void duk_concat(duk_context *ctx, unsigned int count);
void duk_join(duk_context *ctx, unsigned int count);
void duk_decode_string(duk_context *ctx, int index, duk_decode_char_function callback, void *udata);
void duk_map_string(duk_context *ctx, int index, duk_map_char_function callback, void *udata);
void duk_substring(duk_context *ctx, int index, size_t start_offset, size_t end_offset);
void duk_trim(duk_context *ctx, int index);

/*
 *  Ecmascript operators
 */

int duk_equals(duk_context *ctx, int index1, int index2);
int duk_strict_equals(duk_context *ctx, int index1, int index2);

/*
 *  Function (method) calls
 *
 *  If 'errhandler_index' is DUK_INVALID_INDEX, the current errhandler will be
 *  used.  If 'errhandler_index' points to an undefined value in the stack,
 *  a NULL errhandler will be used, replacing any existing errhandler.
 */

void duk_call(duk_context *ctx, int nargs);
void duk_call_method(duk_context *ctx, int nargs);
void duk_call_prop(duk_context *ctx, int obj_index, int nargs);
int duk_pcall(duk_context *ctx, int nargs, int errhandler_index);
int duk_pcall_method(duk_context *ctx, int nargs, int errhandler_index);
int duk_pcall_prop(duk_context *ctx, int obj_index, int nargs, int errhandler_index);
void duk_new(duk_context *ctx, int nargs);
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

void duk_eval_raw(duk_context *ctx);
void duk_compile(duk_context *ctx, int flags);

#define duk_eval(ctx)  \
	do { \
		(void) duk_push_string((ctx),__FILE__); \
		duk_eval_raw((ctx)); \
	} while (0)

#define duk_eval_string(ctx,src)  \
	do { \
		(void) duk_push_string((ctx),(src)); \
		(void) duk_push_string((ctx),__FILE__); \
		duk_eval_raw((ctx)); \
	} while (0)

#define duk_compile_string(ctx,flags,src)  \
	do { \
		(void) duk_push_string((ctx),(src)); \
		(void) duk_push_string((ctx),__FILE__); \
		duk_compile((ctx), (flags)); \
	} while (0)

#define duk_eval_file(ctx,path)  \
	do { \
		(void) duk_push_string_file((ctx),(path)); \
		(void) duk_push_string((ctx),(path)); \
		duk_eval_raw((ctx)); \
	} while (0)

#define duk_compile_file(ctx,flags,path)  \
	do { \
		(void) duk_push_string_file((ctx),(path)); \
		(void) duk_push_string((ctx),(path)); \
		duk_compile((ctx), (flags)); \
	} while (0)

#ifdef __cplusplus
}
#endif

#endif  /* DUKTAPE_H_INCLUDED */

