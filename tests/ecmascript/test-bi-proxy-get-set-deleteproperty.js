/*
 *  Proxy (ES6) 'get', 'set', and 'deleteProperty' traps
 */

/*===
proxy existence
Proxy exists: true function
Proxy.length: 2
Proxy.name: Proxy
Proxy desc: writable=true enumerable=false configurable=true
Proxy.revocable exists: false undefined
===*/

function proxyExistenceTest() {
    var pd;

    print('Proxy exists:', 'Proxy' in this, typeof this.Proxy);
    print('Proxy.length:', this.Proxy.length);
    print('Proxy.name:', this.Proxy.name);
    pd = Object.getOwnPropertyDescriptor(this, 'Proxy');
    if (pd) {
        print('Proxy desc:', 'writable=' + pd.writable, 'enumerable=' + pd.enumerable,
              'configurable=' + pd.configurable);
    }

    print('Proxy.revocable exists:', 'revocable' in this.Proxy, typeof this.Proxy.revocable);
/*
    print('Proxy.revocable.length:', this.Proxy.revocable.length);
    print('Proxy.revocable.name:', this.Proxy.revocable.name);
    pd = Object.getOwnPropertyDescriptor(this.Proxy, 'revocable');
    if (pd) {
        print('Proxy.revocable desc:', 'writable=' + pd.writable, 'enumerable=' + pd.enumerable,
              'configurable=' + pd.configurable);
    }
*/
}

print('proxy existence');

try {
    proxyExistenceTest();
} catch (e) {
    print(e);
}

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
handler.get: true true number 1000 true
get 1000 (number): 2000
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
handler.set: true true number 123 string foo true
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
handler.deleteProperty: true true number 1234
true
counts: get=6 set=8 del=6
target.rejectDel1: reject1
target.rejectDel2: reject2
target.foo 123
target[1234] undefined
===*/

/* Test simple usage of the current Proxy subset.  Does not exercise the
 * hook behaviors related to checking for conflicting properties in the
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
    print(Object.prototype.toString.call(proxy));  // XXX: now class is 'Object'

    // without a 'get' hook, reads go through
    print('get foo:', proxy.foo);
    print('get 1000:', proxy[1000]);
    print('get special:', proxy.special);

    // handler 'get' hook
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

    // without a 'set' hook, writes go through
    print('get special:', proxy.special);
    proxy.special = 'uncaughtValue';  // goes into 'target'
    print('get special:', proxy.special);
    print('target.special:', target.special);

    // handler 'set' hook
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

    // without a 'deleteProperty' hook, deletes go through
    printCounts();
    print(delete proxy.foo);
    print('target.foo:', target.foo);
    print('target.bar:', target.bar);
    printCounts();

    // handler 'deleteProperty' hook
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

/*===
hook post-checks
get hook
property1: success, value: whatever
property2: success, value: whatever
property3: success, value: whatever
property4: success, value: whatever
property5: success, value: 42
property6: success, value: NaN
property7: success, value: 0
property1: success, value: whatever
property2: success, value: whatever
property3: success, value: whatever
property4: success, value: whatever
property5: TypeError
property6: TypeError
property7: TypeError
accessor1: success, value: undefined
accessor2: success, value: undefined
accessor3: success, value: undefined
accessor4: success, value: undefined
accessor5: success, value: undefined
accessor6: success, value: undefined
accessor7: success, value: undefined
accessor1: success, value: 123
accessor2: success, value: 123
accessor3: success, value: 123
accessor4: success, value: 123
accessor5: TypeError
accessor6: TypeError
accessor7: success, value: 123
set hook
property1: success, property1 set to 42
property2: success, property2 set to 42
property3: success, property3 set to 42
property4: success, property4 set to 42
property5: success, property5 set to 42
property6: success, property6 set to NaN
property7: success, property7 set to 0
property1: success, property1 set to 142
property2: success, property2 set to 142
property3: success, property3 set to 142
property4: success, property4 set to 142
property5: TypeError trying to set value to 142
property6: TypeError trying to set value to Infinity
property7: TypeError trying to set value to 0
accessor1: success, accessor1 set to whatever
accessor2: success, accessor2 set to whatever
accessor3: success, accessor3 set to whatever
accessor4: success, accessor4 set to whatever
accessor5: TypeError trying to set value to whatever
accessor6: success, accessor6 set to whatever
accessor7: TypeError trying to set value to whatever
deleteProperty hook
property1: success, result: true
property2: success, result: true
property3: TypeError
property4: TypeError
property5: TypeError
property6: TypeError
property7: TypeError
accessor1: success, result: true
accessor2: success, result: true
accessor3: TypeError
accessor4: success, result: true
accessor5: TypeError
accessor6: TypeError
accessor7: TypeError
===*/

/* If a hook exists and is successfully called, ES6 specifies interesting
 * post-hook behavior where a TypeError may be raised if the hook return
 * value conflicts in some way with a property of the same name in the
 * target object.
 */

function makeDataTestObject() {
    var obj = {};

    Object.defineProperties(obj, {
        // Various properties whose behavior to compare against the hooks

        property1: {
            writable: true, enumerable: true, configurable: true, value: 42
        },
        property2: {
            writable: false, enumerable: true, configurable: true, value: 42
        },
        property3: {
            writable: true, enumerable: true, configurable: false, value: 42
        },
        property4: {
            writable: true, enumerable: false, configurable: false, value: 42
        },
        property5: {
            writable: false, enumerable: true, configurable: false, value: 42
        },
        property6: {
            writable: false, enumerable: true, configurable: false, value: NaN
        },
        property7: {
            writable: false, enumerable: true, configurable: false, value: +0
        }
    });

    return obj;
}

function makeAccessorTestObject() {
    var obj = {};

    function getter() {
        print('getter called');
        return 'getter-value';
    }

    function setter(v) {
        print('setter called:', v);
    }

    Object.defineProperties(obj, {
        // Various properties whose behavior to compare against the hooks
        accessor1: {
            enumerable: true, configurable: true, set: setter, get: getter
        },
        accessor2: {
            enumerable: false, configurable: true, set: setter, get: getter
        },
        accessor3: {
            enumerable: true, configurable: false, set: setter, get: getter
        },
        accessor4: {
            enumerable: true, configurable: true, set: undefined, get: undefined
        },
        accessor5: {
            enumerable: true, configurable: false, set: undefined, get: undefined
        },
        accessor6: {
            enumerable: true, configurable: false, set: setter, get: undefined
        },
        accessor7: {
            enumerable: true, configurable: false, set: undefined, get: getter
        }
    });

    return obj;
}

function getHookPostChecksTest() {
    var target, handler, proxy;

    function getTest(proxy, propname) {
        try {
            var val = proxy[propname];
            print(propname + ': success, value:', val);
        } catch (e) {
            print(propname + ':', e.name);
        }
    }

    /* 'get' return value is rejected if the target has a property of the
     * same name and the property:
     *
     *   - Is a data property, is not configurable, is not writable, and
     *     hook provided value does not match with the current value
     *     (as compared with SameValue).
     *
     *   - Is an accessor property, is not configurable, getter is not
     *     defined, and hook provided value is not undefined.
     */

    /*
     *  Data properties
     */

    target = makeDataTestObject();

    handler = {};
    handler.get = function (targ, key, receiver) {
        if (key === 'property5') { return 42; }   // same value, no error
        if (key === 'property6') { return NaN; }  // same value, no error
        if (key === 'property7') { return +0; }   // same value, no error
        return 'whatever';  // differs from all values
    };
    proxy = new Proxy(target, handler);
    Object.getOwnPropertyNames(target).forEach(function (propname) {
        getTest(proxy, propname);
    });

    handler = {};
    handler.get = function (targ, key, receiver) {
        if (key === 'property5') { return 41; }         // different value, error
        if (key === 'property6') { return undefined; }  // different value, error
        if (key === 'property7') { return -0; }         // different value, error
        return 'whatever';  // differs from all values
    };
    proxy = new Proxy(target, handler);
    Object.getOwnPropertyNames(target).forEach(function (propname) {
        getTest(proxy, propname);
    });

    /*
     *  Accessor properties
     */

    target = makeAccessorTestObject();

    handler = {};
    proxy = new Proxy(target, handler);
    handler.get = function (targ, key, receiver) {
        // If trapResult is undefined, post-hook checks always pass
        return undefined;
    }
    Object.getOwnPropertyNames(target).forEach(function (propname) {
        getTest(proxy, propname);
    });

    handler = {};
    proxy = new Proxy(target, handler);
    handler.get = function (targ, key, receiver) {
        // If trapResult is not undefined, post-hook checks cause a TypeError
        // if property is non-configurable and getter is undefined.
        return 123;
    }
    Object.getOwnPropertyNames(target).forEach(function (propname) {
        getTest(proxy, propname);
    });
}

function setHookPostChecksTest() {
    var target, handler, proxy;

    function setTest(proxy, propname, propvalue) {
        try {
            proxy[propname] = propvalue;
            print(propname + ': success,', propname, 'set to', propvalue);
        } catch (e) {
            print(propname + ':', e.name, 'trying to set value to', propvalue);
        }
    }

    /* 'set' is rejected if the target has a property of the same name
     * and the property:
     *
     *   - Is a data property, is not configurable, is not writable, and
     *     the assignment value does not match with the current value
     *     (as compared with SameValue).
     *
     *   - Is an accessor property, is not configurable, and setter is not
     *     defined.  Unlike for 'get' the value does not matter.
     */

    /*
     *  Data properties
     */

    target = makeDataTestObject();

    handler = {};
    proxy = new Proxy(target, handler);
    handler.set = function (targ, key, val, receiver) {
        // If 'false' is returned, property write is rejected and the post-hook
        // behavior doesn't activate at all, so always return true here.
        return true;
    }
    Object.getOwnPropertyNames(target).forEach(function (propname) {
        // Choose test value to match current value (with SameValue).
        // No TypeError is triggered even if other conditions are met.

        var propval = {
            property1: 42,
            property2: 42,
            property3: 42,
            property4: 42,
            property5: 42,
            property6: NaN,
            property7: +0
        }[propname];

        setTest(proxy, propname, propval);
    });

    handler.set = function (targ, key, val, receiver) {
        return true;
    };
    Object.getOwnPropertyNames(target).forEach(function (propname) {
        // Choose test value to match current value (with SameValue).
        // No TypeError is triggered even if other conditions are met.

        var propval = {
            property1: 142,
            property2: 142,
            property3: 142,
            property4: 142,
            property5: 142,
            property6: 1/0,
            property7: -0  // SameValue, even sign matters
        }[propname];

        setTest(proxy, propname, propval);
    });

    /*
     *  Accessor properties
     */

    target = makeAccessorTestObject();

    handler = {};
    proxy = new Proxy(target, handler);
    handler.set = function (targ, key, val, receiver) {
        // If 'false' is returned, property write is rejected and the post-hook
        // behavior doesn't activate at all, so always return true here.
        return true;
    }
    Object.getOwnPropertyNames(target).forEach(function (propname) {
        // For accessor + 'set' hook, property value does not matter.
        setTest(proxy, propname, 'whatever');
    });
}

function deleteHookPostChecksTest() {
    var target, handler, proxy;

    function deleteTest(proxy, propname) {
        try {
            print(propname + ': success, result:', delete proxy[propname]);
        } catch (e) {
            print(propname + ':', e.name);
        }
    }

    /* 'deleteProperty' is rejected if the target has a property of the
     * same name and the property:
     *
     *   - Is not configurable
     */

    /*
     *  Data properties
     */

    target = makeDataTestObject();

    handler = {};
    proxy = new Proxy(target, handler);
    handler.deleteProperty = function (targ, key, val, receiver) {
        // If 'false' is returned, property delete is rejected and the post-hook
        // behavior doesn't activate at all, so always return true here.
        return true;
    }
    Object.getOwnPropertyNames(target).forEach(function (propname) {
        deleteTest(proxy, propname);
    });

    /*
     *  Accessor properties
     */

    target = makeAccessorTestObject();

    handler = {};
    proxy = new Proxy(target, handler);
    handler.deleteProperty = function (targ, key, val, receiver) {
        // If 'false' is returned, property delete is rejected and the post-hook
        // behavior doesn't activate at all, so always return true here.
        return true;
    }
    Object.getOwnPropertyNames(target).forEach(function (propname) {
        deleteTest(proxy, propname);
    });
}

print('hook post-checks');

try {
    print('get hook');
    getHookPostChecksTest();
    print('set hook');
    setHookPostChecksTest();
    print('deleteProperty hook');
    deleteHookPostChecksTest();
} catch (e) {
    print(e);
}

/*===
recursive proxies
TypeError
TypeError
===*/

/* Currently Duktape doesn't allow a proxy as either a handler or a target.
 * This makes it easier to implement because there is no arbitrary depth C
 * recursion when doing proxy lookups.
 */

function proxyHandlerTest() {
    var target = { foo: 123 };
    var handler = new Proxy({}, {});
    var proxy = new Proxy(target, handler);
}

function proxyTargetTest() {
    var target = new Proxy({}, {});
    var handler = {};
    var proxy = new Proxy(target, handler);
}

print('recursive proxies');

try {
    proxyHandlerTest();
} catch (e) {
    print(e.name);
}

try {
    proxyTargetTest();
} catch (e) {
    print(e.name);
}

/*===
getter handler
handler.get: true true foo true
proxy.foo: dummy-value
===*/

/* A getter as a handler property.  No reason why this wouldn't work but
 * test just in case.
 */

function getterHandlerTest() {
    var target = { foo: 123 };
    var handler = {};
    var proxy = new Proxy(target, handler);

    Object.defineProperty(handler, 'get', {
        enumerable: true,
        configurable: true,
        get: function () {
            return function (targ, key, receiver) {
                print('handler.get:', this === handler, targ === target, key, receiver === proxy);
                return 'dummy-value';
            };
        }
    });

    print('proxy.foo:', proxy.foo);
}

print('getter handler');

try {
    getterHandlerTest();
} catch (e) {
    print(e);
}

/*===
non-callable handler
TypeError
===*/

/* A non-callable handler property.  This is not checked during proxy creation
 * and should cause a TypeError.
 */

function nonCallableHandlerTest() {
    var target = { foo: 123 };
    var handler = { get: 'dummy' };
    var proxy = new Proxy(target, handler);
    print('proxy.foo:', proxy.foo);
}

print('non-callable handler');

try {
    nonCallableHandlerTest();
} catch (e) {
    print(e.name);
}

/*===
throwing handler
handler.get: about to throw
URIError: fake error
===*/

/* Handler function throws.  Nothing special here. */

function throwingHandlerTest() {
    var target = { foo: 123 };
    var handler = {
        get: function() {
            print('handler.get: about to throw');
            throw new URIError('fake error');
        }
    };
    var proxy = new Proxy(target, handler);
    print('proxy.foo:', proxy.foo);

}

print('throwing handler');

try {
    throwingHandlerTest();
} catch (e) {
    print(e);
}

/*===
proxy revocation
handler.get: true true foo true
proxy.foo: dummy-value
===*/

/* Revoked proxy. */

function proxyRevocationTest() {
    var target = { foo: 123 };
    var handler = {
        get: function(targ, key, receiver) {
            print('handler.get:', this === handler, targ === target, key, receiver === proxy);
            return 'dummy-value';
        }
    };
    var proxy = new Proxy(target, handler);
    print('proxy.foo:', proxy.foo);

    // XXX: unimplemented
}

print('proxy revocation');

try {
    proxyRevocationTest();
} catch (e) {
    print(e.name);
}
