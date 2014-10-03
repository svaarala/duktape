// indirect eval -> this is bound to the global object, E5 Section 10.4.2, step 1.a.
var g = (function () { var e = eval; return e('this'); } )();

/*===
true number NaN
true number Infinity
true undefined undefined
===*/

/* This test overlaps with test-bi-properties. */

function testValue(name) {
    var v = g[name];
    print(name in g, typeof v, v);
}

try {
    testValue('NaN');
    testValue('Infinity');
    testValue('undefined');
} catch (e) {
    print(e.name);
}
