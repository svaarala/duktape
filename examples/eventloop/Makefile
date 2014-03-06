evloop:
	gcc -o $@ -std=c99 -Wall -Wextra -O2 -I../../src \
		main.c \
		c_eventloop.c \
		poll.c \
		socket.c \
		fileio.c \
		ncurses.c \
		../../src/duktape.c \
		-lm -lncurses
