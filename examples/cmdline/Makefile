cmdline: duk_cmdline.c
	gcc -o $@ -std=c99 -O2 -Wall -Wextra -I../../src \
		$< ../../src/duktape.c -lm -lreadline
