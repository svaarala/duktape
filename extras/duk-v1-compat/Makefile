# For manual testing; say 'make' in extras/duk-v1-compat and run ./test.

.PHONY: test
test:
	gcc -o $@ -I../../src/ -I. ../../src/duktape.c duk_v1_compat.c test.c -lm
	./test
