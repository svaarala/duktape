/*===
===*/

/* FIXME: differences in support, match with documentation */

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
}

try {
    octalTest();
} catch (e) {
    print(e);
}

