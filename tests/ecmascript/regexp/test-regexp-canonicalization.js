/*
 *  Regexp Canonicalization operation related behavior, including CharacterSetMatcher
 *  behavior.
 *
 *  Particular issues include:
 *    - special rules for canonicalization (multiple code point results,
 *      non-ASCII-to-ASCII results)
 *    - continuous character ranges converted into non-continuous ranges
 *      after canonicalization
 */

var re;
var i;
var t;
var allowed;

/*
 *  Test the property of built-in character ranges mentioned in
 *  doc/regular-expressions.txt: if a character x is contained
 *  in the range, its normalized counterpart is also contained
 *  in the range.
 *
 *  This is not a test of the implementation, but a general sanity
 *  check.
 */

/*
 *  A continuous character range is not necessarily continuous after
 *  canonicalization, which the matcher has to handle correctly.
 */

/*===
match: 88
match: 89
match: 90
match: 120
match: 121
match: 122
match: 123
===*/

/* [x-{] consists (naively) of 'x', 'y', 'z', '{', a continuous range:
 *
 *     U+0078-U+007B
 *
 * Canonicalized (uppercased) range matches 'X', 'Y', 'Z', '{' which
 * consists of ranges:
 *
 *     U+0058-U+005A
 *     U+007B-U+007B.
 *
 * The loop is expected to match the following characters:
 *
 *    x y z X Y Z {
 *
 * Note: Rhino fails this test while smjs succeeds.
 */

re = /[x-{]/i;
for (i = 0; i < 65536; i++) {
    t = String.fromCharCode(i);
    t = re.exec(t);
    if (t != null) {
        print('match: ' + i);
    }
}

/* XXX: add more canonicalization tests */
