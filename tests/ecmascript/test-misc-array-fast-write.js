/*
 *  There's a fast path when writing to Array instances, providing the
 *  following conditions are met:
 *
 *    - The key is numeric, and is an integer in the range [0,2^32-1],
 *      the range of allowed array indices.  For example arr[7] matches
 *      this condition, arr['7'] or arr['foo'] does not.
 *
 *    - The array is not sparse, i.e. the internal representation has an
 *      array part.
 *
 *    - The array is extensible, so that creating new properties is allowed.
 *
 *    - The array write would not grow the internal allocation for the
 *      array; the write -can- increase array ".length" but can't lead
 *      to a reallocation internally.  This condition cannot be known
 *      reliably from outside, of course.
 *
 *  When the fast path is activated, Duktape won't check Array.prototype
 *  for conflicting property values which might prevent a write (e.g. a
 *  non-writable or accessor Array.prototype["7"] would capture a write to
 *  "7").
 *
 *  For ECMAScript code is difficult to predict when the fast path is active
 *  because some of the conditions are related to internal state.  However,
 *  there's no reason to do that: as long as there are no conflicting numeric
 *  properties in Array.prototype there is no outward difference in behavior
 *  (regardless of whether or not the fast path is active for a certain write).
 *
 *  Test for the non-compliant default behavior.  You can restore compliant
 *  behavior by dropping the fast path; undefine DUK_USE_ARRAY_PROP_FASTPATH.
 */

/*---
{
    "custom": true
}
---*/

/*===
empty array
ancestor-17 ancestor-getter-18
TypeError
Error: cannot write 18
0 []
sparse array
ancestor-17 ancestor-getter-18
TypeError
Error: cannot write 18
21 [null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,"ancestor-17","ancestor-getter-18",null,"quux"]
dense but allocation would grow
TypeError
Error: cannot write 18
13 [0,1,2,3,4,5,6,7,8,9,10,11,12]
dense, fast path active
21 [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,null,"foo","foo",null,"quux"]
dense, but use string key
TypeError
Error: cannot write 18
21 [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,null,"ancestor-17","ancestor-getter-18",null,"quux"]
dense, but not extensible
TypeError
Error: cannot write 18
21 [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,null,"ancestor-17","ancestor-getter-18",null,"quux"]
===*/

function arrayFastWriteTest() {
    'use strict';  // use strict mode so that write to non-writable property is not silent
    var arr;
    var i;

    /* Establish a few conflicting properties in Array.prototype.  Not sure
     * why anyone would do this in practice.
     */

    Object.defineProperties(Array.prototype, {
        "17": { value: "ancestor-17", writable: false, enumerable: false, configurable: false },
        "18": { set: function () { throw new Error('cannot write 18'); },
                get: function () { return 'ancestor-getter-18'; },
                enumerable: false, configurable: false }
    });

    /* Here the write to '17' would need to grow the internal allocation
     * so that the write skips the fast path.  As a result the write (and
     * the internal resize) gets prevented.  Same happens for '18'.
     */

    print('empty array');
    arr = [];
    print(arr[17], arr[18]);

    try {
        arr[17] = 'foo';
    } catch (e) {
        print(e.name);
    }

    try {
        arr[18] = 'foo';
    } catch (e) {
        print(e);
    }

    print(arr.length, JSON.stringify(arr));

    /* Here write to '20' skips the fast path because it extends the
     * internal allocation.  It also makes the array sparse, so that
     * the later writes to '17' and '18' don't go through the array
     * write fast path.  As a result Array.prototype checks are made
     * normally.
     */

    print('sparse array');
    arr = [];
    arr[1e6] = 123;
    arr.length = 20;
    arr[20] = 'quux';
    print(arr[17], arr[18]);

    try {
        arr[17] = 'foo';
    } catch (e) {
        print(e.name);
    }

    try {
        arr[18] = 'foo';
    } catch (e) {
        print(e);
    }

    print(arr.length, JSON.stringify(arr));

    /* Here array is not sparse but internal allocation would grow,
     * so the writes don't go through the fast path.  As a result,
     * Array.prototype checks are made normally.
     *
     * NOTE: this test is fragile and may break when internal resize
     * parameters are changed.
     */

    print('dense but allocation would grow');

    arr = [];
    for (i = 0; i < 13; i++) {
        arr[i] = i;
    }

    try {
        arr[17] = 'foo';
    } catch (e) {
        print(e.name);
    }

    try {
        arr[18] = 'foo';
    } catch (e) {
        print(e);
    }

    print(arr.length, JSON.stringify(arr));

    /* Here array is dense and allocation wouldn't grow, so fast path
     * is active and non-compliant writes are allowed.
     */

    print('dense, fast path active');

    arr = [];
    for (i = 0; i < 16; i++) {
        arr[i] = i;
    }
    arr[20] = 'quux';  // ensure allocation is large enough, and dense

    try {
        arr[17] = 'foo';
    } catch (e) {
        print(e);
    }

    try {
        arr[18] = 'foo';
    } catch (e) {
        print(e);
    }

    print(arr.length, JSON.stringify(arr));

    /* Here array is dense and allocation wouldn't grow, but we use a
     * string key so fast path is not activated.
     */

    print('dense, but use string key');

    arr = [];
    for (i = 0; i < 16; i++) {
        arr[i] = i;
    }
    arr[20] = 'quux';  // ensure allocation is large enough

    try {
        arr['17'] = 'foo';
    } catch (e) {
        print(e.name);
    }

    try {
        arr['18'] = 'foo';
    } catch (e) {
        print(e);
    }

    print(arr.length, JSON.stringify(arr));

    /* Array is dense, writes use integer keys, but array is not
     * extensible.
     */

    print('dense, but not extensible');

    arr = [];
    for (i = 0; i < 16; i++) {
        arr[i] = i;
    }
    arr[20] = 'quux';  // ensure allocation is large enough
    Object.preventExtensions(arr);

    try {
        arr[17] = 'foo';
    } catch (e) {
        print(e.name);
    }

    try {
        arr[18] = 'foo';
    } catch (e) {
        print(e);
    }

    print(arr.length, JSON.stringify(arr));
}

try {
    arrayFastWriteTest();
} catch (e) {
    print(e.stack || e);
}
