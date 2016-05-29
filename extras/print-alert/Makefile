# For manual testing; say 'make' in extras/print-alert and run ./test.
.PHONY: test
test:
	gcc -std=c99 -Wall -Wextra -otest -I../../src ../../src/duktape.c duk_print_alert.c test.c -lm
	./test 'print("foo", "bar", 1, 2, 3)'
	./test 'alert("foo", "bar", 1, 2, 3)'
