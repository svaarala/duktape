/*
 *  Constructor .prototype lookup when calling through a Proxy without
 *  'construct' trap.  The .prototype should be looked up via the Proxy
 *  and not the target, allowing the Proxy to capture the lookup.
 */

/*===
- direct call
bar-prototype
- call via proxy
get trap: prototype
bar-replaced
===*/

function MyConstructor() {
}

MyConstructor.prototype.foo = 'bar-prototype';

function test() {
    var replaced = { foo: 'bar-replaced' };
    var proxy = new Proxy(MyConstructor, {
        get: function get(targ, prop, recv) {
            print('get trap:', prop);
            if (prop === 'prototype') {
                return replaced;
            }
            return targ[prop];
        }
    });
    var tmp;

    print('- direct call');
    tmp = new MyConstructor();
    print(tmp.foo);

    print('- call via proxy');
    tmp = new proxy();
    print(tmp.foo);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
