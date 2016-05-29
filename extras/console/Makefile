# For manual testing; say 'make' in extras/print-alert and run ./test.

.PHONY: test
test:
	gcc -std=c99 -Wall -Wextra -o $@ -I../../src/ -I. ../../src/duktape.c duk_console.c test.c -lm
	./test 'console.assert(true, "not shown");'
	./test 'console.assert(false, "shown", { foo: 123 });'
	./test 'console.log(1, 2, 3, { foo: "bar" });'
	./test 'a={}; b={}; a.ref=b; console.log(a,b); b.ref=a; console.log(a,b)'  # circular ref
	./test 'console.trace(1, 2, 3)'
	./test 'console.dir({ foo: 123, bar: [ "foo", "bar" ]});'
