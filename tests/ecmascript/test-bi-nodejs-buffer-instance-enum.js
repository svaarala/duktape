/*
 *  Enumerating a Buffer, keys of a Buffer.
 */

/*@include util-buffer.js@*/

/*===
enumerating and keys test
for-in
string 0
string 1
string 2
string 3
string 4
string 5
string 6
string 7
string 8
string 9
Object.keys
string 0
string 1
string 2
string 3
string 4
string 5
string 6
string 7
string 8
string 9
Object.getOwnPropertyNames
string 0
string 1
string 2
string 3
string 4
string 5
string 6
string 7
string 8
string 9
string length
===*/

function enumeratingAndKeysTest() {
    var b = new Buffer(10);
    var k;

    b.fill(0x12);

    // Node.js v6.9.1 enumerates index keys, 'buffer', 'parent', and all
    // Buffer.prototype methods (they're enumerable).
    //
    // In Duktape the Buffer.prototype methods are not enumerable as that's
    // more in line with other built-ins.

    print('for-in');
    for (k in b) {
        print(typeof k, k);
    }

    print('Object.keys');
    Object.keys(b).forEach(function (k) {
        print(typeof k, k);
    });

    // In ES2015 (and Node.js) .byteOffset, .buffer etc are inherited accessors and
    // not shown here.  Also .length should be an inherited getter, but is
    // currently a virtual own property and shows up here.

    print('Object.getOwnPropertyNames');
    Object.getOwnPropertyNames(b).forEach(function (k) {
        print(typeof k, k);
    });
}

try {
    print('enumerating and keys test');
    enumeratingAndKeysTest();
} catch (e) {
    print(e.stack || e);
}
