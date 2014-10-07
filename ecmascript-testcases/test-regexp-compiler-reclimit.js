/*
 *  RegExp compiler recursion limit
 */

/* Marked custom because limit is custom behavior. */
/*---
{
    "custom": true
}
---*/

/*===
success for: 1
success for: 10
success for: 100
failure for: 1000 -> RangeError
===*/

function createRegexp(n) {
    var res = [];
    var i;

    // Each disjunction causes recursion.
    res = 'x';
    for (i = 0; i < n; i++) {
      res = '(?:y|' + res + ')';
    }

    return res;
}

function wrappedTest(n) {
    var src, reg;

    try {
        src = createRegexp(n);
        reg = new RegExp(src);
        print('success for:', n);
    } catch (e) {
        print('failure for:', n, '->', e.name);
    }
}

function test() {
    wrappedTest(1);
    wrappedTest(10);
    wrappedTest(100);
    wrappedTest(1000);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
