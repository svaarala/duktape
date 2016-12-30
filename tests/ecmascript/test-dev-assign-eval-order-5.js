/*
 *  In X <op>= Y the LHS value should be evaluated first, before evaluating
 *  the RHS.
 */

/*===
slow path variable LHS
Write x: 10
TEST
Read x: 10
Read x: 10
Write x: 40
Write x: 50
Read x: 50
Final x: 50
===*/

// Slow path variable LHS.  Use 'with' statement to record side effects.
function slowPathVariableLhsTest() {
    var obj = {};
    var my_x;
    Object.defineProperty(obj, 'x', {
        set: function (v) { print('Write x:', v); my_x = v; return true; },
        get: function () { print('Read x:', my_x); return my_x; }
    });

    with (obj) {
        x = 10;
        print('TEST');
        x += (x *= 4);
    }
    print('Final x:', obj.x);
}

try {
    print('slow path variable LHS');
    slowPathVariableLhsTest();
} catch (e) {
    print(e.stack || e);
}

/*===
fast path variable LHS
Final x: 50
===*/

// Fast path variable LHS.
function fastPathVariableLhsTest() {
    var x = 10;
    x += (x *= 4);
    print('Final x:', x);
}

try {
    print('fast path variable LHS');
    fastPathVariableLhsTest();
} catch (e) {
    print(e.stack || e);
}

/*===
property access LHS
Final x: 50
===*/

// Property access LHS.
function propertyAccessLhsTest() {
    var obj = { x: 10 };
    obj.x += (obj.x *= 4);
    print('Final x:', obj.x);
}

try {
    print('property access LHS');
    propertyAccessLhsTest();
} catch (e) {
    print(e.stack || e);
}

/*===
function LHS
lhs1
lhs2
ReferenceError
lhs1
lhs2
ReferenceError
done
===*/

// Function as LHS; RHS must be evaluated but a runtime error occurs.
// V8 seems to skip RHS evaluation (maybe this has changed in ES2015?).

function functionLhsTest() {
    function dummy() {}

    try {
        dummy() = (print('lhs1') + print('lhs2'));
        print('never here');
    } catch (e) {
        print(e.name);
    }

    try {
        dummy() += (print('lhs1') + print('lhs2'));
        print('never here');
    } catch (e) {
        print(e.name);
    }
    print('done');
}

try {
    print('function LHS');
    functionLhsTest();
} catch (e) {
    print(e.stack || e);
}

/*===
optimization
4
400
404
2040
10200foobarquux
10200foobarquuxfoobarquux10
===*/

// This is just to inspect bytecode manually for most obvious cases.
function optimizationTest() {
    var x;
    var y = 10;

    print(x = 4);
    x = 4;
    print(x = 4 * 100);
    x = 4 * 100;

    print(x += 4);
    x += 4;

    print(x += x *= 4);
    x += x *= 4;

    print(x += 'foo' + 'bar' + 'quux');
    x += 'foo' + 'bar' + 'quux';

    // Even this evaluates to an optimal sequence: the RHS is 'y' which is
    // a register bound variable so no code is emitted to access it during
    // the RHS evaluation (RHS result is simply an ivalue pointing to the
    // register).  So, x += y is safe to execute without a temporary.
    print(x += y);
    x += y;
}

try {
    print('optimization');
    optimizationTest();
} catch (e) {
    print(e.stack || e);
}
