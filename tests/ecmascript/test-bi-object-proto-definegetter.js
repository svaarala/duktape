/*
 *  ES2017 Annex B __defineGetter__
 */

/*===
BAR
undefined
QUUX
QUUX
BAR
BAR
BAR
TypeError
TypeError
TypeError
2
__defineGetter__
===*/

function testDefineGetter() {
    var obj = {};

    // Basic case.
    obj.__defineGetter__('foo', function returnBar() { return 'BAR'; });
    print(obj.foo);
    print(Object.prototype.foo);

    // Key is ToPropertyKey() coerced.
    Object.prototype.__defineGetter__(123, function returnQuux() { return 'QUUX'; });
    print(obj['123']);
    print(Object.prototype['123']);
    obj.__defineGetter__(void 0, function returnBar() { return 'BAR'; });
    print(obj.undefined);
    obj.__defineGetter__(null, function returnBar() { return 'BAR'; });
    print(obj.null);
    obj.__defineGetter__({}, function returnBar() { return 'BAR'; });
    print(obj['[object Object]']);

    // Non-callable function is rejected with TypeError.
    try {
        obj.__defineGetter__('abc', {});
    } catch (e) {
        print(e.name);
    }

    // Failure to establish the own property is a TypeError; ES7 uses
    // DefinePropertyOrThrow which provides this throwing behavior
    // regardless of function strictness.  NodeJs 4.2.6 will silently
    // ignore failed attempts, Firefox 53.0.3 throws.
    obj = {};
    Object.preventExtensions(obj);
    try {
        obj.__defineGetter__('abc', function returnBar() { return 'BAR'; });
        print('never here');
    } catch (e) {
        print(e.name);
    }
    obj = {};
    Object.defineProperty(obj, 'abc', { value: 123, writable: false, enumerable: false, configurable: false });
    try {
        obj.__defineGetter__('abc', function returnBar() { return 'BAR'; });
        print('never here');
    } catch (e) {
        print(e.name);
    }

    // .length and .name
    print(Object.prototype.__defineGetter__.length);
    print(Object.prototype.__defineGetter__.name);
}

try {
    testDefineGetter();
} catch (e) {
    print(e.stack || e);
}
