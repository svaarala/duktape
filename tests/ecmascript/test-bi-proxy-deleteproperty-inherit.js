/*===
123 123
deleteProperty
object {"name":"handler"}
object {"name":"target","foo":123,"bar":234,"quux":345}
string "foo"
undefined undefined
undefined undefined
123 123
234 234
234 234
===*/

function test() {
    var P = new Proxy({ name: 'target', foo: 123, bar: 234, quux: 345 }, {
        deleteProperty: function (a,b,c,d) {
            'use strict';
            print('deleteProperty');
            print(typeof this, JSON.stringify(this));
            print(typeof a, JSON.stringify(a));
            print(typeof b, JSON.stringify(b));
            print(typeof c, JSON.stringify(c));
            print(typeof d, JSON.stringify(d));
        },
        name: 'handler'
    });
    var O = Object.create(P, { name: { value: 'child', enumerable: true } });
    print(P.foo, O.foo);
    delete(P.foo);
    print(P.foo, O.foo);

    // [[Delete]] doesn't walk the inheritance chain so this delete is NOT
    // inherited through to the Proxy target and doesn't invoke the trap.
    print(P.bar, O.bar);
    delete(O.bar);
    print(P.bar, O.bar);
}

test();
