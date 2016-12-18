/*
 *  Symbols in property accesses.
 */

/*@include util-symbol.js@*/

/*===
symbol property
foo
undefined
foo
true
true
false
false
true
true
false
false
object string true
object string true
object symbol true
object symbol true
object string true
object string true
object symbol true
object symbol true
object string true
object string true
object symbol true
object object true
object string true
object string true
object symbol true
object object true
object string true
object string true
object symbol true
object object true
SETTER called number 123
GETTER called
dummy-getter
SETTER called number 123
GETTER called
dummy-getter
foo bar
===*/

function symbolPropertyTest() {
    var s1, s2, s3, o1, o2, o3;
    var obj, child;
    var pd;

    obj = {};
    child = {}; Object.setPrototypeOf(child, obj);

    s1 = Symbol('123');
    s2 = Symbol('123');
    o1 = Object(s1);
    o2 = Object(s2);

    // Plain symbol and Object coerced Symbol reference the same property.
    obj[s1] = 'foo';
    print(obj[o1]);

    // Symbol with same description is a separate property if the symbol
    // instance is different.  Here, s2 is not present.
    print(obj[s2]);

    // Symbol accesses are normal property accesses in that they're inherited.
    print(child[o1]);

    // The 'in' operator works for symbols too.
    print(s1 in obj);
    print(o1 in obj);
    print(s2 in obj);
    print(o2 in obj);
    print(s1 in child);
    print(o1 in child);
    print(s2 in child);
    print(o2 in child);

    // Object.getOwnPropertyDescriptor() for a symbol key.
    obj[s1] = 'foo';
    pd = Object.getOwnPropertyDescriptor(obj, s1);
    print(typeof pd, typeof pd.value, pd.value === 'foo');
    pd = Object.getOwnPropertyDescriptor(obj, o1);
    print(typeof pd, typeof pd.value, pd.value === 'foo');

    // Object.getOwnPropertyDescrtiptor() for a symbol value.
    obj[s1] = Symbol.for('foobar');
    pd = Object.getOwnPropertyDescriptor(obj, s1);
    print(typeof pd, typeof pd.value, pd.value === Symbol.for('foobar'));
    pd = Object.getOwnPropertyDescriptor(obj, o1);
    print(typeof pd, typeof pd.value, pd.value === Symbol.for('foobar'));

    // Reflect.getOwnPropertyDescriptor(), similar tests.
    obj[s1] = 'foo';
    pd = Reflect.getOwnPropertyDescriptor(obj, s1);
    print(typeof pd, typeof pd.value, pd.value === 'foo');
    pd = Reflect.getOwnPropertyDescriptor(obj, o1);
    print(typeof pd, typeof pd.value, pd.value === 'foo');
    obj[s1] = Symbol.for('foobar');
    pd = Reflect.getOwnPropertyDescriptor(obj, s1);
    print(typeof pd, typeof pd.value, pd.value === Symbol.for('foobar'));
    pd = Reflect.getOwnPropertyDescriptor(obj, o1);
    print(typeof pd, typeof pd.value, pd.value === Symbol.for('foobar'));

    // Object.defineProperty() for a symbol key.
    Object.defineProperty(obj, s2, { value: 'quux', configurable: true });
    pd = Object.getOwnPropertyDescriptor(obj, s2);
    print(typeof pd, typeof pd.value, pd.value === 'quux');
    Object.defineProperty(obj, o2, { value: 'baz', configurable: true });
    pd = Object.getOwnPropertyDescriptor(obj, s2);
    print(typeof pd, typeof pd.value, pd.value === 'baz');

    // Object.defineProperty() for a symbol key and value.
    Object.defineProperty(obj, s2, { value: Symbol.for('123'), configurable: true });
    pd = Object.getOwnPropertyDescriptor(obj, s2);
    print(typeof pd, typeof pd.value, pd.value === Symbol.for('123'));
    Object.defineProperty(obj, o2, { value: Object(Symbol.for('123')), configurable: true });
    pd = Object.getOwnPropertyDescriptor(obj, s2);
    print(typeof pd, typeof pd.value, pd.value == Symbol.for('123'));

    // Object.defineProperties, same tests.
    Object.defineProperties(obj, {
        [ Symbol.for('K1') ]: { value: 'V1', configurable: true },
        [ Object(Symbol.for('K2')) ]: { value: 'V2', configurable: true },
        [ Symbol.for('K3') ]: { value: Symbol.for('V3'), configurable: true },
        [ Object(Symbol.for('K4')) ]: { value: Object(Symbol.for('V4')), configurable: true }
    });
    pd = Object.getOwnPropertyDescriptor(obj, Symbol.for('K1'));
    print(typeof pd, typeof pd.value, pd.value === 'V1');
    pd = Object.getOwnPropertyDescriptor(obj, Symbol.for('K2'));
    print(typeof pd, typeof pd.value, pd.value === 'V2');
    pd = Object.getOwnPropertyDescriptor(obj, Symbol.for('K3'));
    print(typeof pd, typeof pd.value, pd.value === Symbol.for('V3'));
    pd = Object.getOwnPropertyDescriptor(obj, Symbol.for('K4'));
    print(typeof pd, typeof pd.value, pd.value == Symbol.for('V4'));

    // Reflect.defineProperty(), same tests.  Reflect.defineProperties()
    // doesn't exist.
    Reflect.defineProperty(obj, s2, { value: 'quux', configurable: true });
    pd = Reflect.getOwnPropertyDescriptor(obj, s2);
    print(typeof pd, typeof pd.value, pd.value === 'quux');
    Reflect.defineProperty(obj, o2, { value: 'baz', configurable: true });
    pd = Reflect.getOwnPropertyDescriptor(obj, s2);
    print(typeof pd, typeof pd.value, pd.value === 'baz');
    Reflect.defineProperty(obj, s2, { value: Symbol.for('123'), configurable: true });
    pd = Reflect.getOwnPropertyDescriptor(obj, s2);
    print(typeof pd, typeof pd.value, pd.value === Symbol.for('123'));
    Reflect.defineProperty(obj, o2, { value: Object(Symbol.for('123')), configurable: true });
    pd = Reflect.getOwnPropertyDescriptor(obj, s2);
    print(typeof pd, typeof pd.value, pd.value == Symbol.for('123'));

    // Symbol accessor property.
    Object.defineProperty(obj, Symbol.for('accessor'), {
        get: function () { print('GETTER called'); return 'dummy-getter'; },
        set: function (v) { print('SETTER called', typeof v, v); }
    });
    obj[Symbol.for('accessor')] = 123;
    print(obj[Symbol.for('accessor')]);

    // Well-defined symbol accessor property.
    Object.defineProperty(obj, Symbol.iterator, {
        get: function () { print('GETTER called'); return 'dummy-getter'; },
        set: function (v) { print('SETTER called', typeof v, v); }
    });
    obj[Symbol.iterator] = 123;
    print(obj[Symbol.iterator]);

    // Array objects may have symbol properties.
    obj = [];
    obj[s1] = 'foo';
    obj[Symbol.iterator] = 'bar';
    print(obj[s1], obj[Symbol.iterator]);
}

try {
    print('symbol property');
    symbolPropertyTest();
} catch (e) {
    print(e.stack || e);
}
