/*
 *  Typed array out-of-bounds index reads should return undefined and writes
 *  should be ignored, regardless of any inherited index properties.
 *
 *  This is ES6 behavior for typed arrays; Duktape extends the same behavior
 *  to the non-standard ArrayBuffer and plain buffer indices.  Also Node.js
 *  Buffer indices behave the same; in newer Node.js versions Buffer inherits
 *  from Uint8Array.
 *
 *  https://github.com/svaarala/duktape/issues/257
 *  http://www.ecma-international.org/ecma-262/6.0/#sec-integer-indexed-exotic-objects
 *  https://bugzilla.mozilla.org/show_bug.cgi?id=829896
 */

if (typeof print !== 'function') {
    print = function () {
        console.log(Array.prototype.join.call(Array.prototype.map.call(arguments, String), ' '));
    };
}

function cleanup() {
    delete Uint16Array.prototype['-0'];
    delete Uint16Array.prototype['10'];
    delete Uint16Array.prototype['11'];
    delete Uint16Array.prototype['12.0'];
}

function setupU16Array(negZero) {
    cleanup();

    var u16 = new Uint16Array(10);

    if (negZero) {
        Object.defineProperty(u16, '-0', {
            value: 'dummy-neg-zero',
            writable: true,
            enumerable: true,
            configurable: true
        });
        Object.defineProperty(u16, '-0.0', {
            value: 'dummy-neg-zero-dot-zero',
            writable: true,
            enumerable: true,
            configurable: true
        });
    }
    Object.defineProperty(Uint16Array.prototype, '10', {
        value: 'dummy-10',
        writable: true,
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(Uint16Array.prototype, '11', {
        get: function () { print('GETTER CALLED for 11'); return 'dummy-11'; },
        set: function () { print('SETTER CALLED for 11'); },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(Uint16Array.prototype, '12.0', {
        value: 'dummy-12.0',
        writable: true,
        enumerable: true,
        configurable: true
    });

    return u16;
}

/*===
read test
- normal read
0
0
0
- out-of-bounds read
dummy-neg-zero
dummy-neg-zero-dot-zero
undefined
undefined
undefined
undefined
undefined
undefined
undefined
undefined
dummy-12.0
- zero length Uint16Array
undefined
undefined
undefined
===*/

function readTest() {
    var b = setupU16Array(true);

    // Read and write indexed elements within range normally.
    print('- normal read');
    print(b[-0]);    // String(-0) === '0', so valid index
    print(b[0]);
    print(b[9]);

    // Out-of-bounds read returns undefined, despire inherited
    // property.  This happens *only* when index is a canonical
    // one or '-0' (!).
    //
    // Negative zero is interesting: ES6 CanonicalNumberIndexString("-0")
    // returns the number -0 so it triggers exotic index behavior.
    // However, it's not allowed to actually read/write the view item
    // at index 0!
    //
    // Chrome example (same behavior on Firefox, but not on Node v0.12.1,
    // probably with newer Node):
    //
    //     > x = new Uint8Array(10);
    //     [0, 0, 0, 0, 0, 0, 0, 0]
    //     > Uint8Array.prototype['-0'] = 'WOT';
    //     "WOT"
    //     > x['-0']
    //     undefined
    //     > x['0']
    //     0

    print('- out-of-bounds read');
    print(b['-0']);    // considered a numeric index, but does not access [0] *nor* inherits...
    print(b['-0.0']);  // not considered a numeric index so does inherit
    print(b[10]);
    print(b[11]);
    print(b[12]);
    print(b['10']);
    print(b['11']);
    print(b['12']);
    print(b['10.0']);
    print(b['11.0']);
    print(b['12.0']);

    // For a zero length typed array all index properties are out-of-bounds.
    b = new Uint16Array(0);
    print('- zero length Uint16Array');
    print(b['-0']);
    print(b[0]);
    print(b['0']);
}

try {
    print('read test');
    readTest();
} catch (e) {
    print(e.stack || e);
}

/*===
write test
- normal write
0
123
123
234
0
123
- out-of-bounds Write
undefined
undefined
dummy-neg-zero-dot-zero
123
undefined
undefined
undefined
undefined
- zero length Uint16Array
undefined
undefined
undefined
undefined
undefined
undefined
===*/

function writeTest() {
    var b = setupU16Array(true);

    // Write indexed elements within range normally.
    print('- normal write');
    print(b[-0]); b[-0] = 123; print(b[-0]);  // String(-0) === '0', so valid index
    print(b[0]); b[0] = 234; print(b[0]);
    print(b[9]); b[9] = 123; print(b[9]);

    // Out-of-bounds write is ignored, despite inherited property.
    print('- out-of-bounds Write');
    print(b['-0']); b['-0'] = 123; print(b['-0']);
    print(b['-0.0']); b['-0.0'] = 123; print(b['-0.0']);
    print(b[10]); b[10] = 123; print(b[10]);
    print(b[11]); b[11] = 123; print(b[11]);

    // For a zero length typed array all index properties are out-of-bounds.
    b = new Uint16Array(0);
    print('- zero length Uint16Array');
    print(b['-0']); b['-0'] = 123; print(b['-0']);
    print(b[0]); b[0] = 123; print(b[0]);
    print(b['0']); b['0'] = 234; print(b['0']);
}

try {
    print('write test');
    writeTest();
} catch (e) {
    print(e.stack || e);
}

/*===
[[HasProperty]] test
number 0 true
string -0 false
string -0.0 true
number 0 true
string 0 true
string 0.0 false
number 9 true
string 9 true
string 9.0 false
number 10 true
string 10 true
string 10.0 false
number 11 true
string 11 true
string 11.0 false
number 12 false
string 12 false
string 12.0 true
===*/

function hasTest() {
    var b = setupU16Array(true);

    // Also [[HasProperty]] must ignore inherited values for index
    // properties.
    //
    // The string '-0' is considered an index by ES6 CanonicalNumberIndexString
    // but the property is never considered to be present regardless of
    // inherited properties.

    [
        // -0: String(-0) === '0' so present
        // '-0': special treatment, never present despire inherited '0'
        // '-0.0': normal property, here inherited
        -0, '-0', '-0.0',
        0, '0', '0.0',
        9, '9', '9.0',
        10, '10', '10.0',
        11, '11', '11.0',
        12, '12', '12.0'   // '12.0' is present, inherited
    ].forEach(function (v) {
        print(typeof v, v, v in b);
    });
}

try {
    print('[[HasProperty]] test');
    hasTest();
} catch (e) {
    print(e.stack || e);
}

/*===
===*/

function fullTest() {
//    var plain = Duktape.Buffer(10);
    var u8 = new Uint8Array(10);
    var u32 = new Uint32Array(10);
    var f64 = new Float64Array(10);
    var node = new Buffer(10);
    var i;

    // Add some inherited properties.
    [
        ArrayBuffer.prototype,
        Uint8Array.prototype,
        Uint32Array.prototype,
        Float64Array.prototype,
        Buffer.prototype
    ].forEach(function (proto) {
        var i;

        function f1(idx) {
            Object.defineProperty(proto, idx, {
                value: 'dummy-' + idx,
                writable: true,
                enumerable: true,  // interfere with enumeration too
                configurable: true
            });
        }
        function f2(idx) {
            Object.defineProperty(proto, idx, {
                get: function () { print('GETTER CALLED:', idx); return 'dummy-getter'; },
                set: function (k) { print('SETTER CALLED:', idx); },
                enumerable: true,  // interfere with enumeration too
                configurable: true
            });
        }

        for (i = 0; i < 15; i++) {
            f1(i);
        }
        for (i = 15; i < 20; i++) {
            f2(i);
        }
        f2(0xfffffffe);   // valid index
        f2(0xffffffff);   // not a valid index
        f2(0x100000000);  // not a valid index, getter matches
        f2(0x10000000000000000);  // not a valid index, getter matches
        f2('1.0');  // not a valid index
    });

    [ /*plain,*/ u8, u32, f64, node ].forEach(function (b, bufidx) {
        print('*** Buffer value ' + bufidx);

        function test(idx) {
            print('=== index', idx, '(' + typeof idx + ')');
            print(idx, 'bufidx', idx in b);
            print(idx, 'bufstr', String(idx) in b);
            print(idx, 'bufidx', b[idx]);
            print(idx, 'bufstr', b[String(idx)]);
            print(idx, 'parent', Object.getPrototypeOf(Object(b))[idx]);
            b[idx] = 123;
            print(idx, b[idx]);
            print(idx, b[String(idx)]);
            print(idx, Object.getPrototypeOf(Object(b))[idx]);
        }
        for (var i = -1; i < 20; i++) {
            test(i);
        }

        // Test non-canonical number index; must be inherited.
        test('1.0');  // not valid, has ancestor
        test('2.0');  // not valid, but no ancestor

        // 4G corner cases, also test that index isn't incorrectly
        // ToUint32() (or otherwise integer) coerced in which case
        // it would wrap (incorrect).
        [ 0xfffffffe, 0xffffffff,
          0x100000000, 0x100000001,
          '4294967294', '4294967294.0',
          '4294967295', '4294967295.0',
          '4294967296', '4294967296.0',
          '4294967297', '4294967297.0',
          0x10000000000000000, 0x10000000000001111,  // step must be large enough to make a difference for an IEEE double
          '18446744073709552000', '18446744073709552000.0',
          '18446744073709556000', '18446744073709556000.0'
        ].forEach(function (idx) {
            test(idx);
        });

        // Enumeration: FIXME; inherit too?
        print('== for-in enumeration');
        for (var k in b) {
            print(k);
        }

        // FIXME: own props
    });
}

try {
    //fullTest();
} catch (e) {
    print(e.stack || e);
}
