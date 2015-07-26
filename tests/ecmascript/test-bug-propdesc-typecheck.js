/*
 *  Object.defineProperty(), Object.defineProperties(), and Object.create()
 *  must reject (TypeError) non-object property descriptors.
 */

/*===
TypeError
TypeError
TypeError
===*/

function test1() {
    var o = Object.create(Object.prototype, { prop1: 'non-object' });
    print('should not be here');
}

function test2() {
    var o = {};
    Object.defineProperty(o, 'prop', 'non-object');
    print('should not be here');
}

function test3() {
    var o = {};
    Object.defineProperties(o, { prop1: { value: 'foo' }, prop2: 'non-object' });
    print('should not be here');
}

try {
    test1();
} catch (e) {
    print(e.name);
}

try {
    test2();
} catch (e) {
    print(e.name);
}

try {
    test3();
} catch (e) {
    print(e.name);
}
