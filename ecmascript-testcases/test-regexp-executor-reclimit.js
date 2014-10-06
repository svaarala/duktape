/*
 *  RegExp executor recursion limit
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

    // Each non-simple expression (where alternatives match different char
    // counts) cause executor recursion without causing compiler recursion.
    // The pattern and input must be such that the recursion actually
    // realizes.  Avoid making the input non-matching: this will cause a
    // long timeout due to execution *step limit* due to back tracking,
    // which we're not testing here.

    res = '';
    for (i = 0; i < n; i++) {
      res = '(?:y|y.)' + res;
    }

    return res;
}

function wrappedTest(n) {
    var src, reg;
    var input;

    try {
        src = createRegexp(n);
        reg = new RegExp(src);
        input = '';
        while (input.length < n) { input += 'y'; }
        reg.exec(input);
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
