/*
 * Copyright (c) 2015 Nanchao Inc. All rights reserved.
 * Written by Shanjin Yang <sjyangv0@qq.com>.
 */

#include "duktape.h"
#include "uart.h"

static int ruff_uart_open(duk_context *ctx)
{
	char *uart_name;
	uart_name = (char *)duk_to_string(ctx, -1);

	int uart_fd = uart_open(uart_name);
	fprintf(stderr, "uart %s fd %d open...\n", uart_name, uart_fd);
	
	if (uart_fd < 0) {
		return DUK_RET_API_ERROR;
	} else {
		duk_push_int(ctx, uart_fd);
		return 1;
	}
}

static int ruff_uart_close(duk_context *ctx)
{
	int uart_fd = duk_to_int(ctx, -1);

	fprintf(stderr, "uart fd %d close...\n", uart_fd);
	uart_close(uart_fd);

	return 0;
}

static int ruff_uart_read(duk_context *ctx)
{
	char *p;
	int size = 1024;
	char buf[size];
	memset(buf, '\0', size); 
	int uart_fd = duk_to_int(ctx, 0);
	/*fprintf(stderr, "uart %d read...\n", uart_fd);*/
	uart_read(uart_fd, buf, size);
#if 0
	/*TODO*/
	int i = 10;
	while (i--) {
		uart_read(uart_fd, buf, size);
		fprintf(stderr, "uart %d read: %c\n", uart_fd, (char)buf[0]);
	}
#endif 
	p = duk_push_buffer(ctx, size, 0);
	duk_push_string(ctx, buf);

	return 1;
}

static int ruff_uart_write(duk_context *ctx)
{
	int uart_fd = duk_to_int(ctx, 0);
	char *write_buf;
	write_buf = (char *)duk_to_string(ctx, 1);
	fprintf(stderr, "uart fd %d write: %s", uart_fd, write_buf);
	/*TODO*/
	uart_write(uart_fd, write_buf, strlen(write_buf));
	return 0;
}

static struct duk_function_list_entry uart_func[] = {
	{ "open", ruff_uart_open, 1 },
	{ "close", ruff_uart_close, 1 },
	{ "read", ruff_uart_read, 1},
	{ "write", ruff_uart_write, 2},
	{ NULL, NULL, 0 }
};

duk_ret_t dukopen_my_module(duk_context *ctx) {
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, uart_func);

    return 1;
}

int main(int argc, char **argv)
{
	duk_context *ctx;
	ctx = duk_create_heap_default();
	if (!ctx) {
		/*TODO*/
	}

	/* Module loading happens with a Duktape/C call wrapper. */
	duk_push_c_function(ctx, dukopen_my_module, 0 /*nargs*/);
	duk_call(ctx, 0);
	duk_put_global_string(ctx, "uart");

	/* my_module is now registered in the global object. */
	duk_eval_file_noresult(ctx, "uart.js");

	duk_destroy_heap(ctx);

	return 0;
}
