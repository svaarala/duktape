#if !defined(EVENTLOOP_H)
#define EVENTLOOP_H

#include "duktape.h"

void eventloop_register(duk_context *);

void eventloop_run(duk_context *);

#endif
