evloop:
	#eventloop.c
	gcc -o $@ -std=c99 -O2 -I../../src \
		main.c \
		duk_socket.c \
		duk_fileio.c \
		duk_ncurses.c \
		../../src/duktape.c \
		-lm -lncurses
