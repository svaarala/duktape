/* Behavior being fixed: https://github.com/tc39/ecma262/pull/1556. */

/*===
- inherited
child 9 ok
child 10 ok
child -0 ok
child Infinity ok
0 123 undefined undefined undefined
0 0 undefined undefined undefined
- direct
direct 9 ok
direct 10 ok
direct -0 ok
direct Infinity ok
0 123 undefined undefined undefined
0 123 undefined undefined undefined
===*/

function test() {
    'use strict';

    Object.defineProperty(Object.prototype, '9', {
        set: function () {
            print('setter called for 9');
            throw new RangeError('aiee');
        }
    });
    Object.defineProperty(Object.prototype, '10', {
        set: function () {
            print('setter called for 10');
            throw new RangeError('aiee');
        }
    });
    Object.defineProperty(Object.prototype, '-0', {
        set: function () {
            print('setter called for -0');
            throw new RangeError('aiee');
        }
    });
    Object.defineProperty(Object.prototype, 'Infinity', {
        set: function () {
            print('setter called for Infinite');
            throw new RangeError('aiee');
        }
    });

    var u8, child;

    // For inherited case 10 proceeds to setter; [[Set]] exotic does
    // not trigger, [[GetOwnProperty]] for 10 returns not found, so
    // we walk the prototype chain in ordinary [[Set]].

    print('- inherited');

    u8 = new Uint8Array(10);
    child = Object.create(u8);
    try {
        child[9] = 123;
        print('child 9 ok');
    } catch (e) {
        print(e.name);
    }
    try {
        child[10] = 234;
        print('child 10 ok');
    } catch (e) {
        print(e.name);
    }
    try {
        child['-0'] = 234;
        print('child -0 ok');
    } catch (e) {
        print(e.name);
    }
    try {
        child['Infinity'] = 234;
        print('child Infinity ok');
    } catch (e) {
        print(e.name);
    }
    print(child[0], child[9], child[10], child['-0'], child['Infinity']);
    print(u8[0], u8[9], u8[10], u8['-0'], u8['Infinity']);

    // For direct case 10 does NOT proceed to setter, but is captured.
    // Out-of-bounds write returns false from [[Set]], which in strict
    // mode should cause a TypeError.

    print('- direct');

    u8 = new Uint8Array(10);
    child = Object.create(u8);
    try {
        u8[9] = 123;
        print('direct 9 ok');
    } catch (e) {
        print(e.name);
    }
    try {
        u8[10] = 234;
        print('direct 10 ok');
    } catch (e) {
        print(e.name);
    }
    try {
        u8['-0'] = 234;
        print('direct -0 ok');
    } catch (e) {
        print(e.name);
    }
    try {
        u8['Infinity'] = 234;
        print('direct Infinity ok');
    } catch (e) {
        print(e.name);
    }
    print(child[0], child[9], child[10], child['-0'], child['Infinity']);
    print(u8[0], u8[9], u8[10], u8['-0'], u8['Infinity']);
}
test();
