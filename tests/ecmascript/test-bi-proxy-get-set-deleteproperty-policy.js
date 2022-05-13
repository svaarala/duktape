/*===
trap post-checks
get trap
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
set trap
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
deleteProperty trap
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
getter handler
handler.get: true true foo true
proxy.foo: dummy-value
non-callable handler
TypeError
throwing handler
handler.get: about to throw
URIError
done
===*/

function makeDataTestObject() {
    var obj = {};

    Object.defineProperties(obj, {
        // Various properties whose behavior to compare against the traps.

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
        // Various properties whose behavior to compare against the traps
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
     *     trap provided value does not match with the current value
     *     (as compared with SameValue).
     *
     *   - Is an accessor property, is not configurable, getter is not
     *     defined, and trap provided value is not undefined.
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
        // If trapResult is undefined, post-trap checks always pass
        return undefined;
    }
    Object.getOwnPropertyNames(target).forEach(function (propname) {
        getTest(proxy, propname);
    });

    handler = {};
    proxy = new Proxy(target, handler);
    handler.get = function (targ, key, receiver) {
        // If trapResult is not undefined, post-trap checks cause a TypeError
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
        // If 'false' is returned, property write is rejected and the post-trap
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
        // If 'false' is returned, property write is rejected and the post-trap
        // behavior doesn't activate at all, so always return true here.
        return true;
    }
    Object.getOwnPropertyNames(target).forEach(function (propname) {
        // For accessor + 'set' trap, property value does not matter.
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
        // If 'false' is returned, property delete is rejected and the post-trap
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
        // If 'false' is returned, property delete is rejected and the post-trap
        // behavior doesn't activate at all, so always return true here.
        return true;
    }
    Object.getOwnPropertyNames(target).forEach(function (propname) {
        deleteTest(proxy, propname);
    });
}

print('trap post-checks');

try {
    print('get trap');
    getHookPostChecksTest();
    print('set trap');
    setHookPostChecksTest();
    print('deleteProperty trap');
    deleteHookPostChecksTest();
} catch (e) {
    print(e);
}

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


/* A non-callable handler property.  This is not checked during proxy creation
 * and should cause a TypeError later.
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

/* Handler function throws.  Error propagates out without being caught automatically. */

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
    print(e.name);
}

print('done');
