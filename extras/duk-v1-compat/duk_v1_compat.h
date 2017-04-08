#if !defined(DUK_V1_COMPAT_INCLUDED)
#define DUK_V1_COMPAT_INCLUDED

#include "duktape.h"

/* Straight flag rename */
#if !defined(DUK_ENUM_INCLUDE_INTERNAL)
#define DUK_ENUM_INCLUDE_INTERNAL DUK_ENUM_INCLUDE_HIDDEN
#endif

/* Flags for duk_push_string_file_raw() */
#define DUK_STRING_PUSH_SAFE              (1 << 0)    /* no error if file does not exist */

extern void duk_dump_context_stdout(duk_context *ctx);
extern void duk_dump_context_stderr(duk_context *ctx);
extern const char *duk_push_string_file_raw(duk_context *ctx, const char *path, duk_uint_t flags);
extern void duk_eval_file(duk_context *ctx, const char *path);
extern void duk_eval_file_noresult(duk_context *ctx, const char *path);
extern duk_int_t duk_peval_file(duk_context *ctx, const char *path);
extern duk_int_t duk_peval_file_noresult(duk_context *ctx, const char *path);
extern void duk_compile_file(duk_context *ctx, duk_uint_t flags, const char *path);
extern duk_int_t duk_pcompile_file(duk_context *ctx, duk_uint_t flags, const char *path);
extern void duk_to_defaultvalue(duk_context *ctx, duk_idx_t idx, duk_int_t hint);

#define duk_push_string_file(ctx,path) \
	duk_push_string_file_raw((ctx), (path), 0)

typedef struct {
	const char *key;
	duk_c_function value;
	duk_idx_t nargs;
} duk_function_list_entry;

typedef struct {
	const char *key;
	duk_double_t value;
} duk_number_list_entry;

void duk_put_number_list(duk_context *ctx, duk_idx_t obj_idx, const duk_number_list_entry *numbers);
void duk_put_function_list(duk_context *ctx, duk_idx_t obj_idx, const duk_function_list_entry *funcs);

#endif  /* DUK_V1_COMPAT_INCLUDED */
