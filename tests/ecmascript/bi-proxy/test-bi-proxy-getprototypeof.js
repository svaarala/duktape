/*===
123
===*/

function test1() {
    // When [[Get]] is running from bar -> P -> obj, the 'getPrototypeOf' trap is
    // NOT invoked.  If the Proxy doesn't have a 'get' trap, [[Get]] just moves
    // silently to the target of the Proxy.
    var obj = { foo: 123 };
    var handler = {
        getPrototypeOf: function (target) {
            'use strict';
            print('getPrototypeOf:', this === handler, target === P, target === bar);
            return { foo: 234 };
        }
    };
    var P = new Proxy(obj, handler);
    var bar = Object.create(P);
    print(bar.foo);
}

test1();
