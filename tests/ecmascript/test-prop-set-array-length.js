/*===
direct
- array is receiver, writable length
4 [1,2,3,4]
3 [1,2,3]
- array is receiver, nonwritable length, same length
4 [1,2,3,4]
TypeError
4 [1,2,3,4]
- array is receiver, nonwritable length, higher length
4 [1,2,3,4]
TypeError
4 [1,2,3,4]
- array is receiver, nonwritable length, lower length
4 [1,2,3,4]
TypeError
4 [1,2,3,4]
- array is receiver, nonwritable length, invalid length
4 [1,2,3,4]
TypeError
4 [1,2,3,4]
- array is receiver, writable length, length: number -1
4 1 2 3 4 undefined
RangeError
4 1 2 3 4 undefined
- array is receiver, writable length, length: number -0
4 1 2 3 4 undefined
0 undefined undefined undefined undefined undefined
- array is receiver, writable length, length: number 0
4 1 2 3 4 undefined
0 undefined undefined undefined undefined undefined
- array is receiver, writable length, length: number 0.5
4 1 2 3 4 undefined
RangeError
4 1 2 3 4 undefined
- array is receiver, writable length, length: string 2
4 1 2 3 4 undefined
2 1 2 undefined undefined undefined
- array is receiver, writable length, length: string 2.0
4 1 2 3 4 undefined
2 1 2 undefined undefined undefined
- array is receiver, writable length, length: string -0
4 1 2 3 4 undefined
0 undefined undefined undefined undefined undefined
- array is receiver, writable length, length: string +0
4 1 2 3 4 undefined
0 undefined undefined undefined undefined undefined
- array is receiver, writable length, length: number 4294967295
4 1 2 3 4 undefined
4294967295 1 2 3 4 undefined
- array is receiver, writable length, length: number 4294967296
4 1 2 3 4 undefined
RangeError
4 1 2 3 4 undefined
inherited
- array is not receiver, length writable
4 1 2 3 4
3 1 2 3 4
4 [1,2,3,4]
- array is not receiver, length non-writable, write lower length
4 1 2 3 4
TypeError
4 1 2 3 4
4 [1,2,3,4]
- array is not receiver, length non-writable, write same length
4 1 2 3 4
TypeError
4 1 2 3 4
4 [1,2,3,4]
===*/

// [[Set]] where receiver is an Array.
function testDirect() {
    'use strict';
    var A, O;

    // Arrays don't have an exotic [[Set]].  Conceptually ordinary [[Set]]
    // finds the .length property (always a non-configurable data descriptor).
    // The ordinary [[Set]] validates writability first; if not writable, the
    // [[Set]] is rejected even if the new length matches the old length.
    // If the Array is writable, ordinary [[Set]] invokes Array's exotic
    // [[DefineOwnProperty]] behavior.

    // Array is receiver, .length is writable: base case.
    print('- array is receiver, writable length');
    A = [ 1, 2, 3, 4 ];
    print(A.length, JSON.stringify(A));
    A.length = 3;
    print(A.length, JSON.stringify(A));

    // Array is receiver but .length not writable, same length: [[Set]]
    // rejects write before Array [[DefineOwnProperty]] sees it.
    print('- array is receiver, nonwritable length, same length');
    A = [ 1, 2, 3, 4 ];
    Object.defineProperty(A, 'length', { writable: false });
    print(A.length, JSON.stringify(A));
    try {
        A.length = 4;
    } catch (e) {
        print(e.name);
    }
    print(A.length, JSON.stringify(A));

    // Array is receiver but .length not writable, higher length also fail
    // early in [[Set]].
    print('- array is receiver, nonwritable length, higher length');
    A = [ 1, 2, 3, 4 ];
    Object.defineProperty(A, 'length', { writable: false });
    print(A.length, JSON.stringify(A));
    try {
        A.length = 5;
    } catch (e) {
        print(e.name);
    }
    print(A.length, JSON.stringify(A));

    // Same for lower length.
    print('- array is receiver, nonwritable length, lower length');
    A = [ 1, 2, 3, 4 ];
    Object.defineProperty(A, 'length', { writable: false });
    print(A.length, JSON.stringify(A));
    try {
        A.length = 3;
    } catch (e) {
        print(e.name);
    }
    print(A.length, JSON.stringify(A));

    // Array is receiver but .length not writable, invalid length: [[Set]]
    // rejects write early.  So we see a TypeError, not a RangeError.
    print('- array is receiver, nonwritable length, invalid length');
    A = [ 1, 2, 3, 4 ];
    Object.defineProperty(A, 'length', { writable: false });
    print(A.length, JSON.stringify(A));
    try {
        A.length = 3.5;
    } catch (e) {
        print(e.name);
    }
    print(A.length, JSON.stringify(A));

    // Writable lengths, some special length values.
    [
        -1, -0, +0, 0.5, '2', '2.0', '-0', '+0',
        4294967295,
        4294967296
    ].forEach(function (v) {
        print('- array is receiver, writable length, length: ' + typeof v + ' ' + (v === 0 && 1 / v < 0 ? '-0' : v));
        var A = [ 1, 2, 3, 4 ];
        //print(A.length, JSON.stringify(A));
        print(A.length, A[0], A[1], A[2], A[3], A[4]);
        // Don't JSON.stringify(A): for length 2**32-1 it takes too long.
        try {
            A.length = v;
        } catch (e) {
            print(e.name);
        }
        //print(A.length, JSON.stringify(A));
        print(A.length, A[0], A[1], A[2], A[3], A[4]);
    });
}

// [[Set]] where receiver inherits from an Array.
function testInherited() {
    'use strict';
    var A, O;

    // Conceptually [[Set]] finds .length property in parent Array and notices
    // it is writable.  The final [[Set]] write on the receiver is ordinary and
    // writes to 'O' without affecting the Array.
    print('- array is not receiver, length writable');
    A = [ 1, 2, 3, 4 ];
    O = Object.create(A);
    print(O.length, O[0], O[1], O[2], O[3]);
    O.length = 3;
    print(O.length, O[0], O[1], O[2], O[3]);
    print(A.length, JSON.stringify(A));

    // If length is non-writable, all attempts to write to the inheriting object
    // .length should fail.
    print('- array is not receiver, length non-writable, write lower length');
    A = [ 1, 2, 3, 4 ];
    Object.defineProperty(A, 'length', { writable: false });
    O = Object.create(A);
    print(O.length, O[0], O[1], O[2], O[3]);
    try {
        O.length = 3;
    } catch (e) {
        print(e.name);
    }
    print(O.length, O[0], O[1], O[2], O[3]);
    print(A.length, JSON.stringify(A));

    print('- array is not receiver, length non-writable, write same length');
    A = [ 1, 2, 3, 4 ];
    Object.defineProperty(A, 'length', { writable: false });
    O = Object.create(A);
    print(O.length, O[0], O[1], O[2], O[3]);
    try {
        O.length = 4;
    } catch (e) {
        print(e.name);
    }
    print(O.length, O[0], O[1], O[2], O[3]);
    print(A.length, JSON.stringify(A));
}

print('direct');
testDirect();
print('inherited');
testInherited();
