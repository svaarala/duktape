/*
 *  RegExp executor step limit
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
failure for: 100 -> RangeError true
failure for: 1000 -> RangeError true
===*/

function createRegexp(n) {
    var res = [];
    var i;

    // Each non-simple expression (where alternatives match different char
    // counts) cause executor recursion without causing compiler recursion.
    //
    // The pattern and input are chosen such that the executor will first
    // match all 'y' alternatives, and then the final 'x' will cause a
    // match failure and very time consuming back tracking (each back
    // tracking alternative failing at the 'x' at the latest).  This causes
    // a step limit before a recursion limit is reached.

    res = 'x';
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
        print('failure for:', n, '->', e.name, e.message.indexOf('step') >= 0);
    }
}

function test() {
    wrappedTest(1);
    wrappedTest(10);
    wrappedTest(100);   // Causes a step limit RangeError
    wrappedTest(1000);  // Causes a step limit RangeError
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
