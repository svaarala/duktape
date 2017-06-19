/*
 *  'eval()' call with eval mapping to something else than the built-in
 *  native eval() function allows Ecma-to-Ecma calls without native
 *  recursion.
 */

/*===
count: 3000
===*/

var origEval = eval;
var count = 0;

var eval = function (x) {
    count++;
    if (x <= 1) { return; }
    test(x - 1);
}

function test(x) {
    eval(x);
}

try {
    test(3e3);
    print('count:', count);
} catch (e) {
    print(e.stack || e);
}
