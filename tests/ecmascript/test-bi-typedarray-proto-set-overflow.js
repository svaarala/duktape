/*
 *  Overflow tests for TypedArray set().
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
    var v, v2;

    // Offset argument (2nd arg) is an element index which gets translated to
    // a byte index by shifting, ensure overflow handling is correct (especially
    // on 32-bit platforms).

    try {
        v = new Uint32Array(16);   // shift=2 (x4)
        v.set([1,2,3], 0x40000001);
        print('should not be here');
    } catch (e) {
        print(e.name);
    }

    // When source for set() is a another TypedArray, there are two shifts:
    //   - Source byte length is shifted right to arrive at source element
    //     count.  This shift is always safe.
    //   - Source element count is left shifted to arrive at target byte
    //     count.  This might overflow.

    try {
        v = new Float64Array(16);   // shift=3 (x8)
        v2 = new Uint8Array(0x20000000);  // just overflows with <<3; 512MB
        v.set(v2);
        print('should not be here');
    } catch (e) {
        print(e.name);
    }
}

try {
    overflowTest();
} catch (e) {
    print(e.stack || e);
}
