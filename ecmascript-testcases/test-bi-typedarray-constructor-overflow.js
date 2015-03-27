/*
 *  A few overflow cases for TypedArray constructor.
 */

/*---
{
    "custom": true
}
---*/

/*===
RangeError
RangeError
===*/

function overflowTest() {
    var buf = new ArrayBuffer(16);
    var v;

    // On a 32-bit platform the element length 0x40000001 is shifted << 3
    // to get the byte length of the view.  Without an explicit overflow
    // check, the byte length overflows and becomes 0x00000008.  This would
    // not be a memory safety issue because the final view length is checked
    // against the input ArrayBuffer and also against the underlying buffer
    // at runtime.  Still, the wrapping should be checked for.

    try {
        v = new Float64Array(buf, 0, 0x40000001);
        print('should not be here');
        print(v.length);
    } catch (e) {
        print(e.name);
    }

    // Similar issue when argument is Array-like.  Without an overflow check
    // a result Float32Array of element length 1 is created but the constructor
    // will loop through 1G elements when populating the result.  Memory safety
    // would not be compromised, but wrapping should still be checked for.

    try {
        v = new Float32Array({ length: 0x40000001 });
        print('should not be here');
        print(v.length);
    } catch (e) {
        print(e.name);
    }
}

try {
    overflowTest();
} catch (e) {
    print(e.stack || e);
}
