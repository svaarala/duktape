// Keepcheck has an ASCII fast path which drops out to a non-ASCII path as needed.
// Cover various scenarios in an attempt to achieve code coverage for the various
// paths.

/*@include util-buffer.js@*/

/*---
custom: true
---*/

/*===
0
1000
2000
3000
4000
5000
6000
7000
8000
9000
0
1000
2000
3000
4000
5000
6000
7000
8000
9000
done
===*/

function createRandomStringPart() {
    var plen;
    var t = '';
    if (Math.random() >= 0.50) {
        plen = Math.random() * 100;
        while (t.length < plen) {
            t += String.fromCharCode(Math.floor(Math.random() * 0x80));
        }
    } else {
        plen = Math.random() * 20;
        while (t.length < plen) {
            t += String.fromCharCode(Math.floor(Math.random() * 0x10000));
        }
    }
    return t;
}

// Test a random string by building it up from blocks of ASCII and non-ASCII
// codepoints.
function testRandomString() {
    var s = [];
    var parts = Math.floor(Math.random() * 100);

    for (var i = 0; i < parts; i++) {
        s.push(createRandomStringPart());
    }

    return s.concat();
}

function testRandomBuffer() {
    var blen = Math.floor(Math.random() * 10000);
    var b = new Uint8Array(blen);

    for (var i = 0; i < blen; i++) {
        if (Math.random() >= 0.10) {
            b[i] = Math.floor(Math.random() * 0x80);
        } else {
            b[i] = Math.floor(Math.random() * 0x100);
        }
    }

    var s = new TextDecoder().decode(b);
    return s;
}

// Test a random string by building it up from blocks of ASCII, non-ASCII, and
// invalid codepoint data.

function test() {
    var i;

    for (i = 0; i < 10000; i++) {
        if ((i % 1000) == 0) {
            print(i);
        }
        var t = testRandomString();
        //print(Duktape.enc('jx', t));
    }

    for (i = 0; i < 10000; i++) {
        if ((i % 1000) == 0) {
            print(i);
        }
        var t = testRandomBuffer();
        //print(Duktape.enc('jx', t));
    }

    print('done');
}

test();
