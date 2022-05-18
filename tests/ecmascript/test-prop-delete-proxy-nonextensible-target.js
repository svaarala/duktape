/*===
{"foo":1,"bar":2,"quux":3}
true
{"bar":2,"quux":3}
delete trap called for key bar
TypeError
{"bar":2,"quux":3}
===*/

function test() {
    var obj = { foo: 1, bar: 2, quux: 3 };
    Object.preventExtensions(obj);
    print(JSON.stringify(obj));

    var P = new Proxy(obj, {
        deleteProperty: function (targ, key) {
            print('delete trap called for key', key);
            return true;
        }
    });

    // Without Proxy can delete an own property of a non-extensible object.
    print(delete obj.foo);
    print(JSON.stringify(obj));

    // A Proxy cannot report an own property of a non-extensible object
    // deleted (without deleting it from the target prior to trap return);
    // this violates Proxy policy checks.
    try {
        print(delete P.bar);
    } catch (e) {
        print(e.name);
    }
    print(JSON.stringify(obj));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
