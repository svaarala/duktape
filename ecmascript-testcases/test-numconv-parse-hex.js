/*===
57005
3735928559
3735928559
SyntaxError
SyntaxError
57005
3735928559
3735928559
NaN
NaN
===*/

function hexTest() {
    function e(x) {
        try {
            print(eval(x));
        } catch (e) {
            print(e.name);
        }
    }

    function pI(x) {
        try {
            print(parseInt(x));
        } catch (e) {
            print(e.name);
        }
    }

    /*
     *  Hex support in compiler literals.
     */

    e('0xdead');
    e('0Xdeadbeef');
    e('0x0000deadbeef');  // leading zeroes allowed

    // Hex prefix without digits -> SyntaxError.  One could also interpret
    // this as an octal number (0) followed by an 'x'.  (In fact, with the
    // interpretation that one should parse the longest valid prefix, this
    // might be the correct interpretation; a SyntaxError would still happen
    // when 'x' was parsed.)

    e('0x');  // Rhino will parse this as a NaN, not SyntaxError

    // Hex prefix with an invalid first digit -> SyntaxError.
    e('0xg');

    /*
     *  Hex support in parseInt().
     */

    pI('0xdead');
    pI('0XDEADBEEF');
    pI('0x0000deadbeef');

    // Here '0x' triggers hex mode and parse fails, so NaN is expected.

    pI('0x');
    pI('0xg');
}

try {
    hexTest();
} catch (e) {
    print(e);
}
