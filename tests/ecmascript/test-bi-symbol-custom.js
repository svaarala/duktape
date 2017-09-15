/*
 *  Some Duktape specific Symbol tests.
 */

/*@include util-buffer.js@*/

/*===
TypeError
TypeError
{"foo":123,"baz":[1,2,null,4,5]}
{foo:123,baz:[1,2,null,4,5]}
0
0
SETTER called number 123 symbol Symbol(accessor)
GETTER called symbol Symbol(accessor)
dummy-getter
===*/

function test() {
    // Because Symbols fail with TypeError in string concatenation, this
    // Duktape 1.x idiom to create an internal string no longer works.
    try {
        print(Duktape.enc('jx', bufferToStringRaw(Duktape.dec('hex', 'ff')) + 'Value'));
    } catch (e) {
        print(e.name);
    }

    // Uint8Array.allocPlain() rejects symbols with TypeError.
    try {
        Uint8Array.allocPlain(Symbol.for('foo'));
    } catch (e) {
        print(e.name);
    }

    // For now, Symbols don't serialize as JX/JC property keys or values.
    var obj = {
        foo: 123,
        [ Symbol.for('bar') ]: 234,
        quux: Symbol.iterator,
        baz: [ 1, 2, Symbol(), 4, 5 ]
    }
    print(JSON.stringify(obj));
    print(Duktape.enc('jx', obj));

    // Date instances have a hidden _Value (\x82Value) property.
    // It is not returned by Object.getOwnPropertySymbols() directly
    // or via a Proxy.

    var date = new Date();
    print(Object.getOwnPropertySymbols(date).length);
    var proxy = new Proxy(date, {});
    print(Object.getOwnPropertySymbols(proxy).length);

    // Extra key argument for getter and setter.
    var obj = {};
    Object.defineProperty(obj, Symbol.for('accessor'), {
        get: function (k) { print('GETTER called', typeof k, String(k)); return 'dummy-getter'; },
        set: function (v, k) { print('SETTER called', typeof v, v, typeof k, String(k)); }
    });
    obj[Symbol.for('accessor')] = 123;
    print(obj[Symbol.for('accessor')]);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
