/*
 *  Using Array.prototype methods for buffers
 *
 *  Neither ArrayBuffer nor any of the view types inherit from Array so array
 *  methods like .join() can't be used directly like "buf.join(' ')".  But we
 *  can call Array methods explicitly and they are required to operate on
 *  anything "Array-like".  ArrayBuffer is -not- Array-like because it lacks
 *  'length', but all the views have a 'length' so they must work.
 *
 *  Spot check just a few methods, not exhaustive.
 */

/*@include util-typedarray.js@*/

/*---
{
    "custom": true,
    "endianness": "little"
}
---*/

/*===
ArrayBuffer join: 0 - 1 - 2 - 3 - 4 - 5 - 6 - 7 - 8 - 9 - 10 - 11 - 12 - 13 - 14 - 15
Uint16Array join: 0 - 31337 - 62674 - 28475 - 59812 - 25613 - 56950 - 22751
Uint16Array reverse
Uint16Array join: 22751 - 56950 - 25613 - 59812 - 28475 - 62674 - 31337 - 0
Uint16Array sort
Uint16Array join: 0 - 22751 - 25613 - 28475 - 31337 - 56950 - 59812 - 62674
Uint16Array filter even: 0,56950,59812,62674
Uint16Array indexOf 31337: 4
Uint16Array shift: TypeError
Uint16Array join: 22751 - 25613 - 28475 - 31337 - 56950 - 59812 - 62674 - 62674
===*/

function arrayMethodTest() {
    var buf = new ArrayBuffer(16);
    var i;

    // ArrayBuffer does NOT work properly here because it lacks 'length'.

    for (i = 0; i < buf.byteLength; i++) {
        buf[i] = i;
    }

    print('ArrayBuffer join:', Array.prototype.join.call(buf, ' - '));

    // But Uint16Array must work

    var v1 = new Uint16Array(buf);

    for (i = 0; i < v1.length; i++) {
        v1[i] = (i * 31337);
    }

    print('Uint16Array join:', Array.prototype.join.call(v1, ' - '));
    print('Uint16Array reverse');
    Array.prototype.reverse.call(v1);  // reverse in place
    print('Uint16Array join:', Array.prototype.join.call(v1, ' - '));
    print('Uint16Array sort');
    Array.prototype.sort.call(v1);  // sort in place
    print('Uint16Array join:', Array.prototype.join.call(v1, ' - '));
    print('Uint16Array filter even:', Array.prototype.filter.call(v1, function (v) { return (v % 2) == 0; }));
    print('Uint16Array indexOf 31337:', Array.prototype.indexOf.call(v1, 31337));

    // The 'length' property is not writable so nothing that writes length
    // can be completed.  Here e.g. 'shift' will successfully assign values
    // but fails to delete the last element or adjust length.  The result
    // is as expected from shift() but the last element is duplicated.

    try {
        print('Uint16Array shift:', Array.prototype.shift.call(v1))
    } catch (e) {
        print('Uint16Array shift:', e.name);
    }
    print('Uint16Array join:', Array.prototype.join.call(v1, ' - '));
}

try {
    arrayMethodTest();
} catch (e) {
    print(e.stack || e);
}
