/*
 *  Test case analogous to test-bug-array-proto-protected-1.js but for Object.
 *  Object literals should use [[DefineOwnProperty]] which allows writing an
 *  own property regardless of what is present in the parent.
 */

/*===
defineProperty success
inherit undefined
foo bar
inherit bar
shouldsucceed bar
===*/

function test() {
    var obj;

    Object.defineProperty(Object.prototype,
                          'prop1',
                          {
                              value: 'inherit',
                              writable: false,
                              enumerable: true,
                              configurable: true
                          });
    print('defineProperty success');

    // No problem here
    obj = {};
    print(obj.prop1, obj.prop2);

    // Again should succeed
    obj = { prop1: 'foo', prop2: 'bar' };
    print(obj.prop1, obj.prop2);

    // Assignment should not work unless an own property already exists
    obj = {};
    obj.prop1 = 'foo';  // failure is silent
    obj.prop2 = 'bar';
    print(obj.prop1, obj.prop2);

    obj = { prop1: 'foo', prop2: 'bar' };
    obj.prop1 = 'shouldsucceed';
    print(obj.prop1, obj.prop2);
}

try {
    test();
} catch (e) {
    print(e);
}
