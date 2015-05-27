/*
 *  Virtual properties for plain buffers and buffer objects.
 */

/*---
{
    "custom": true
}
---*/

/*===
read non-strict
buffer
length 4
222 222
173 173
190 190
239 239
0 222 222
1 173 173
2 190 190
3 239 239
object
length 4
222 222
173 173
190 190
239 239
0 222 222
1 173 173
2 190 190
3 239 239
===*/

function readTestNonStrict() {
    var buf_plain, buf_object;
    var i;

    buf_plain = Duktape.dec('hex', 'deadbeef');
    buf_object = new Duktape.Buffer(Duktape.dec('hex', 'deadbeef'));

    print(typeof buf_plain);
    print('length', buf_plain.length);
    for (i = 0; i < buf_plain.length; i++) {
        print(buf_plain[i], buf_plain[String(i)]);
    }
    for (i in buf_plain) {
        print(i, buf_plain[i], buf_plain[String(i)]);
    }

    print(typeof buf_object);
    print('length', buf_object.length);
    for (i = 0; i < buf_object.length; i++) {
        print(buf_object[i], buf_object[String(i)]);
    }
    for (i in buf_object) {
        print(i, buf_object[i], buf_object[String(i)]);
    }
}

print('read non-strict');

try {
    readTestNonStrict();
} catch (e) {
    print(e);
}

/*===
write non-strict
65
65
4 AAAD
65
65
4 AAAD
69
69
4 AAAD
69
69
4 AAAD
true 69
true 69
0 254 254
1 69 69
2 66 66
3 253 253
4 0 0
5 103 103
6 1 1
7 0 0
8 73 73
9 65 65
10 0 0
11 0 0
n/a true [object Object] marker
===*/

/*===
write strict
TypeError
4 ABCD
69
4 ABCD
undefined
69
===*/

function writeTestNonStrict() {
    var buf_plain, buf_object;
    var i;

    buf_plain = Duktape.dec('hex', '41424344');  // ABCD
    buf_object = new Duktape.Buffer(Duktape.dec('hex', '41424344'));

    print(buf_plain[1] = 0x41);     // AACD, assignment result printed
    print(buf_plain['2'] = 0x41);   // AAAD
    print(buf_plain.length, String(buf_plain));

    print(buf_object[1] = 0x41);    // AACD, assignment result printed
    print(buf_object['2'] = 0x41);  // AAAD
    print(buf_object.length, String(buf_object));

    // Non-strict mode: write out of bounds is a silent failure for
    // plain buffers.  For buffer objects, the write becomes a normal
    // property.

    print(buf_plain[4] = 0x45);    // out of buffer, no autoextend now -> ignored
    print(buf_plain['5'] = 0x45);  // same -> ignored
    print(buf_plain.length, String(buf_plain));

    print(buf_object[4] = 0x45);
    print(buf_object['5'] = 0x45);
    print(buf_object.length, String(buf_object));
    print('4' in buf_object, buf_object['4']);  // stored as a normal property
    print('5' in buf_object, buf_object['5']);  // stored as a normal property

    // Values written are coerced with ToNumber, coerced to integer,
    // then bit masked with 0xff from their 2's complement representation.

    buf_plain = Duktape.dec('hex', '4142434445464748494a4b4c');  // ABCDEFGHIJKL
    buf_object = new Duktape.Buffer(Duktape.dec('hex', '4142434445464748494a4b4c'));

    buf_plain[0] = -2;     // -> 0xfe
    buf_plain[1] = 0x145;  // 0x45 -> E
    buf_plain[2] = 66.7;   // 66.7 -> 66 = 0x42 = B
    buf_plain[3] = -259.9; // -259.9 -> -259 -> 0xfffffefd -> 0xfd
    buf_plain[4] = 'x';    // Intuitively this would be codepoint of 'x',
                           // but currently coerces with ToNumber('x'} -> 0
    buf_plain[5] = '0x67'; // number parsing, part of ToNumber()
    buf_plain[6] = true;   // true coerces to 1
    buf_plain[7] = false;  // false coerces to 0
    // index is coerced with ToString() and becomes '[object Object]' and thus ignored
    buf_plain[{ valueOf: function() { return 8; } }] = { valueOf: function() { return 0x41; } };
    // here index works
    buf_plain[{ toString: function() { return '9'; } }] = { valueOf: function() { return 0x41; } };
    buf_plain[10] = -1/0;  // negative infinity, coerces to 0x00
    buf_plain[11] = 1/0;   // positive infinity, coerces to 0x00

    buf_object[0] = -2;
    buf_object[1] = 0x145;
    buf_object[2] = 66.7;
    buf_object[3] = -3.9;
    buf_object[4] = 'x';
    buf_object[5] = '0x67';
    buf_object[6] = true;
    buf_object[7] = false;
    // index is coerced with ToString(), and will go to '[object Object]', value will
    // not be coerced
    buf_object[{ valueOf: function() { return 8; } }] = { valueOf: function() { return 0x41; }, marker: 'marker' };
    // here index works
    buf_object[{ toString: function() { return '9'; } }] = { valueOf: function() { return 0x41; } };
    buf_object[10] = -1/0;
    buf_object[11] = 1/0;

    for (i = 0; i < buf_plain.length; i++) {
        print(i, buf_plain[i], buf_object[i]);
    }
    print('n/a', '[object Object]' in buf_object, buf_object['[object Object]'], buf_object['[object Object]'].marker);
}

function writeTestStrict() {
    'use strict';
    var buf_plain, buf_object;
    var i;

    // Some specific tests in strict mode

    buf_plain = Duktape.dec('hex', '41424344');  // ABCD
    buf_object = new Duktape.Buffer(Duktape.dec('hex', '41424344'));

    // Write to outside plain buffer range is a TypeError, not silent as in
    // non-strict mode.  Write to outside buffer object range is OK, because
    // it becomes a new valid property of the object.

    try {
        print(buf_plain[4] = 0x45);
    } catch (e) {
        print(e.name);
    }
    print(buf_plain.length, String(buf_plain));

    try {
        print(buf_object[4] = 0x45);
    } catch (e) {
        print(e.name);
    }
    print(buf_object.length, String(buf_object));
    print(buf_plain['4']);   // not stored -> undefined
    print(buf_object['4']);  // stored as a normal property
}

print('write non-strict');

try {
    writeTestNonStrict();
} catch (e) {
    print(e);
}

print('write strict');

try {
    writeTestStrict();
} catch (e) {
    print(e);
}

/*===
defineProperty
3 102 111 111 undefined
TypeError
3 102 111 111 undefined
TypeError
3 102 111 111 undefined
TypeError
3 102 111 111 undefined
3 102 111 111 undefined
===*/

/* There is currently no full support for buffer virtual properties in defineProperty().
 * This testcase illustrates the current (not as such desirable) behavior.
 */

function definePropertyTest() {
    var b;

    // Plain buffer: TypeError (defineProperty normal behavior).

    try {
        b = Duktape.Buffer('foo');
        print(b.length, b[0], b[1], b[2], b[3]);

        Object.defineProperty(b, 'length', { value: 5 });
        print(b.length, b[0], b[1], b[2], b[3]);
    } catch (e) {
        print(e.name);
    }

    // Buffer Object: 'length' is not configurable, so TypeError.

    try {
        b = new Duktape.Buffer('foo');
        print(b.length, b[0], b[1], b[2], b[3]);

        Object.defineProperty(b, 'length', { value: 5 });
        print(b.length, b[0], b[1], b[2], b[3]);
    } catch (e) {
        print(e.name);
    }

    // Buffer Object: virtual properties are writable but defineProperty()
    // will detect them as being virtual and reject any changes at the moment.

    try {
        b = new Duktape.Buffer('foo');
        print(b.length, b[0], b[1], b[2], b[3]);

        Object.defineProperty(b, '0', { value: 0x45 });
        print(b.length, b[0], b[1], b[2], b[3]);
    } catch (e) {
        print(e.name);
    }

    // Buffer Object: attempt to write the *same value* will be successful.

    try {
        b = new Duktape.Buffer('foo');
        print(b.length, b[0], b[1], b[2], b[3]);

        Object.defineProperty(b, '0', { value: 102 });
        print(b.length, b[0], b[1], b[2], b[3]);
    } catch (e) {
        print(e.name);
    }
}

print('defineProperty');

try {
    definePropertyTest();
} catch (e) {
    print(e);
}
