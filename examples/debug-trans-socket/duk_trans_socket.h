#if !defined(DUK_TRANS_SOCKET_H_INCLUDED)
#define DUK_TRANS_SOCKET_H_INCLUDED

#include "duktape.h"

#if defined(__cplusplus)
extern "C" {
#endif

void duk_trans_socket_init(void);
void duk_trans_socket_finish(void);
void duk_trans_socket_waitconn(void);
duk_size_t duk_trans_socket_read_cb(void *udata, char *buffer, duk_size_t length);
duk_size_t duk_trans_socket_write_cb(void *udata, const char *buffer, duk_size_t length);
duk_size_t duk_trans_socket_peek_cb(void *udata);
void duk_trans_socket_read_flush_cb(void *udata);
void duk_trans_socket_write_flush_cb(void *udata);

#if defined(__cplusplus)
}
#endif  /* end 'extern "C"' wrapper */

#endif  /* DUK_TRANS_SOCKET_H_INCLUDED */
