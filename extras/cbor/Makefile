# For manual testing; say 'make' in extras/cbor and run ./test.

VALGRIND=valgrind
.PHONY: all
all: clean test jsoncbor test-runs

.PHONY: clean
clean:
	-rm -f test jsoncbor

test: test.c duk_cbor.c duk_cbor.h
	gcc -std=c99 -Wall -Wextra -g -ggdb -o $@ -I../../src/ -I. ../../src/duktape.c duk_cbor.c test.c -lm

jsoncbor: jsoncbor.c duk_cbor.c duk_cbor.h
	gcc -std=c99 -Wall -Wextra -O2 -o $@ -I../../src/ -I. ../../src/duktape.c duk_cbor.c jsoncbor.c -lm

.PHONY: test-runs
test-runs: test jsoncbor enc-primitives enc-number enc-buffer enc-string enc-array enc-object enc-misc dec-misc

.PHONY: dec-misc
dec-misc: jsoncbor
	python -c 'import sys; sys.stdout.write("\xa1cfoocbar")'| python cbordecode.py
	python -c 'import sys; sys.stdout.write("\xa1cfoocbar")'| $(VALGRIND) -q ./jsoncbor -d
	python -c 'import sys; sys.stdout.write("\xa1\xa0\xa0")'| python cbordecode.py
	python -c 'import sys; sys.stdout.write("\xa1\xa0\xa0")'| $(VALGRIND) -q ./jsoncbor -d  # object key, gets string coerced in JSON output
	python -c 'import sys; sys.stdout.write("\x7f\xff")'| python cbordecode.py
	python -c 'import sys; sys.stdout.write("\x7f\xff")'| $(VALGRIND) -q ./jsoncbor -d
	python -c 'import sys; sys.stdout.write("\x7f`````````````````````````````````````````````````````````````````````````````````````````````````````````\xff")'| python cbordecode.py
	python -c 'import sys; sys.stdout.write("\x7f`````````````````````````````````````````````````````````````````````````````````````````````````````````\xff")'| $(VALGRIND) -q ./jsoncbor -d
	python -c 'import sys; sys.stdout.write("\x7fcfoo\xff")'| python cbordecode.py
	python -c 'import sys; sys.stdout.write("\x7fcfoo\xff")'| $(VALGRIND) -q ./jsoncbor -d
	python -c 'import sys; sys.stdout.write("\x7fcfoocbaraqau`auax\xff")'| python cbordecode.py
	python -c 'import sys; sys.stdout.write("\x7fcfoocbaraqau`auax\xff")'| $(VALGRIND) -q ./jsoncbor -d
	python -c 'import sys; sys.stdout.write("\x5f\xff")'| python cbordecode.py
	python -c 'import sys; sys.stdout.write("\x5f\xff")'| $(VALGRIND) -q ./jsoncbor -d
	python -c 'import sys; sys.stdout.write("\x5f@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\xff")'| python cbordecode.py
	python -c 'import sys; sys.stdout.write("\x5f@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\xff")'| $(VALGRIND) -q ./jsoncbor -d
	python -c 'import sys; sys.stdout.write("\x5fCfoo\xff")'| python cbordecode.py
	python -c 'import sys; sys.stdout.write("\x5fCfoo\xff")'| $(VALGRIND) -q ./jsoncbor -d
	python -c 'import sys; sys.stdout.write("\x5fCfooCbarAqAu@AuAx\xff")'| python cbordecode.py
	python -c 'import sys; sys.stdout.write("\x5fCfooCbarAqAu@AuAx\xff")'| $(VALGRIND) -q ./jsoncbor -d
	python -c 'import sys; sys.stdout.write("\xc0cfoo")'| python cbordecode.py
	python -c 'import sys; sys.stdout.write("\xc0cfoo")'| $(VALGRIND) -q ./jsoncbor -d
	python -c 'import sys; sys.stdout.write("\xc0\xc1cfoo")'| python cbordecode.py
	python -c 'import sys; sys.stdout.write("\xc0\xc1cfoo")'| $(VALGRIND) -q ./jsoncbor -d
	python -c 'import sys; sys.stdout.write("\xc0\xd7cfoo")'| python cbordecode.py
	python -c 'import sys; sys.stdout.write("\xc0\xd7cfoo")'| $(VALGRIND) -q ./jsoncbor -d
	python -c 'import sys; sys.stdout.write("\xc0\xd8\xffcfoo")'| python cbordecode.py
	python -c 'import sys; sys.stdout.write("\xc0\xd8\xffcfoo")'| $(VALGRIND) -q ./jsoncbor -d

.PHONY: enc-primitives
enc-primitives: test
	$(VALGRIND) -q ./test 'void 0' | python2 cbordecode.py
	$(VALGRIND) -q ./test 'null' | python2 cbordecode.py
	$(VALGRIND) -q ./test 'true' | python2 cbordecode.py
	$(VALGRIND) -q ./test 'false' | python2 cbordecode.py

.PHONY: enc-number
enc-number: test
	$(VALGRIND) -q ./test '0' | python2 cbordecode.py
	$(VALGRIND) -q ./test '0.1' | python2 cbordecode.py
	$(VALGRIND) -q ./test '1' | python2 cbordecode.py
	$(VALGRIND) -q ./test '23' | python2 cbordecode.py
	$(VALGRIND) -q ./test '24' | python2 cbordecode.py
	$(VALGRIND) -q ./test '25' | python2 cbordecode.py
	$(VALGRIND) -q ./test '255' | python2 cbordecode.py
	$(VALGRIND) -q ./test '256' | python2 cbordecode.py
	$(VALGRIND) -q ./test '65535' | python2 cbordecode.py
	$(VALGRIND) -q ./test '65536' | python2 cbordecode.py
	$(VALGRIND) -q ./test '65536.9' | python2 cbordecode.py
	$(VALGRIND) -q ./test '1048576' | python2 cbordecode.py  # shortest encoding: half-float
	$(VALGRIND) -q ./test '4294967295' | python2 cbordecode.py
	$(VALGRIND) -q ./test '4294967296' | python2 cbordecode.py
	$(VALGRIND) -q ./test '4294967297' | python2 cbordecode.py
	$(VALGRIND) -q ./test '0xdeadbeef' | python2 cbordecode.py
	$(VALGRIND) -q ./test '3.141592653589793' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-0' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-0.1' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-1' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-23' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-24' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-25' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-255' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-256' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-257' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-65535' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-65536' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-65536.9' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-65537' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-1048576' | python2 cbordecode.py  # shortest encoding: half-float
	$(VALGRIND) -q ./test '-4294967295' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-4294967296' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-4294967297' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-0xdeadbeef' | python2 cbordecode.py
	$(VALGRIND) -q ./test '-3.141592653589793' | python2 cbordecode.py
	$(VALGRIND) -q ./test '1/0' | python2 cbordecode.py  # shortest encoding: half-float
	$(VALGRIND) -q ./test '0/0' | python2 cbordecode.py  # shortest encoding: half-float
	$(VALGRIND) -q ./test '-1/0' | python2 cbordecode.py  # shortest encoding: half-float

.PHONY: enc-buffer
enc-buffer: test
	$(VALGRIND) -q ./test '(function () { var p = Uint8Array.allocPlain(0); return p; })()' | python2 cbordecode.py
	$(VALGRIND) -q ./test '(function () { var p = Uint8Array.allocPlain(1); p[0] = 0xfe; return p; })()' | python2 cbordecode.py
	$(VALGRIND) -q ./test '(function () { var p = Uint8Array.allocPlain(2); p[0] = 0xc0; p[1] = 0x80; return p; })()' | python2 cbordecode.py
	$(VALGRIND) -q ./test '(function () { var p = Uint8Array.allocPlain(3); p[0] = 0x31; p[1] = 0x4a; p[2] = 0x7a; return p; })()' | python2 cbordecode.py
	$(VALGRIND) -q ./test '(function () { var p = Uint8Array.allocPlain(23); p[22] = 0xfe; return p; })()' | python2 cbordecode.py
	$(VALGRIND) -q ./test '(function () { var p = Uint8Array.allocPlain(24); p[23] = 0xfe; return p; })()' | python2 cbordecode.py
	$(VALGRIND) -q ./test '(function () { var p = Uint8Array.allocPlain(255); p[254] = 0xfe; return p; })()' | python2 cbordecode.py
	$(VALGRIND) -q ./test '(function () { var p = Uint8Array.allocPlain(256); p[255] = 0xfe; return p; })()' | python2 cbordecode.py
	$(VALGRIND) -q ./test '(function () { var p = Uint8Array.allocPlain(256); p[255] = 0xfe; return p; })()' | python2 cbordecode.py
	$(VALGRIND) -q ./test '(function () { var p = new Uint16Array([ 1, 2, 3, 4 ]); return p; })()' | python2 cbordecode.py

.PHONY: enc-string
enc-string: test
	$(VALGRIND) -q ./test '""' | python2 cbordecode.py
	$(VALGRIND) -q ./test '"foo"' | python2 cbordecode.py
	$(VALGRIND) -q ./test '"foo\u20acbar"' | python2 cbordecode.py
	$(VALGRIND) -q ./test '"foo\ud800bar"' | python2 cbordecode.py  # unpaired surrogate
	$(VALGRIND) -q ./test '"\u4321\u4321\u4321\u4321\u4321\u4321\u4321xy"' | python2 cbordecode.py
	$(VALGRIND) -q ./test '"\u4321\u4321\u4321\u4321\u4321\u4321\u4321xyz"' | python2 cbordecode.py
	$(VALGRIND) -q ./test '"\u4321\u4321\u4321\u4321\u4321\u4321\u4321xyzw"' | python2 cbordecode.py
	$(VALGRIND) -q ./test '"\u4321\u4321\u4321\u4321\u4321\u4321\u4321xyzw......................................................................................................................................................................................................................................"' | python2 cbordecode.py
	$(VALGRIND) -q ./test '"\u4321\u4321\u4321\u4321\u4321\u4321\u4321xyzw......................................................................................................................................................................................................................................!"' | python2 cbordecode.py

.PHONY: enc-array
enc-array: test
	$(VALGRIND) -q ./test '[]' | python2 cbordecode.py
	$(VALGRIND) -q ./test '[ "foo", "bar", "quux" ]' | python2 cbordecode.py
	# XXX: other lengths
	$(VALGRIND) -q ./test '([[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[{ foo: 123 }]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]])' | python2 cbordecode.py

.PHONY: enc-object
enc-object: test
	$(VALGRIND) -q ./test '({})' | python2 cbordecode.py
	$(VALGRIND) -q ./test '({ foo: 123 })' | python2 cbordecode.py
	$(VALGRIND) -q ./test '({ foo: 123, bar: 234, quux: 345 })' | python2 cbordecode.py
	$(VALGRIND) -q ./test '({ foo: 123, bar: [ "foo", "bar", { baz: true } ], quux: 345 })' | python2 cbordecode.py
	$(VALGRIND) -q ./test '({ a:1, b:2, c:3, d:4, e:5, f:6, g:7, h:8, i:9, j:10, k:11, l:12, m:13, n:14, o:15, p:16, q:17, r:18, s:19, t:20, u:21, v:22 })' | python2 cbordecode.py
	$(VALGRIND) -q ./test '({ a:1, b:2, c:3, d:4, e:5, f:6, g:7, h:8, i:9, j:10, k:11, l:12, m:13, n:14, o:15, p:16, q:17, r:18, s:19, t:20, u:21, v:22, w:23 })' | python2 cbordecode.py
	$(VALGRIND) -q ./test '({ a:1, b:2, c:3, d:4, e:5, f:6, g:7, h:8, i:9, j:10, k:11, l:12, m:13, n:14, o:15, p:16, q:17, r:18, s:19, t:20, u:21, v:22, w:23, x:24 })' | python2 cbordecode.py
	$(VALGRIND) -q ./test '({ a:1, b:2, c:3, d:4, e:5, f:6, g:7, h:8, i:9, j:10, k:11, l:12, m:13, n:14, o:15, p:16, q:17, r:18, s:19, t:20, u:21, v:22, w:23, x:24, y:25 })' | python2 cbordecode.py
	$(VALGRIND) -q ./test '({ a:1, b:2, c:3, d:4, e:5, f:6, g:7, h:8, i:9, j:10, k:11, l:12, m:13, n:14, o:15, p:16, q:17, r:18, s:19, t:20, u:21, v:22, w:23, x:24, y:25, z:26 })' | python2 cbordecode.py
	$(VALGRIND) -q ./test '({foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:{foo:123}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}})' | python2 cbordecode.py

.PHONY: enc-misc
enc-misc: test
	$(VALGRIND) -q ./test '({ jsonrpc: "2.0", id: "foo-1", method: "Add", params: { a: 123, b: 234 }})' | python2 cbordecode.py
