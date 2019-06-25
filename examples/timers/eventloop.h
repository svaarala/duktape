#if !defined(EVENTLOOP_H)
#define EVENTLOOP_H

#include "duktape.h"

duk_ret_t eventloop_register(duk_context *, void *);

void eventloop_run(duk_context *);

#endif
