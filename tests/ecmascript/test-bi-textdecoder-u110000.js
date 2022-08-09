/*===
65
65533
65533
65533
65533
===*/

function test() {
    // U+10FFFF encodes to F4 8F BF BF and is the highest allowed UTF-8 codepoint.
    //
    // U+110000 encodes (loosely) to F4 90 80 80.  Here F4 is a valid byte but 90
    // is broken because the output codepoint would necessarily be above U+10FFFF.
    // So F4 gets replaced with a U+FFFD.  We then reconsider 90, emit a U+FFFD,
    // and same for each 80 individually.  The result is 4 U+FFFD replacements.
    var t = new TextDecoder().decode(new Uint8Array([ 0x41, 0xf4, 0x90, 0x80, 0x80 ]));
    for (i = 0; i < t.length; i++) {
        print(t.charCodeAt(i));
    }
}

test();
