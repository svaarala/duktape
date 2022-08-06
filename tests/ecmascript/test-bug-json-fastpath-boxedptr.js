/*
 *  Duktape 1.3.0 JSON.stringify() fast path bug which caused a boxed pointer
 *  to be emitted as "{}" instead of being omitted.
 */

/*---
custom: true
duktape_config:
  DUK_USE_JSON_STRINGIFY_FASTPATH: true
---*/

/*===
[null,null]
[null,null]
===*/

function test() {
    // Fast path enabled
    print(JSON.stringify([ Duktape.Pointer('dummy'), new Duktape.Pointer('dummy') ]));

    // Fast path disabled (replacer)
    print(JSON.stringify([ Duktape.Pointer('dummy'), new Duktape.Pointer('dummy') ], function id(k,v) { return v; }));
}

test();
