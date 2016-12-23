/*
 *  TextEncoder
 */

/*@include util-buffer.js@*/

// determine if two arrays contain the same values, used here to compare buffers
function equiv(arr1, arr2) {
    if (Object.getPrototypeOf(arr1) !== Object.getPrototypeOf(arr2)) {
        return false;
    }
    if (arr1.length !== arr2.length) {
        return false;
    }
    for (i = 0; i < arr1.length; ++i) {
        if (arr1[i] !== arr2[i]) {
            return false;
        }
    }
    return true;
}

/*===
basic encode
utf-8
true
4
true
10
true
6
true
0
true
===*/

function encodeTest() {
    var encoder;
    var ref, input, output;

    encoder = new TextEncoder();
    print(encoder.encoding);  // always 'utf-8'

    // non-BMP surrogate pair should become a single codepoint
    input = '\ud83d\udc37';
    ref = new Uint8Array([ 0xf0, 0x9f, 0x90, 0xb7 ]);
    output = encoder.encode(input);
    print(output instanceof Uint8Array);
    print(output.length);  // should be 4
    print(equiv(output, ref));

    // unpaired surrogate halves should become U+FFFD
    input = '\ud83dABCD\udc37';
    ref = new Uint8Array([ 0xef, 0xbf, 0xbd, 0x41, 0x42, 0x43, 0x44, 0xef, 0xbf, 0xbd ]);
    output = encoder.encode(input);
    print(output.length);
    print(equiv(output, ref));

    // consecutive high surrogates should not be combined
    input = '\ud83d\ud83d';
    ref = new Uint8Array([ 0xef, 0xbf, 0xbd, 0xef, 0xbf, 0xbd ]);
    output = encoder.encode(input);
    print(output.length);
    print(equiv(output, ref));

    // no argument is the same as an empty string
    ref = new Uint8Array(0);
    output = encoder.encode();
    print(output.length);
    print(equiv(output, ref));

}

try {
    print('basic encode');
    encodeTest();
} catch (e) {
    print(e.stack || e);
}

/*===
argument coercion and validation
0 [object Undefined] 0 ||
1 [object Null] 4 |6e756c6c|
2 [object Boolean] 4 |74727565|
3 [object Boolean] 5 |66616c7365|
4 [object Number] 3 |313233|
5 [object String] 5 |64756d6d79|
6 [object String] 5 |64756d6d79|
7 [object Number] 3 |313233|
8 [object Uint8Array] 19 |5b6f626a6563742055696e743841727261795d|
9 [object ArrayBuffer] 20 |5b6f626a6563742041727261794275666665725d|
10 [object Float32Array] 21 |5b6f626a65637420466c6f6174333241727261795d|
11 [object Uint8Array] 10 |00000000000000000000|
12 [object Array] 5 |312c322c33|
13 [object Object] 15 |5b6f626a656374204f626a6563745d|
===*/

function argumentTest() {
    [
        undefined,
        null,
        true,
        false,
        123,
        'dummy',
        new String('dummy'),
        new Number(123),
        createPlainBuffer(10),
        // skip function, ToString() output is fragile
        new ArrayBuffer(10),
        new Float32Array(10),
        new Buffer(10),
        [ 1, 2, 3 ],
        { foo: true }
    ].forEach(function (v, i) {
        try {
            var res = new TextEncoder().encode(v);
            print(i, Object.prototype.toString.call(v), res.length, Duktape.enc('jx', res));
        } catch (e) {
            print(i, Object.prototype.toString.call(v), e.name);
        }
    });
}

try {
    print('argument coercion and validation');
    argumentTest();
} catch (e) {
    print(e.stack || e);
}
