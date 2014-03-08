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
222
173
190
239
0 222
1 173
2 190
3 239
object
length 4
222
173
190
239
0 222
1 173
2 190
3 239
===*/

function readTestNonStrict() {
    var buf_plain, buf_object;
    var i;

    buf_plain = Duktape.dec('hex', 'deadbeef');
    buf_object = new Duktape.Buffer(Duktape.dec('hex', 'deadbeef'));

    print(typeof buf_plain);
    print('length', buf_plain.length);
    for (i = 0; i < buf_plain.length; i++) {
        print(buf_plain[i]);
    }
    for (i in buf_plain) {
        print(i, buf_plain[i]);
    }

    print(typeof buf_object);
    print('length', buf_object.length);
    for (i = 0; i < buf_object.length; i++) {
        print(buf_object[i]);
    }
    for (i in buf_object) {
        print(i, buf_object[i]);
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
4 AACD
65
4 AACD
69
4 AACD
69
4 AACD
true 69
write strict
TypeError
4 ABCD
69
4 ABCD
true 69
===*/

function writeTestNonStrict() {
    var buf_plain, buf_object;
    var i;

    buf_plain = Duktape.dec('hex', '41424344');  // ABCD
    buf_object = new Duktape.Buffer(Duktape.dec('hex', '41424344'));

    print(buf_plain[1] = 0x41);  // AACD, assignment result printed
    print(buf_plain.length, String(buf_plain));

    print(buf_object[1] = 0x41);  // AACD, assignment result printed
    print(buf_object.length, String(buf_object));

    // Non-strict mode: write out of bounds is a silent failure for
    // plain buffers.  For buffer objects, the write becomes a normal
    // property.

    print(buf_plain[4] = 0x45);  // out of buffer, no autoextend now
    print(buf_plain.length, String(buf_plain));

    print(buf_object[4] = 0x45);
    print(buf_object.length, String(buf_object));
    print('4' in buf_object, buf_object['4']);  // stored as a normal property
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
    print('4' in buf_object, buf_object['4']);  // stored as a normal property
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
