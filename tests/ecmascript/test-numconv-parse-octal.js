/*---
{
    "custom": true
}
---*/

/*===
63
SyntaxError
SyntaxError
SyntaxError
SyntaxError
63
SyntaxError
SyntaxError
63
0
0
7
63
63
NaN
NaN
===*/

/* FIXME: what's the expected output?  Currently SyntaxError for invalid octal
 * in eval().
 */

function octalTest() {
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
     *  Technically a leading zero digit indicates octal and if the
     *  number doesn't comply with octal syntax, a SyntaxError should
     *  happen: a valid numeric literal cannot be followed by a digit,
     *  after "longest match" semantics are applied.
     *
     *  In practice, at least V8 and Rhino will parse offending octal
     *  literals in decimal.
     */

    e('077');
    e('088');
    e('099');
    e('0789');
    e('07789');
    e('00077');  // leading zeroes are typically allowed for octals

    // For comparison: hex prefix without digits -> SyntaxError.  One
    // could also interpret this as an octal number (0) followed by an 'x'.
    e('0x');

    // For comparison: hex prefix with an invalid first digit -> SyntaxError.
    e('0xg');

    /*
     *  Technically, there is no octal syntax for parseInt().
     *
     *  In practice, at least V8 and Rhino will detect octals in parseInt().
     *  They will stop parsing at a non-octal digit, but will parse the
     *  longest valid prefix as octal (as expected).  However, Rhino will
     *  return a NaN if the offending digit follows the octal leading zero
     *  immediately; V8 will return a 0 in such a case.
     */

    pI('077');
    pI('088');    // V8: 0, Rhino: NaN
    pI('099');    // V8: 0, Rhino: NaN
    pI('0789');   // V8 and Rhino: 7
    pI('07789');  // V8 and Rhino: 63
    pI('00077');

    // For comparison: hex prefix without digits -> NaN.
    pI('0x');

    // For comparison: hex prefix with an invalid first digit -> NaN.
    pI('0xg');

}

try {
    octalTest();
} catch (e) {
    print(e);
}
