# For manual testing; say 'make' in extras/logging and run ./test.
.PHONY: test
test:
	gcc -std=c99 -Wall -Wextra -otest -I../../src ../../src/duktape.c duk_logging.c test.c -lm
	./test 'L = new Duktape.Logger(); L.trace("testing"); L.l = 0; L.trace("testing 2");'
	./test 'L = new Duktape.Logger(); L.debug("testing"); L.l = 1; L.debug("testing 2");'
	./test 'L = new Duktape.Logger(); L.info("testing");'
	./test 'L = new Duktape.Logger(); L.warn("testing");'
	./test 'L = new Duktape.Logger(); L.error("testing");'
	./test 'L = new Duktape.Logger(); L.fatal("testing");'
	./test 'L = new Duktape.Logger(); L.info("testing"); L.n = "loggerName"; L.info("new name");'
	./test 'L = new Duktape.Logger("argname"); L.info("testing");'
