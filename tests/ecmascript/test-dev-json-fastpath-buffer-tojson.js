/*
 *  Test for a JSON fast path respecting an inherited .toJSON().
 */

/*@include util-buffer.js@*/

/*===
{"0":97,"1":98,"2":99,"3":100}
|61626364|
{"_buf":"61626364"}
"replaced"
"replaced"
"replaced"
"replaced"
"replaced"
"replaced"
===*/

function test() {
    var buf = createPlainBuffer('abcd');
    function id(k, v) { return v; }

    // Base case.

    print(JSON.stringify(buf));
    print(Duktape.enc('jx', buf));
    print(Duktape.enc('jc', buf));

    // Add .toJSON() and re-run, using fast path (assuming it's enabled).

    Object.prototype.toJSON = function (v) { 'use strict'; return 'replaced'; }
    print(JSON.stringify(buf));
    print(Duktape.enc('jx', buf));
    print(Duktape.enc('jc', buf));

    // Disable fast path using an identity replacer.

    print(JSON.stringify(buf, id));
    print(Duktape.enc('jx', buf, id));
    print(Duktape.enc('jc', buf, id));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
