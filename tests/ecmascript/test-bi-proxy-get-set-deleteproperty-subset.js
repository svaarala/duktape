/*
 *  Proxy (ES2015) 'get', 'set', and 'deleteProperty' traps
 */

/*===
subset proxy
object
[object Object]
get foo: 123
get 1000: thousand
get special: specialValue
counts: get=0 set=0 del=0
handler.get: true true string foo true
get foo: fake-for-key-foo
handler.get: true true string bar true
get bar: fake-for-key-bar
handler.get: true true string 1000 true
get 1000 (string): fake-for-key-1000
handler.get: true true string 1000 true
get 1000 (number): fake-for-key-1000
counts: get=4 set=0 del=0
handler.get: true true string special true
get special: specialValue
handler.get: true true string special true
get special: uncaughtValue
target.special: uncaughtValue
counts: get=6 set=0 del=0
handler.set: true true string foo number 1001 true
handler.set: true true string bar number 1002 true
handler.set: true true string quux number 1003 true
handler.set: true true string 123 string foo true
handler.set: true true string rejectSet1 string reject true
handler.set: true true string rejectSet2 string reject true
handler.set: true true string rejectSet1 string throw true
TypeError
handler.set: true true string rejectSet2 string throw true
TypeError
counts: get=6 set=8 del=0
target.foo: 123
target.bar: 1002
target.quux: 1003
target[123]: foo
target.rejectSet1: undefined
target.rejectSet2: undefined
counts: get=6 set=8 del=0
true
target.foo: undefined
target.bar: 1002
counts: get=6 set=8 del=0
handler.deleteProperty: true true string rejectDel1
false
handler.deleteProperty: true true string rejectDel2
false
handler.deleteProperty: true true string rejectDel1
TypeError
handler.deleteProperty: true true string rejectDel2
TypeError
handler.deleteProperty: true true string foo
true
handler.deleteProperty: true true string 1234
true
counts: get=6 set=8 del=6
target.rejectDel1: reject1
target.rejectDel2: reject2
target.foo 123
target[1234] undefined
===*/

/* Test simple usage of the Duktape 2.x Proxy subset.  Does not exercise the
 * trap behaviors related to checking for conflicting properties in the
 * target object.
 */
function subsetProxyTest() {
    var getCount = 0;
    var setCount = 0;
    var deleteCount = 0;
    var target = { foo: 123, '1000': 'thousand', special: 'specialValue' };
    var handler = {};
    var proxy = new Proxy(target, handler);

    function printCounts() {
        print('counts:', 'get=' + getCount, 'set=' + setCount, 'del=' + deleteCount);
    }

    print(typeof proxy);
    print(Object.prototype.toString.call(proxy));

    // without a 'get' trap, reads go through
    print('get foo:', proxy.foo);
    print('get 1000:', proxy[1000]);
    print('get special:', proxy.special);

    // handler 'get' trap
    handler.get = function(targ, key, receiver) {
        print('handler.get:', this === handler, targ === target, typeof key, key, receiver === proxy);
        getCount++;
        if (typeof key === 'number') {
            return 2 * (+key);
        } else if (key === 'special') {
            return targ.special;
        } else {
            return 'fake-for-key-' + key;
        }
    };

    // Get tests
    printCounts();
    print('get foo:', proxy.foo);
    print('get bar:', proxy.bar);
    print('get 1000 (string):', proxy['1000']);
    print('get 1000 (number):', proxy[1000]);
    printCounts();

    // without a 'set' trap, writes go through
    print('get special:', proxy.special);
    proxy.special = 'uncaughtValue';  // goes into 'target'
    print('get special:', proxy.special);
    print('target.special:', target.special);

    // handler 'set' trap
    handler.set = function(targ, key, val, receiver) {
        print('handler.set:', this === handler, targ === target, typeof key, key, typeof val, val, receiver === proxy);
        setCount++;
        if (key === 'rejectSet1') {
            // False: indicate that property set is rejected (TypeError in strict mode).
            // No change happens to the target object.
            return false;
        }
        if (key === 'rejectSet2') {
            // Same for any 'falsy' value.
            return 0;
        }
        if (key === 'foo') {
             // True: indicate that property set is allowed, but no change happens
             // to the target object if we don't do it explicitly here.
             return true;
        }

        // Setting to target must be done explicitly.
        targ[key] = val;
        return true;
    };

    // Set tests
    printCounts();
    proxy.foo = 1001;
    proxy.bar = 1002;
    proxy.quux = 1003;
    proxy[123] = 'foo';
    proxy.rejectSet1 = 'reject';  // reject silently in non-strict mode
    proxy.rejectSet2 = 'reject';
    try {
        (function () { 'use strict'; proxy.rejectSet1 = 'throw'; })();
    } catch (e) {
        print(e.name);
    }
    try {
        (function () { 'use strict'; proxy.rejectSet2 = 'throw'; })();
    } catch (e) {
        print(e.name);
    }
    printCounts();
    print('target.foo:', target.foo);
    print('target.bar:', target.bar);
    print('target.quux:', target.quux);
    print('target[123]:', target[123]);
    print('target.rejectSet1:', target.rejectSet1);
    print('target.rejectSet2:', target.rejectSet2);

    // without a 'deleteProperty' trap, deletes go through
    printCounts();
    print(delete proxy.foo);
    print('target.foo:', target.foo);
    print('target.bar:', target.bar);
    printCounts();

    // handler 'deleteProperty' trap
    handler.deleteProperty = function(targ, key) {
        print('handler.deleteProperty:', this === handler, targ === target, typeof key, key);
        deleteCount++;
        if (key === 'rejectDel1') {
            // False return code indicates delete is rejected.
            return false;
        }
        if (key === 'rejectDel2') {
            // Same for any 'falsy' value.
            return 0;
        }
        if (key === 'foo') {
            // True return code indicates delete is accepted (but it has no
            // effect on the target unless we delete the property from the
            // target here).
            return true;
        }

        // Deletion to target must be done explicitly.
        delete targ[key];
        return true;
    };

    target.rejectDel1 = 'reject1';
    target.rejectDel2 = 'reject2';
    target.foo = 123;
    target[1234] = 4321;
    print(delete proxy.rejectDel1);
    print(delete proxy.rejectDel2);
    try {
        (function () { 'use strict'; print(delete proxy.rejectDel1); })();
    } catch (e) {
        print(e.name);
    }
    try {
        (function () { 'use strict'; print(delete proxy.rejectDel2); })();
    } catch (e) {
        print(e.name);
    }
    print(delete proxy.foo);    // allowed, but no effect on target
    print(delete proxy[1234]);  // allowed, deletes value on target
    printCounts();
    print('target.rejectDel1:', target.rejectDel1);
    print('target.rejectDel2:', target.rejectDel2);
    print('target.foo', target.foo);
    print('target[1234]', target[1234]);
}

print('subset proxy');

try {
    subsetProxyTest();
} catch (e) {
    print(e);
}
