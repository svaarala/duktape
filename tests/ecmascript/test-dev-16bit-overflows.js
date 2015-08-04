/*
 *  Test for handling of size overflows when using 16-bit field sizes for
 *  string and buffer lengths, etc.
 *
 *  To be able to test the mechanisms, you need to have 16-bit fields enabled
 *  but have enough memory so that large enough intermediate values can be
 *  created.  Heap pointer compression, for instance, may prevent testing.
 *  Specific tests here also need different options, e.g. to test property
 *  64k limit you must define OBJSIZES16 but not BUFLEN16 (see discussion
 *  below).
 *
 *  For now, this must be executed and the output inspected manually.  The
 *  "skip" metadata property is set because of this.
 */

/*---
{
    "custom": true,
    "skip": true
}
---*/

// Make string of N bytes (must be 2^x)
function makeString(n) {
    var t = 'x';
    while (t.length < n) {
        t = t + t;
    }
    return t;
}

// Test string byte length overflow.  There's no point in a char length
// overflow test because the byte length will always overflow first.
function stringConcatByteLenTest() {
    var s = makeString(32768);
    var t = s.substring(0, 32767);
    print(s.length);
    print(t.length);
    var justok = s + t;    // 65535 exactly
    print(justok.length);
    var overflow = s + s;  // 65536 exactly
    print(overflow.length);
}

function bufferConcatLenTest() {
    // Buffer concatenation currently results in a string so this test is
    // not very useful now.

    var s = makeString(32768);
    var t = s.substring(0, 32767);
    s = Duktape.Buffer(s);
    t = Duktape.Buffer(t);
    print(s.length);
    print(t.length);
    var justok = s + t;    // 65535 exactly
    print(justok.length);
    var overflow = s + s;  // 65536 exactly
    print(overflow.length);
}

function bufferConstructTest() {
    var b;

    b = new Duktape.Buffer(65535);  // ok
    print(b.length);
    b = new Duktape.Buffer(65536);  // overflow
    print(b.length);
}

var reached = 0;

function objectPropertyLimitTest() {
    var i, n;
    var obj = {};

    // The 65536 property limit is never triggered when BUFLEN16 is used,
    // because the object property table and the string table allocations
    // will trigger the buffer size limit way before we have 65536 properties
    // (the buffer limits are triggered around ~2300 properties on x64 and
    // ~4800 properties on x86).
    //
    // To test the object property limit, define OBJSIZES16 but don't define
    // BUFLEN16.  Even in this case the property limit is triggered before
    // 65536 because the "spare" allocated during a property table resize is
    // counted towards the limit.  (Right now the highest property count
    // reached is 64231 but that limit depends on tuning.)

    for (i = 0; i < 65536; i++) {
        reached = i;
        obj['str-' + i] = true;
    }
}

try {
    stringConcatByteLenTest();
} catch (e) {
    print(e);
}

try {
    bufferConcatLenTest();
} catch (e) {
    print(e);
}

try {
    bufferConstructTest();
} catch (e) {
    print(e);
}

try {
    objectPropertyLimitTest();
} catch (e) {
    print(e);
}

print('Reached: ' + reached);
