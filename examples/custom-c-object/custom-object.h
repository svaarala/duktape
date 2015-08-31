#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "duktape.h"

typedef struct vector2d_t {
	float x; float y;
} vector2D;

duk_ret_t duk_register_shape(duk_context *ctx);
duk_ret_t duk_register_Vector2D(duk_context *ctx); 
