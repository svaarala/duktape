/*
 *  Node.js Buffer toString()
 */

/*@include util-nodejs-buffer.js@*/

/*===
node.js Buffer toString() test
function
false
true
"ABC"
"ABC"
"ABC"
"DEFG"
"EFG"
"E"
"<U+CAFE>A"
"<U+D800><U+DFFF>"
"<Error>ABC"
"ABB<Error>"
"DEFG"
NONE NONE "DEFG"
NONE undefined "DEFG"
NONE null "DEFG"
NONE true "DEFG"
NONE false "DEFG"
NONE [object Object] "DEFG"
NONE [object Object] "DEFG"
NONE -1 "DEFG"
NONE 0 "DEFG"
NONE 1 "DEFG"
NONE 2 "DEFG"
NONE 3 "DEFG"
NONE 4 "DEFG"
NONE 5 "DEFG"
undefined NONE "DEFG"
undefined undefined "DEFG"
undefined null ""
undefined true "D"
undefined false ""
undefined [object Object] "D"
undefined [object Object] "DEF"
undefined -1 ""
undefined 0 ""
undefined 1 "D"
undefined 2 "DE"
undefined 3 "DEF"
undefined 4 "DEFG"
undefined 5 "DEFG"
null NONE "DEFG"
null undefined "DEFG"
null null ""
null true "D"
null false ""
null [object Object] "D"
null [object Object] "DEF"
null -1 ""
null 0 ""
null 1 "D"
null 2 "DE"
null 3 "DEF"
null 4 "DEFG"
null 5 "DEFG"
true NONE "EFG"
true undefined "EFG"
true null ""
true true ""
true false ""
true [object Object] ""
true [object Object] "EF"
true -1 ""
true 0 ""
true 1 ""
true 2 "E"
true 3 "EF"
true 4 "EFG"
true 5 "EFG"
false NONE "DEFG"
false undefined "DEFG"
false null ""
false true "D"
false false ""
false [object Object] "D"
false [object Object] "DEF"
false -1 ""
false 0 ""
false 1 "D"
false 2 "DE"
false 3 "DEF"
false 4 "DEFG"
false 5 "DEFG"
[object Object] NONE "EFG"
[object Object] undefined "EFG"
[object Object] null ""
[object Object] true ""
[object Object] false ""
[object Object] [object Object] ""
[object Object] [object Object] "EF"
[object Object] -1 ""
[object Object] 0 ""
[object Object] 1 ""
[object Object] 2 "E"
[object Object] 3 "EF"
[object Object] 4 "EFG"
[object Object] 5 "EFG"
[object Object] NONE "G"
[object Object] undefined "G"
[object Object] null ""
[object Object] true ""
[object Object] false ""
[object Object] [object Object] ""
[object Object] [object Object] ""
[object Object] -1 ""
[object Object] 0 ""
[object Object] 1 ""
[object Object] 2 ""
[object Object] 3 ""
[object Object] 4 "G"
[object Object] 5 "G"
-1 NONE "DEFG"
-1 undefined "DEFG"
-1 null ""
-1 true "D"
-1 false ""
-1 [object Object] "D"
-1 [object Object] "DEF"
-1 -1 ""
-1 0 ""
-1 1 "D"
-1 2 "DE"
-1 3 "DEF"
-1 4 "DEFG"
-1 5 "DEFG"
0 NONE "DEFG"
0 undefined "DEFG"
0 null ""
0 true "D"
0 false ""
0 [object Object] "D"
0 [object Object] "DEF"
0 -1 ""
0 0 ""
0 1 "D"
0 2 "DE"
0 3 "DEF"
0 4 "DEFG"
0 5 "DEFG"
1 NONE "EFG"
1 undefined "EFG"
1 null ""
1 true ""
1 false ""
1 [object Object] ""
1 [object Object] "EF"
1 -1 ""
1 0 ""
1 1 ""
1 2 "E"
1 3 "EF"
1 4 "EFG"
1 5 "EFG"
2 NONE "FG"
2 undefined "FG"
2 null ""
2 true ""
2 false ""
2 [object Object] ""
2 [object Object] "F"
2 -1 ""
2 0 ""
2 1 ""
2 2 ""
2 3 "F"
2 4 "FG"
2 5 "FG"
3 NONE "G"
3 undefined "G"
3 null ""
3 true ""
3 false ""
3 [object Object] ""
3 [object Object] ""
3 -1 ""
3 0 ""
3 1 ""
3 2 ""
3 3 ""
3 4 "G"
3 5 "G"
4 NONE ""
4 undefined ""
4 null ""
4 true ""
4 false ""
4 [object Object] ""
4 [object Object] ""
4 -1 ""
4 0 ""
4 1 ""
4 2 ""
4 3 ""
4 4 ""
4 5 ""
5 NONE ""
5 undefined ""
5 null ""
5 true ""
5 false ""
5 [object Object] ""
5 [object Object] ""
5 -1 ""
5 0 ""
5 1 ""
5 2 ""
5 3 ""
5 4 ""
5 5 ""
===*/

function nodejsBufferToStringTest() {
    var b, s;

    // Check inheritance; not inherited from Object.prototype
    print(typeof Buffer.prototype.toString);
    print(Buffer.prototype.toString === Object.prototype.valueOf);
    print(Buffer.prototype.hasOwnProperty('toString'));

    // buf.toString([encoding], [start], [end])

    // Without arguments encoding defaults to UTF-8 and the entire
    // buffer is converted to string.  At least undefined and null
    // are accepted as "not defined" for encoding.
    b = new Buffer('ABC');
    safePrint(b.toString());
    safePrint(b.toString(undefined));
    safePrint(b.toString(null));

    // If the buffer is a slice of an underlying buffer, only that slice
    // is string converted.  Offsets are relative to the slice.
    b = new Buffer('ABCDEFGH');
    b = b.slice(3, 7);  // DEFG
    safePrint(b.toString());
    safePrint(b.toString(null, 1));
    safePrint(b.toString(null, 1, 2));

    // When the buffer data is legal UTF-8 and the chosen encoding
    // is UTF-8 (default), Duktape internal representation is correct
    // as is.  Here the 4-byte data is U+CAFE U+0041.
    b = new Buffer(4);
    b[0] = 0xec; b[1] = 0xab; b[2] = 0xbe; b[3] = 0x41;
    safePrint(b.toString());

    // When the buffer data is not legal UTF-8 Node.js behavior is
    // interesting and a bit inconsistent with other API calls:
    // CESU-8 encoded strings are accepted as is.  Here U+D800 U+DFFF
    // encoded as CESU-8 is ED A0 80 ED BF BF and the string that comes
    // back is U+D800 U+DFFF with both Node.js and Duktape.
    b = new Buffer(6);
    b[0] = 0xed; b[1] = 0xa0; b[2] = 0x80;
    b[3] = 0xed; b[4] = 0xbf; b[5] = 0xbf;
    safePrint(b.toString());

    // Here the buffer data is invalid UTF-8 and invalid CESU-8.
    // Node.js replaces the offending character (0xff) with U+FFFD
    // (replacement character).  Duktape currently allows as is.

    // XXX: current output is odd because charCodeAt() doesn't
    // work well now for invalid strings.

    b = new Buffer(4);
    b[0] = 0xff; b[1] = 0x41; b[2] = 0x42; b[3] = 0x43;
    safePrint(b.toString());

    // Invalid continuation characters.  Node.js seems to scan for
    // the next valid starting byte and each offending byte causes
    // a new U+FFFD to be emitted (here U+FFFD U+FFFD U+0042 U+FFFD).
    // Duktape currently allows the bytes as is.

    // XXX: current output is odd because charCodeAt() doesn't
    // work well now for invalid strings.

    b = new Buffer(4);
    b[0] = 0xc1; b[1] = 0xc1; b[2] = 0x42; b[3] = 0xc1;
    safePrint(b.toString());

    // XXX: encoding test?

    // Offsets, very lenient, something like:
    //     - Non-numbers coerce to 0 (no valueOf() etc consulted)
    //     - Negative starting point => empty result, regardless of 'end'
    //     - Valid starting point but 'end' out of bounds => clamp to end
    //     - Crossed indices => empty result
    //
    // Duktape behavior is a bit more lenient: everything is ToInteger()
    // coerced, clamped to valid range, and crossed indices are allowed.
    // Testcase expect string has been corrected for this.
    //
    // Offsets are relative to a slice (if buffer is a slice).

    b = new Buffer('ABCDEFGH');
    b = b.slice(3, 7);  // DEFG
    safePrint(b.toString());

    var offsetList = [
        'NONE',
        undefined,
        null,
        true,
        false,
        { valueOf: function () { return 1; } },
        { valueOf: function () { return 3; } },
        -1, 0, 1, 2, 3, 4, 5
    ]

    offsetList.forEach(function (start) {
        offsetList.forEach(function (end) {
            try {
                if (start === 'NONE') {
                    s = b.toString('utf8');
                } else if (end === 'NONE') {
                    s = b.toString('utf8', start);
                } else {
                    s = b.toString('utf8', start, end);
                }
                print(start, end, safeEscape(s));
            } catch (e) {
                print(start, end, e.name);
            }
        });
    });
}

try {
    print('node.js Buffer toString() test');
    nodejsBufferToStringTest();
} catch (e) {
    print(e.stack || e);
}
