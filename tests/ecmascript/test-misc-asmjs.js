/*
 *  Test basic asm.js compatibility.  This should be a given because
 *  asm.js is a strict subset of Javascript.
 */

/*===
undefined
===*/

/* Ensure that strict mode is detected even if followed by a 'use asm'. */

function declarationTest() {
    "use asm";
    "use strict";

    print(typeof this);  // strict: 'undefined', non-strict: 'object'
}

declarationTest.call(undefined);

/*===
5
===*/

/* Example from asm.js spec: http://asmjs.org/spec/latest/.
 * Ensure that it works as expected, ignoring asm.js mode.
 */

function DiagModule(stdlib) {
    "use asm";

    var sqrt = stdlib.Math.sqrt;

    function square(x) {
        x = +x;
        return +(x*x);
    }

    function diag(x, y) {
        x = +x;
        y = +y;
        return +sqrt(square(x) + square(y));
    }

    return { diag: diag };
}

var mod = DiagModule(this);  // this = global object
print(mod.diag(3, 4));  // -> 5
