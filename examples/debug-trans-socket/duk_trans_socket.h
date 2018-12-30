#if !defined(DUK_TRANS_SOCKET_H_INCLUDED)
#define DUK_TRANS_SOCKET_H_INCLUDED

#include "duktape.h"

#if defined(__cplusplus)
extern "C" {
#endif

DUK_EXTERNAL_DECL void duk_trans_socket_init(void);
DUK_EXTERNAL_DECL void duk_trans_socket_finish(void);
DUK_EXTERNAL_DECL void duk_trans_socket_waitconn(void);
DUK_EXTERNAL_DECL duk_size_t duk_trans_socket_read_cb(void *udata, char *buffer, duk_size_t length);
DUK_EXTERNAL_DECL duk_size_t duk_trans_socket_write_cb(void *udata, const char *buffer, duk_size_t length);
DUK_EXTERNAL_DECL duk_size_t duk_trans_socket_peek_cb(void *udata);
DUK_EXTERNAL_DECL void duk_trans_socket_read_flush_cb(void *udata);
DUK_EXTERNAL_DECL void duk_trans_socket_write_flush_cb(void *udata);

#if defined(__cplusplus)
}
#endif  /* end 'extern "C"' wrapper */

#endif  /* DUK_TRANS_SOCKET_H_INCLUDED */
