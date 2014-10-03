/*===
TypeError
TypeError
TypeError
===*/

/* TypeError is required for cyclic structures. */

function testCycle1() {
    var obj = { foo: 1 };
    obj.cycle = obj;
    print(JSON.stringify(obj));
}

function testCycle2() {
    var obj1 = { foo: 1 };
    var obj2 = { bar: 2 };
    obj1.cycle = obj2;
    obj2.cycle = obj1;
    print(JSON.stringify(obj1));
}

function testCycle3() {
    var obj1 = { foo: 1 };
    var obj2 = { bar: 2 };
    var obj3 = { quux: 3 };
    obj1.cycle = obj2;
    obj2.cycle = obj3;
    obj3.cycle = obj1;
    print(JSON.stringify(obj1));
}

try {
    testCycle1();
} catch (e) {
    print(e.name);
}

try {
    testCycle2();
} catch (e) {
    print(e.name);
}

try {
    testCycle3();
} catch (e) {
    print(e.name);
}

/*===
{"foo":1,"bar":2,"ref1":{"quux":3,"baz":4,"ref1":{"quuux":5},"ref2":{"quuux":5}},"ref2":{"quux":3,"baz":4,"ref1":{"quuux":5},"ref2":{"quuux":5}}}
===*/

/* The following visits the same object multiple times, but not in a cycle.
 * This test ensures that any loop stack or hash map is unwound correctly.
 */

function testNoCycle1() {
    var obj1 = { foo: 1, bar: 2 };
    var obj2 = { quux: 3, baz: 4 };
    var obj3 = { quuux: 5 };
    obj1.ref1 = obj2;
    obj1.ref2 = obj2;
    obj2.ref1 = obj3;
    obj2.ref2 = obj3;
    print(JSON.stringify(obj1));
}

try {
    testNoCycle1();
} catch (e) {
    print(e.name);
}
