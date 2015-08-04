/*
 *  Bug test case for a pc2line encoding error.  Triggered when the full
 *  32 bit encoding activates (diff is > 8 bits, roughly).
 */

/*===
line 2
TypeError 3
line 2
TypeError 53
line 2
TypeError 103
line 2
TypeError 153
line 2
TypeError 203
===*/

function rep(s, n) {
    var res = [];
    while (n--) { res.push(s); }
    return res.join('');
}

function evalCheck(code) {
    try {
        eval(code);
        print('successful eval (unexpected)');
    } catch (e) {
        print(e.name + ' ' + e.lineNumber);
    }
}

function test() {
    /* This problem originally occurred in test262 testing (for 'this').
     * The `null.foo` access causes a TypeError (from getting a property
     * from undefined), but it is attributed incorrectly if there are
     * enough many empty lines in between.
     */

    evalCheck("\nprint('line 2')\n" + "if (null.foo) {}");
    evalCheck("\nprint('line 2')\n" + rep('\n', 50) + "if (null.foo) {}");
    evalCheck("\nprint('line 2')\n" + rep('\n', 100) + "if (null.foo) {}");
    evalCheck("\nprint('line 2')\n" + rep('\n', 150) + "if (null.foo) {}");
    evalCheck("\nprint('line 2')\n" + rep('\n', 200) + "if (null.foo) {}");
}

try {
    test();
} catch (e) {
    print(e);
}
