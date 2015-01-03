/*
 *  For-in statement (E5 Section 12.6.4).
 *
 *  test-stmt-for-in-lhs.js tests for various forms of LHS expressions
 *  which has a lot of engine variation.
 */

/*===
0
1
2
final i: 2
0
1
2
final globalIdx: 2
foo
bar
quux
final j: quux
final k: undefined
shadow before: undefined
final shadow: undefined
Object.keys: foo,quux,bar
foo
quux
bar
===*/

/* Some basic tests. */

var globalIdx;
var shadow = 'global shadow';

function basicTest() {
    var i;
    var k;

    // The for-in loop will enumerate keys of an object; for arrays this
    // means key indices.
    for (i in [ 'foo', 'bar', 'quux' ]) {
        print(i);
    }
    print('final i:', i);

    // Global variable as LHS.
    for (globalIdx in [ 'foo', 'bar', 'quux' ]) {
        print(globalIdx);
    }
    print('final globalIdx:', globalIdx);

    // Declare variable in LHS.
    for (var j in { foo: 1, bar: 2, quux: 3 }) {
        print(j);
    }
    print('final j:', j);  // variable declaration is 'hoisted'

    // If RHS is empty, LHS does not get executed/assigned to.
    for (k in {}) {
        print(k);
    }
    print('final k:', k);

    // But if LHS is a variable declaration, that variable gets declared
    // even if RHS is empty (hoisting).
    print('shadow before:', shadow);  // declared already here
    for (var shadow in {}) {
        print(shadow);
    }
    print('final shadow:', shadow);

    // for-in order matches Object.keys(); detailed enumeration tests are
    // covered by other tests.
    var testObj = {};
    testObj.foo = 1;
    testObj.quux = 2;
    testObj.bar = 3;
    print('Object.keys:', Object.keys(testObj));
    for (i in testObj) {
        print(i);
    }
}

try {
    basicTest();
} catch (e) {
    print(e.stack || e);
}
