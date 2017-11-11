# For manual testing; say 'make' in extras/duk-v1-compat and run ./test.

.PHONY: test
test:
	-rm -rf ./prep
	python2 ../../tools/configure.py --quiet --output-directory ./prep
	gcc -std=c99 -Wall -Wextra -o $@ -I./prep -I. ./prep/duktape.c duk_v1_compat.c test.c -lm
	./test

.PHONY: clean
clean:
	-rm -rf ./prep
	-rm -f test
