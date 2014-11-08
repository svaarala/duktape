#ifndef DUK_DEBUG_TRANS_SOCKET_H_INCLUDED
#define DUK_DEBUG_TRANS_SOCKET_H_INCLUDED

#include "duktape.h"

void duk_debug_trans_socket_init(void);
void duk_debug_trans_socket_waitconn(void);
duk_size_t duk_debug_trans_socket_read(void *udata, char *buffer, duk_size_t length);
duk_size_t duk_debug_trans_socket_write(void *udata, const char *buffer, duk_size_t length);

#endif  /* DUK_DEBUG_TRANS_SOCKET_H_INCLUDED */
