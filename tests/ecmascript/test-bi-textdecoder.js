/*
 *  TextDecoder
 */

/*===
basic decode
utf-8
false
false
2
55357 56375
true
5
65533 65 66 67 68
true
0
true
===*/

function decodeTest() {
    var decoder;
    var ref, input, output;

    decoder = new TextDecoder();  // defaults to UTF-8
    print(decoder.encoding);
    print(decoder.fatal);
    print(decoder.ignoreBOM);

    // non-BMP codepoint should become a surrogate pair
    input = new Uint8Array([ 0xf0, 0x9f, 0x90, 0xb7 ]);
    ref = "\ud83d\udc37";
    output = decoder.decode(input);
    print(output.length);  // should be 2
    print(output.charCodeAt(0), output.charCodeAt(1));
    print(output === ref);

    // incomplete codepoint should not mask characters
    input = new Uint8Array([ 0xf0, 0x41, 0x42, 0x43, 0x44 ]);
    ref = "\ufffdABCD";
    output = decoder.decode(input);
    print(output.length);
    print(output.charCodeAt(0), output.charCodeAt(1), output.charCodeAt(2), output.charCodeAt(3), output.charCodeAt(4));
    print(output === ref);

    // no argument is the same as an empty buffer
    ref = "";
    output = decoder.decode();
    print(output.length);
    print(output === ref);

}

try {
    print("basic decode");
    decodeTest();
} catch (e) {
    print(e.stack || e);
}

/*===
fatal mode
true
false
foo
true
TypeError
===*/

function fatalTest() {
    var fatal, nonFatal;
    var ref, input, output;

    fatal = new TextDecoder('utf-8', { fatal: true });
    print(fatal.fatal);
    nonFatal = new TextDecoder('utf-8', { fatal: false });
    print(nonFatal.fatal);

    // well-formed input, should not throw
    input = new Uint8Array([ 0x66, 0x6f, 0x6f ]);
    ref = "foo";
    output = fatal.decode(input);
    print(output);

    // buffer contains invalid UTF-8 code unit 0xFF
    input = new Uint8Array([ 0x62, 0x61, 0x72, 0xff ]);
    ref = "bar\ufffd";
    print(nonFatal.decode(input) === ref);
    try {
        print(fatal.decode(input));
    } catch (e) {
        print(e.name);
    }
}

try {
    print("fatal mode");
    fatalTest();
} catch (e) {
    print(e.stack || e);
}

/*===
ignore BOM
false
true
true
true
===*/

function ignoreBOMTest() {
    var stripBOM, keepBOM;
    var ref, input, output;

    var stripBOM = new TextDecoder('utf-8', { ignoreBOM: false });
    print(stripBOM.ignoreBOM);
    var keepBOM = new TextDecoder('utf-8', { ignoreBOM: true });
    print(keepBOM.ignoreBOM);

    input = new Uint8Array([ 0xef, 0xbb, 0xbf, 0x42, 0x4f, 0x4d ]);
    ref = "\ufeffBOM";
    output = keepBOM.decode(input);
    print(output === ref);
    ref = "BOM";
    output = stripBOM.decode(input);
    print(output === ref);
}

try {
    print("ignore BOM");
    ignoreBOMTest();
} catch (e) {
    print(e.stack || e);
}

/*===
streaming
true
true
===*/

function streamTest()
{
    var decoder;
    var ref, input, output;

    // include a multi-byte character to test streaming correctness
    decoder = new TextDecoder();
    ref = "pig \ud83d\udc37";
    input = new Uint8Array([ 0x70, 0x69, 0x67, 0x20, 0xf0, 0x9f, 0x90, 0xb7 ]);
    output = "";
    for (var i = 0; i < input.length; ++i) {
        var byte = input.subarray(i, i + 1);
        output += decoder.decode(byte, { stream: true });
    }
    output += decoder.decode();
    print(output === ref);

    // incomplete codepoint at end of string is caught in final decode().
    // note that it's safe to reuse the decoder instance for another go as long
    // as the last call to decode() was with {stream:false} (the default).
    ref = "pig \ud83d\udc37\ufffd";
    input = new Uint8Array([ 0x70, 0x69, 0x67, 0x20, 0xf0, 0x9f, 0x90, 0xb7, 0xf0 ]);
    output = "";
    for (var i = 0; i < input.length; ++i) {
        var byte = input.subarray(i, i + 1);
        output += decoder.decode(byte, { stream: true });
    }
    output += decoder.decode();
    print(output === ref);
}

try {
    print("streaming");
    streamTest();
} catch (e) {
    print(e.stack || e);
}
