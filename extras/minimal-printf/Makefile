# Just for manual testing
.PHONY: test
test: duk_minimal_printf.c
	gcc -fno-stack-protector -m32 -otest -Wall -Wextra -Os -fomit-frame-pointer duk_minimal_printf.c test.c
	./test
