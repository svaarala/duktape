#if !defined(DUK_MODULE_NODE_H_INCLUDED)
#define DUK_MODULE_NODE_H_INCLUDED

#include "duktape.h"

/* Callback to allow an application to transform the filename used for a module to a canonical form.
 * The name is on the TOS and can be replaced by whatever the application needs. Don't touch the rest of the stack.
 */
typedef void (*NormalizeFunction) (duk_context *ctx);

extern duk_ret_t duk_module_node_peval_main(duk_context *ctx, const char *path);
extern void duk_module_node_init(duk_context *ctx, NormalizeFunction normalize = NULL);

#endif  /* DUK_MODULE_NODE_H_INCLUDED */
