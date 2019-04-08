/*
 *  Example of virtualizing enumeration using a Proxy.
 */

/*===
ownKeys trap: true
getOwnPropertyDescriptor trap: true true string 0
getOwnPropertyDescriptor trap: true true string 1
getOwnPropertyDescriptor trap: true true string 2
getOwnPropertyDescriptor trap: true true string 3
getOwnPropertyDescriptor trap: true true string 4
getOwnPropertyDescriptor trap: true true string noSuch
getOwnPropertyDescriptor trap: true true string length
0
1
2
3
4
noSuch
===*/

function test() {
    var target = [ 'foo', 'bar', 'quux' ];
    var handler = {
        ownKeys: function () {
            // The ownKeys result set is validated against the Proxy to ensure
            // enumerated keys are enumerable; unless a getOwnPropertyDescriptor
            // trap exists these checks will be made against the target which
            // means non-existent properties won't be enumerated.§
            print('ownKeys trap:', this === handler);
            return [ '0', '1', '2', '3', '4', 'noSuch', 'length' ]
        },
        getOwnPropertyDescriptor: function (targ, key) {
            // Cannot report 'length' as enumerable (that would violate
            // getOwnPropertyDescriptor trap constraints).  However,
            // Duktape 2.2 (at least) won't enforce that constraint yet.
            print('getOwnPropertyDescriptor trap:', this === handler, targ === target, typeof key, key);
            if (key === 'length') {
                return Object.getOwnPropertyDescriptor(target, key);
            }
            return {
                value: target[key],
                enumerable: true,
                writable: true,
                configurable: true
            };
        }
    };
    var proxy = new Proxy(target, handler);

    // Without 'getOwnPropertyDescriptor' trap the only keys visible would be
    // '0', '1', and '2' which backing in the target object.
    for (var k in proxy) {
        print(k);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
