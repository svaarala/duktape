/*
 *  Test illustrating GH-1684, GH-1726, GH1786.
 */

/*===
entered test1
entered anotherFunction
entered test2
entered anotherFunction
===*/

/*
 *  Run with debugger attached, and:
 *
 *  - StepOut from test1 and test2.
 *
 *  - StepOver the 'return' statement in test1 and test2.
 *
 *  - StepInto anotherFunction in test1 and test2.
 */

function anotherFunction() {
    var ret;
    print('entered anotherFunction');
    ret = 'foo' + 123;
    return ret;
}

function test1() {
    print('entered test1');
    return anotherFunction();
}

function test2() {
    var ret;

    print('entered test2');
    ret = anotherFunction();
    return ret;
}

test1();
test2();

/*===
before bar
after bar
===*/

/*
 *  Nested case.  The tailcall in a nested call (not occurring in the
 *  function where stepping starts) shouldn't affect line pausing.
 */

function foo() {
    return 1;
}
function bar() {
    return foo();
}
function test() {
    print('before bar');
    bar();  // step over this
    print('after bar');
}

test();
