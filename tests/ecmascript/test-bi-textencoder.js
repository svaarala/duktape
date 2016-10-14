/*
 *  TextEncoder
 */

// determine if two arrays contain the same values, used here to compare buffers
function equiv(arr1, arr2) {
    if (arr1.prototype !== arr2.prototype) {
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
    print(encoder.encoding);  // always "utf-8"

    // non-BMP surrogate pair should become a single codepoint
    input = "\ud83d\udc37";
    ref = new Uint8Array([ 0xf0, 0x9f, 0x90, 0xb7 ]);
    output = encoder.encode(input);
    print(output instanceof Uint8Array);
    print(output.length);  // should be 4
    print(equiv(output, ref));

    // unpaired surrogate halves should become U+FFFD
    input = "\ud83dABCD\udc37";
    ref = new Uint8Array([ 0xef, 0xbf, 0xbd, 0x41, 0x42, 0x43, 0x44, 0xef, 0xbf, 0xbd ]);
    output = encoder.encode(input);
    print(output.length);
    print(equiv(output, ref));

    // consecutive high surrogates should not be combined
    input = "\ud83d\ud83d";
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
    print("basic encode");
    encodeTest();
} catch (e) {
    print(e.stack || e);
}
