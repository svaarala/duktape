# For manual testing; say 'make' in extras/module-node and run ./test.

.PHONY: test
test:
	gcc -std=c99 -Wall -Wextra -o $@ -I../../src/ -I. ../../src/duktape.c duk_module_node.c test.c -lm
	@printf '\n'
	./test 'assert(typeof require("pig") === "string", "basic require()");'
	./test 'assert(require("cow").indexOf("pig") !== -1, "nested require()");'
	./test 'var ape1 = require("ape"); var ape2 = require("ape"); assert(ape1 === ape2, "caching");'
	./test 'var ape1 = require("ape"); var inCache = "ape.js" in require.cache; delete require.cache["ape.js"]; var ape2 = require("ape"); assert(inCache && ape2 !== ape1, "require.cache");'
	./test 'var ape = require("ape"); assert(typeof ape.module.require === "function", "module.require()");'
	./test 'var ape = require("ape"); assert(ape.module.exports === ape, "module.exports");'
	./test 'var ape = require("ape"); assert(ape.module.id === "ape.js" && ape.module.id === ape.module.filename, "module.id");'
	./test 'var ape = require("ape"); assert(ape.module.filename === "ape.js", "module.filename");'
	./test 'var ape = require("ape"); assert(ape.module.loaded === true && ape.wasLoaded === false, "module.loaded");'
	./test 'var ape = require("ape"); assert(ape.__filename === "ape.js", "__filename");'
	./test 'var badger = require("badger"); assert(badger.foo === 123 && badger.bar === 234, "exports.foo assignment");'
