# For manual testing; say 'make' in extras/module-node and run ./test.

.PHONY: test
test:
	gcc -o $@ -I../../src/ -I. ../../src/duktape.c duk_module_node.c test.c -lm
	@printf '\n'
	./test 'typeof require("pig") === "string" ? "require() OK" : "require() FAILED";'
	./test 'require("cow").indexOf("pig") !== -1 ? "nested require() OK" : "nested require() FAILED";'
	./test 'var ape1 = require("ape"); var ape2 = require("ape"); ape1 === ape2 ? "caching OK" : "caching FAILED";'
	./test 'var ape1 = require("ape"); var inCache = "ape.js" in require.cache; delete require.cache["ape.js"]; var ape2 = require("ape"); inCache && ape2 !== ape1 ? "require.cache OK" : "require.cache FAILED";'
	./test 'var ape = require("ape"); typeof ape.module.require === "function" ? "module.require OK" : "module.require FAILED";'
	./test 'var ape = require("ape"); ape.module.exports === ape ? "module.exports OK" : "module.exports FAILED";'
	./test 'var ape = require("ape"); ape.module.id === "ape.js" && ape.module.id === ape.module.filename ? "module.id OK" : "module.id FAILED";'
	./test 'var ape = require("ape"); ape.module.filename === "ape.js" ? "module.filename OK" : "module.filename FAILED";'
	./test 'var ape = require("ape"); ape.module.loaded === true && ape.wasLoaded === false ? "module.loaded OK" : "module.loaded FAILED";'
	./test 'var ape = require("ape"); ape.__filename === "ape.js" ? "__filename OK" : "__filename FAILED";'
