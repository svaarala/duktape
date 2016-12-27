/*@include util-buffer.js@*/

/*===
this coercion test
object true
object false
===*/

function thisCoercionTest() {
    var pb = createPlainBuffer('abcdefghijklmnop');

    function myStrict() {
        'use strict';
        print(typeof this, isPlainBuffer(this));
    }
    myStrict.call(pb);

    // The 'this' binding of a non-strict function is not further coerced if
    // it is an object.  While plain buffers mimic Uint8Array objects, they
    // are ToObject() coerced when the call target is a non-strict function.
    // This matches lightfunc 'this' binding behavior in Duktape 2.x.

    function myNonStrict() {
        print(typeof this, isPlainBuffer(this));
    }
    myNonStrict.call(pb);
}

try {
    print('this coercion test');
    thisCoercionTest();
} catch (e) {
    print(e.stack || e);
}
