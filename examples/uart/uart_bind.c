/*
 * Copyright (c) 2015 Nanchao Inc. All rights reserved.
 * Written by Shanjin Yang <sjyangv0@qq.com>
 */

#include "duktape.h"
#include "uart.h"

static int uart_fd;
static int ruff_uart_open(duk_context *ctx)
{
	char filename[32];
	void *uart_name;
	duk_size_t size;

	uart_name = duk_to_buffer(ctx, -3, &size);

	int uart_fd = uart_open(uart_name);
	if (uart_fd < 0) {
		return DUK_RET_API_ERROR;
	} else {
		duk_push_string(ctx, filename);
		return 0;
	}
}

static int ruff_uart_close(duk_context *ctx)
{
	uart_close(uart_fd);
	return 0;
}

static int ruff_uart_read(duk_context *ctx)
{
	void *p;
	int size = 1024;
	char buf[size];
	/*TODO*/
	uart_read(uart_fd, buf, size);

	p = duk_push_buffer(ctx, size, 0);
	//fprintf(stderr, "%c", (char *)p);
	return 0;
}

static int ruff_uart_write(duk_context *ctx)
{
	void *p;
	duk_size_t size;

	p = duk_to_buffer(ctx, -3, &size);
	/*TODO*/
	uart_write(uart_fd, p, size);
	return 0;
}

static struct duk_function_list_entry uart_func[] = {
	{ "open", ruff_uart_open, 1 },
	{ "close", ruff_uart_close, 0 },
	{ "read", ruff_uart_read, 2},
	{ "write", ruff_uart_write, 2},
	{ NULL, NULL, 0 }
};

int main(int argc, char **argv)
{
	duk_context *ctx;
	ctx = duk_create_heap_default();

	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, uart_func);
	duk_put_prop_string(ctx, -2, "UART");
	duk_pop(ctx);

	duk_destroy_heap(ctx);

	return 0;
}
