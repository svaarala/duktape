/*
 *  A bound eval call is an indirect eval, even if called via 'eval'
 *  and the final function being the built-in native eval function.
 */

/*===
6
local foo
global foo
global foo
===*/

var FOO = 'global foo';

var origEval = eval;

function test() {
    var eval;
    var FOO = 'local foo';

    // Basic test.
    eval = origEval.bind(null, 'print(1+2+3)');
    eval('print(1000)');

    // Direct eval sees our closure.
    eval = origEval;
    eval('print(FOO)');

    // Indirect eval does not.
    eval = origEval;
    (1, eval)('print(FOO)');

    // Bound eval also doesn't see our closure.
    eval = origEval.bind(null, 'print(FOO)');
    eval('print(1000)');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
