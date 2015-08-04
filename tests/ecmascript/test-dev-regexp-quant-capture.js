/*
 *  Development time tests for quantifier capture fixing
 *  (branch regexp-quantifier-capture-fix).
 */

/*===
object abcABC ABC A B C
object abcBC BC undefined B C
===*/

function test() {
    /* A 'wipe' instruction is used to wipe saved pointers when a quantifier
     * is re-matched.  Values before the wipe must be restored when backtracking,
     * otherwise this fails: the quantified is matched successfully twice, but
     * the third time fails.  We must backtrack and return captures from the
     * second round.  (This actually worked in Duktape 0.9.0 because quantifier
     * re-matching didn't reset captures, but it worked for the wrong reason.)
     */

    m = /((a)(b)(c))+/i.exec('abcABCabz');
    print(typeof m, m[0], m[1], m[2], m[3], m[4]);

    /* Same test case but with a small twist.  This failed in Duktape 0.9.0:
     * the second capture matches 'a' on the first repetition and is undefined
     * on the second.  The final result should be undefined but is 'a' in 0.9.0.
     */
    m = /((a)?(b)(c))+/i.exec('abcBCbz');
    print(typeof m, m[0], m[1], m[2], m[3], m[4]);
}

try {
    test();
} catch (e) {
    print(e);
}
