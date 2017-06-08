/*
 *  ES2017 Annex B: __defineSetter__
 */

/*===
set: 123
set: foo
set: bar
set: quux
set: baz
set: quuux
TypeError
TypeError
TypeError
2
__defineSetter__
===*/

function setter(v) {
    print('set:', v);
}

function testDefineSetter() {
    var obj = {};

    // Basic case.
    obj.__defineSetter__('foo', setter);
    obj.foo = 123;
    Object.prototype.foo = 123;

    // Key is ToPropertyKey() coerced.
    Object.prototype.__defineSetter__(123, setter);
    obj['123'] = 'foo';
    Object.prototype['123'] = 'bar';
    obj.__defineSetter__(void 0, setter);
    obj.undefined = 'quux';
    obj.__defineSetter__(null, setter);
    obj.null = 'baz';
    obj.__defineSetter__({}, setter);
    obj['[object Object]'] = 'quuux';

    // Non-callable function is rejected with TypeError.
    try {
        obj.__defineSetter__('abc', {});
    } catch (e) {
        print(e.name);
    }

    // Failure to establish the own property is a TypeError. ES7 uses
    obj = {};
    Object.preventExtensions(obj);
    try {
        obj.__defineSetter__('abc', setter);
        print('never here');
    } catch (e) {
        print(e.name);
    }
    obj = {};
    Object.defineProperty(obj, 'abc', { value: 123, writable: false, enumerable: false, configurable: false });
    try {
        obj.__defineSetter__('abc', setter);
        print('never here');
    } catch (e) {
        print(e.name);
    }

    // .length and .name
    print(Object.prototype.__defineSetter__.length);
    print(Object.prototype.__defineSetter__.name);
}

try {
    testDefineSetter();
} catch (e) {
    print(e.stack || e);
}
