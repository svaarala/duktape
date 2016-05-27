# For manual testing; say 'make' in extras/module-duktape and run ./test.
# There's test coverage in tests/ecmascript, so tests here are very simple.

.PHONY: test
test:
	gcc -std=c99 -Wall -Wextra -o $@ -I../../src/ -I. ../../src/duktape.c duk_module_duktape.c test.c -lm
	@printf '\n'
	./test 'assert(typeof require === "function");'
	./test 'assert(require.name === "require");'
	./test 'assert(typeof Duktape.modLoaded === "object");'
	./test 'assert(typeof Duktape.modSearch === "undefined");'
	./test 'Duktape.modSearch = function myModSearch(id) { return "exports.foo = 123;" }; assert(require("dummy").foo === 123);'
	./test 'Duktape.modSearch = function myModSearch(id) { return "exports.foo = 234;" }; delete Duktape; assert(typeof Duktape === "undefined"); assert(require("dummy").foo === 234);'
