# octane/mandreel.js: exceeds const limit at present.
# octane/run.js: not needed, duktape_harness.js handles
# octane/typescript-compiler.js: I/O to be stubbed
# octane/typescript-input.js: -""-
# octane/typescript.js: -""-
# octane/zlib.js: read() dependency
# octane/zlib-data.js: -""-
# octane/regexp.js: checksum error

TESTS= \
	octane/base.js \
	octane/box2d.js \
	octane/code-load.js \
	octane/crypto.js \
	octane/deltablue.js \
	octane/earley-boyer.js \
	octane/gbemu-part1.js \
	octane/gbemu-part2.js \
	octane/navier-stokes.js \
	octane/pdfjs.js \
	octane/raytrace.js \
	octane/richards.js \
	octane/splay.js

.PHONY: test
test: octane
	if [ ! -f "../../duk.O2" ]; then echo "*** Expected ../../duk.O2 to exist"; exit 1; fi
	../../duk.O2 $(TESTS) duktape_harness.js

octane:
	git clone https://github.com/chromium/octane.git
	touch $@
