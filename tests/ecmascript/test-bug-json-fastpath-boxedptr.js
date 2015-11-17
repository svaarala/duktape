/*
 *  Duktape 1.3.0 JSON.stringify() fast path bug which caused a boxed pointer
 *  to be emitted as "{}" instead of being omitted.
 */

/*===
[null,null]
[null,null]
[null,null]
[null,null]
===*/

function test() {
    // Fast path enabled
    print(JSON.stringify([ Duktape.Pointer('dummy'), new Duktape.Pointer('dummy') ]));
    print(JSON.stringify([ Duktape.Buffer('dummy'), new Duktape.Buffer('dummy') ]));

    // Fast path disabled (replacer)
    print(JSON.stringify([ Duktape.Pointer('dummy'), new Duktape.Pointer('dummy') ], function id(k,v) { return v; }));
    print(JSON.stringify([ Duktape.Buffer('dummy'), new Duktape.Buffer('dummy') ], function id(k,v) { return v; }));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
