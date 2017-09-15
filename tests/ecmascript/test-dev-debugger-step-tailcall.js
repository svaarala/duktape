/*
 *  Test illustrating GH-1684 and GH-1726.
 *
 *  Run with debugger attached, and:
 *
 *  - StepOut from test1 and test2.
 *
 *  - StepOver the 'return' statement in test1 and test2.
 *
 *  - StepInto anotherFunction in test1 and test2.
 */

/*===
entered test1
entered anotherFunction
entered test2
entered anotherFunction
done
===*/

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

try {
    test1();
    test2();
} catch (e) {
    print(e.stack || e);
}

print('done');
