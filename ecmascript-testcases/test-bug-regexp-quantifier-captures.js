/*
 *  Bug testcase for a Duktape 0.9.0 RegExp quantifier / capture handling bug.
 *  Captures inside a quantified atom are not reset to undefined when the
 *  quantifier re-matches the quantified.  This was found with test262 suite
 *  tests:
 *
 *    ch15/15.10/15.10.2/15.10.2.5/S15.10.2.5_A1_T4
 *    ch15/15.10/15.10.6/15.10.6.2/S15.10.6.2_A1_T6
 */

/*===
type object
index 0
input zaacbbbcac
length 6
0 zaacbbbcac
1 z
2 ac
3 a
4 undefined
5 c
===*/

function regexpQuantifierCaptureTest() {
    var re = /(z)((a+)?(b+)?(c))*/;
    var m = re.exec("zaacbbbcac");
    var i;

    // Matching:
    //
    //  - '(z)' should match (into capture 1)
    //
    // At this point the quantifier '*' (at the end) starts iterating
    // over "((a+)?(b+)?(c))".
    //
    //  - Captures 2, 3, 4, 5 are set to undefined
    //  - '(a+)?' should match 'aa' greedily (into capture 3)
    //  - '(b+)?' should be skipped (undefined into capture 4)
    //  - '(c)' should match the first 'c' (into capture 5)
    //  - 'aac' gets recorded into capture 2
    //
    // At this point we've matched 'zaac', and the second capture
    // group is matched again, at 'bbbcac':
    //
    //  - Captures 2, 3, 4, 5 are set to undefined
    //  - '(a+)' is skipped (undefined into capture 3)
    //  - '(b+)' matches 'bbb' (into capture 4)
    //  - '(c)' matches 'c' (into capture 5)
    //  - 'bbbc' gets recorded into capture 2
    //
    // At this point we've matched 'zaac' and 'bbbc', and the second
    // capture group is matched again, at 'ac':
    //
    //  - Captures 2, 3, 4, 5 are set to undefined
    //  - '(a+)' matches 'a' (into capture 3)
    //  - '(b+)' should be skipped (undefined into capture 4)
    //  - '(c) should match 'c' (into capture 5)
    //  - 'ac' gets recorded into capture 2
    //
    // The bug in Duktape 0.9.0 is that when the second capture group
    // (i.e. "((a+)?(b+)?(c))*") gets re-matched by the quantifier,
    // the captures inside must be set to undefined at each "loop" of
    // the quantifier.  See E5.1 Section 15.10.2.5, step 4 of the
    // RepeatMatcher algorithm.  Because Duktape doesn't do that now,
    // the (b+)? matcher, which is skipped on the third iteration,
    // retains its value from the second iteration (i.e. 'bbb')

    print('type', typeof m);
    print('index', m.index);
    print('input', m.input);
    print('length', m.length);
    for (i = 0; i <= 5; i++) {
        print(i, m[i]);
    }
}

try {
    regexpQuantifierCaptureTest();
} catch (e) {
    print(e);
}
