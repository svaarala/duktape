/*
 *  Object.defineProperty() is pretty complex and intricate object property
 *  manipulation function.  The test cases here try to exercise all paths
 *  of the algorithm in E5.1 Section 8.12.9.
 *
 *  There is a separate brute force combination test.
 */

// XXX: util
function formatValue(v) {
    if (typeof v === 'function') {
        // avoid implementation dependent string formatting
        if (v.funcName) {
            return '[function ' + v.funcName + ']';
        } else {
            return Object.prototype.toString.call(v);
        }
    }
    if (typeof v === 'number') {
        if (v === 0) {
            if (1/v === Number.NEGATIVE_INFINITY) {
                // format negative zero specially to detect them in the output
                return '-0';
            }
        }
    }
    return String(v);
}

function getDesc(obj, prop) {
    var pd;

    if (typeof obj !== 'object' || obj === null) {
        // valueOf()
        return prop + ': non-object (' + Object.prototype.toString.call(obj) + ')';
    }

    // ToString(prop) coercion
    pd = Object.getOwnPropertyDescriptor(obj, prop);

    if (pd === undefined) {
        // valueOf()
        return prop + ': undefined';
    }

    // ToPrimitive(prop) coercion without hint -> valueOf()
    return prop + ': ' +
           'value=' + formatValue(pd.value) +
           ', writable=' + formatValue(pd.writable) +
           ', enumerable=' + formatValue(pd.enumerable) +
           ', configurable=' + formatValue(pd.configurable) +
           ', typeof(get)=' + formatValue(pd.get) +
           ', typeof(set)=' + formatValue(pd.set);
}

function printDesc(obj, prop) {
    print(getDesc(obj, prop));
}

function testDef(obj, prop, attrs, arg_count) {
    var t;

    // coercion side effects
    print('pre:  ' + getDesc(obj, prop));

    try {
        if (arg_count === 0) {
            t = Object.defineProperty();
        } else if (arg_count === 1) {
            t = Object.defineProperty(obj);
        } else if (arg_count === 2) {
            t = Object.defineProperty(obj, prop);
        } else {
            t = Object.defineProperty(obj, prop, attrs);
        }
        print(typeof t, formatValue(t));
    } catch (e) {
        print(e.name);
    }

    // coercion side effects
    print('post: ' + getDesc(obj, prop));
}


/*===
coercion
pre:  undefined: non-object ([object Undefined])
TypeError
post: undefined: non-object ([object Undefined])
pre:  foo: non-object ([object Undefined])
TypeError
post: foo: non-object ([object Undefined])
pre:  foo: non-object ([object Null])
TypeError
post: foo: non-object ([object Null])
pre:  foo: non-object ([object Boolean])
TypeError
post: foo: non-object ([object Boolean])
pre:  foo: non-object ([object Boolean])
TypeError
post: foo: non-object ([object Boolean])
pre:  foo: non-object ([object Number])
TypeError
post: foo: non-object ([object Number])
pre:  foo: non-object ([object String])
TypeError
post: foo: non-object ([object String])
pre:  foo: undefined
object 1,2
post: foo: value=undefined, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
toString() prop
valueOf() prop
pre:  bar: undefined
toString() prop
object [object Object]
toString() prop
valueOf() prop
post: bar: value=1, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
foo: value=1, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
bar: undefined
pre:  123: undefined
object [object Object]
post: 123: value=2, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
123: value=2, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
valueOf() prop
pre:  bar: non-object ([object Number])
TypeError
valueOf() prop
post: bar: non-object ([object Number])
===*/

/* Coercion of 'this' and property name. */

print('coercion');

function coercionTest() {
    var test = testDef;

    // Type(O) checks

    test(undefined, undefined, undefined, 0);
    test(undefined, 'foo', {});
    test(null, 'foo', {});
    test(true, 'foo', {});
    test(false, 'foo', {});
    test(123, 'foo', {});
    test('foo', 'foo', {});
    test([1,2], 'foo', {});
    test({ foo: 1, bar: 2 }, 'foo', {});

    // ToString(P)

    obj = {};
    test(obj, {
        toString: function() { print('toString() prop'); return 'foo'; },
        valueOf: function() { print('valueOf() prop'); return 'bar'; }
    }, { value: 1 }, 3);
    printDesc(obj, 'foo');
    printDesc(obj, 'bar');

    // ToString(123) -> '123'

    obj = {};
    test(obj, 123, { value: 2 });
    printDesc(obj, '123');

    // Type(O) check precedes ToString(P) coercion

    test(123, {
        toString: function() { print('toString() prop'); return 'foo'; },
        valueOf: function() { print('valueOf() prop'); return 'bar'; }
    }, { value: 1 });
}

try {
    coercionTest();
} catch (e) {
    print(e);
}

/*===
topropertydescriptor
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
object [object Object]
post: foo: value=foo, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=foo, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=foo, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=foo, writable=true, enumerable=false, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=foo, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=foo, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=undefined, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=null, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=true, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=false, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=123, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=foo, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=1,2, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=[object Object], writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=[object Function], writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: undefined
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=[object Function]
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=true, configurable=true, typeof(get)=[object Function], typeof(set)=[object Function]
pre:  foo: undefined
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=true, configurable=true, typeof(get)=[object Function], typeof(set)=undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=true, configurable=true, typeof(get)=[object Function], typeof(set)=[object Function]
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
TypeError
post: foo: undefined
pre:  foo: undefined
object [object Object]
post: foo: value=foo, writable=true, enumerable=false, configurable=true, typeof(get)=undefined, typeof(set)=undefined
===*/

/* ToPropertyDescriptor call (after this and property name coercion).
 * Detecting invalid descriptors.  E5.1 Section 8.10.5.
 */

print('topropertydescriptor');

function toPropertyDescriptorTest() {
    function test(desc) {
        var obj = {};
        testDef(obj, 'foo', desc);
    }

    // TypeError if not object
    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('foo');

    // Steps 3-8: ToBoolean() coercions.
    //
    // Note that ToBoolean() has no side effects so we can't verify that
    // it happens except by checking the resulting descriptor results.
    //
    //   undefined, null, NaN, +/-0, '' -> false
    //   otherwise -> true

    test({ value: 'foo', writable: undefined, enumerable: null, configurable: Number.NaN });
    test({ value: 'foo', writable: -0, enumerable: +0, configurable: '' });
    test({ value: 'foo', writable: Number.NEGATIVE_INFINITY, enumerable: Number.POSITIVE_INFINITY, configurable: 1 });
    test({ value: 'foo', writable: true, enumerable: false, configurable: 'x' });
    test({ value: 'foo', writable: '\u0000', enumerable: {}, configurable: [] });
    test({ value: 'foo', writable: function(){}, enumerable: /foo/, configurable: new Date() });

    // Step 5: no Value coercion.

    test({ value: undefined, writable: true, enumerable: true, configurable: true });
    test({ value: null, writable: true, enumerable: true, configurable: true });
    test({ value: true, writable: true, enumerable: true, configurable: true });
    test({ value: false, writable: true, enumerable: true, configurable: true });
    test({ value: 123, writable: true, enumerable: true, configurable: true });
    test({ value: 'foo', writable: true, enumerable: true, configurable: true });
    test({ value: [1,2], writable: true, enumerable: true, configurable: true });
    test({ value: { foo: 1, bar: 2 }, writable: true, enumerable: true, configurable: true });
    test({ value: function(){}, writable: true, enumerable: true, configurable: true });

    // Steps 7-8: get/set are checked to be callable.

    test({ get: undefined, set: function(){}, enumerable: true, configurable: true });
    test({ get: null, set: function(){}, enumerable: true, configurable: true });
    test({ get: true, set: function(){}, enumerable: true, configurable: true });
    test({ get: false, set: function(){}, enumerable: true, configurable: true });
    test({ get: 123, set: function(){}, enumerable: true, configurable: true });
    test({ get: 'foo', set: function(){}, enumerable: true, configurable: true });
    test({ get: [1,2], set: function(){}, enumerable: true, configurable: true });
    test({ get: { foo: 1, bar: 2 }, set: function(){}, enumerable: true, configurable: true });
    test({ get: function(){}, set: function(){}, enumerable: true, configurable: true });

    test({ set: undefined, get: function(){}, enumerable: true, configurable: true });
    test({ set: null, get: function(){}, enumerable: true, configurable: true });
    test({ set: true, get: function(){}, enumerable: true, configurable: true });
    test({ set: false, get: function(){}, enumerable: true, configurable: true });
    test({ set: 123, get: function(){}, enumerable: true, configurable: true });
    test({ set: 'foo', get: function(){}, enumerable: true, configurable: true });
    test({ set: [1,2], get: function(){}, enumerable: true, configurable: true });
    test({ set: { foo: 1, bar: 2 }, get: function(){}, enumerable: true, configurable: true });
    test({ set: function(){}, get: function(){}, enumerable: true, configurable: true });

    // Step 10, set/get vs. writable/value conflicts
    test({ set: function(){}, writable: true });
    test({ set: function(){}, value: 1 });
    test({ set: function(){}, writable: true, value: 1 });
    test({ get: function(){}, writable: true });
    test({ get: function(){}, value: 1 });
    test({ get: function(){}, writable: true, value: 1 });
    test({ get: function(){}, set: function(){}, writable: true });
    test({ get: function(){}, set: function(){}, value: 1 });
    test({ get: function(){}, set: function(){}, writable: true, value: 1 });

    // Additional properties are ignored
    test({ value: 'foo', writable: true, enumerable: false, configurable: true, unknown: 'property' });
}

try {
    toPropertyDescriptorTest();
} catch (e) {
    print(e);
}

/*===
return value
true
===*/

/* Return value of defineProperty() is the object itself.  Curiously,
 * this does not allow chained calls because defineProperty() is not
 * in Object.prototype.
 */

print('return value');

function returnValueTest() {
    var obj = {};

    print(Object.defineProperty(obj, 'foo', { value: 1 }) === obj);
}

try {
    returnValueTest();
} catch (e) {
    print(e);
}

/*===
no current, not extensible
pre:  foo: undefined
TypeError
post: foo: undefined
foo: undefined
===*/

/* Steps 1-3: no current value, object is not extensible. */

print('no current, not extensible');

function noCurrentNotExtensibleTest() {
    var obj = {};

    Object.seal(obj);

    testDef(obj, 'foo', { value: 1 });
    printDesc(obj, 'foo');
}

try {
    noCurrentNotExtensibleTest();
} catch (e) {
    print(e);
}

/*===
no current, is extensible
pre:  foo: undefined
object [object Object]
post: foo: value=1, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
foo: value=1, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
===*/

print('no current, is extensible');

/* Steps 1-4: no current value, object is extensible. */

function noCurrentIsExtensibleTest() {
    var obj = {};

    testDef(obj, 'foo', { value: 1 });

    // All other property attributes (writable, enumerable, configurable)
    // will have default values, i.e., false

    printDesc(obj, 'foo');
}

try {
    noCurrentIsExtensibleTest();
} catch (e) {
    print(e);
}

/*===
has current, desc empty
pre:  foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
===*/

/* Step 5: has current value, but descriptor is empty. */

print('has current, desc empty');

function hasCurrentDescEmptyTest() {
    var obj = { foo: 1 };

    testDef(obj, 'foo', {});

    // No changes; here property attributes will have default values
    // object literals (i.e. writable, configurable, enumerable are
    // all true).

    printDesc(obj, 'foo');
}

try {
    hasCurrentDescEmptyTest();
} catch (e) {
    print(e);
}

/*===
step6
pre:  foo: value=0, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=0, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=0, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
TypeError
post: foo: value=0, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=0, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
TypeError
post: foo: value=0, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=0, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
TypeError
post: foo: value=0, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=0, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
TypeError
post: foo: value=0, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=0, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=0, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
TypeError
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
TypeError
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
TypeError
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
TypeError
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
===*/

/* Step 6: has current value, descriptor is identical to current values,
 * when compared using SameValue (which distinguishes zero sign).
 */

print('step6');

function step6Test() {
    var obj;

    function fun1() {
    }

    function fun2() {
    }

    function fun3() {
    }

    var test = testDef;

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: +0, writable: false, enumerable: false, configurable: false
    });

    test(obj, 'foo', { value: +0, writable: false, enumerable: false, configurable: false });
    test(obj, 'foo', { value: -0, writable: false, enumerable: false, configurable: false });  // not allowed
    test(obj, 'foo', { value: +0, writable: true, enumerable: false, configurable: false });
    test(obj, 'foo', { value: +0, writable: false, enumerable: true, configurable: false });
    test(obj, 'foo', { value: +0, writable: false, enumerable: false, configurable: true });
    test(obj, 'foo', { value: +0, writable: '', enumerable: 0, configurable: Number.NaN });  // allowed, coercion happens before actual algorithm

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: fun1, set: fun2, enumerable: false, configurable: false
    });

    test(obj, 'foo', { get: fun1, set: fun2, enumerable: false, configurable: false });
    test(obj, 'foo', { get: fun3, set: fun2, enumerable: false, configurable: false });
    test(obj, 'foo', { get: fun1, set: fun3, enumerable: false, configurable: false });
    test(obj, 'foo', { get: fun1, set: fun2, enumerable: true, configurable: false });
    test(obj, 'foo', { get: fun1, set: fun2, enumerable: false, configurable: true });
}

try {
    step6Test();
} catch (e) {
    print(e);
}

/*===
step7
pre:  foo: value=1, writable=true, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
TypeError
post: foo: value=1, writable=true, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
TypeError
post: foo: value=1, writable=true, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=true, configurable=false, typeof(get)=undefined, typeof(set)=undefined
TypeError
post: foo: value=1, writable=true, enumerable=true, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=true, configurable=false, typeof(get)=undefined, typeof(set)=undefined
TypeError
post: foo: value=1, writable=true, enumerable=true, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
TypeError
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
TypeError
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
pre:  foo: value=undefined, writable=undefined, enumerable=true, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
TypeError
post: foo: value=undefined, writable=undefined, enumerable=true, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
pre:  foo: value=undefined, writable=undefined, enumerable=true, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
TypeError
post: foo: value=undefined, writable=undefined, enumerable=true, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
===*/

/* Step 7: has current value, descriptor is not identical, current is not
 * configurable, and attempt to (7.a) change property to configurable; or
 * (7.b) change enumerability status.
 */

print('step7');

function step7Test() {
    var obj;

    var test = testDef;

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: false, configurable: false
    });
    test(obj, 'foo', { configurable: true });
    test(obj, 'foo', { enumerable: true });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: true, configurable: false
    });
    test(obj, 'foo', { configurable: true });
    test(obj, 'foo', { enumerable: false });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: function(){}, set: function(){}, enumerable: false, configurable: false
    });
    test(obj, 'foo', { configurable: true });
    test(obj, 'foo', { enumerable: true });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: function(){}, set: function(){}, enumerable: true, configurable: false
    });
    test(obj, 'foo', { configurable: true });
    test(obj, 'foo', { enumerable: false });
}

try {
    step7Test();
} catch (e) {
    print(e);
}

/*===
step8
pre:  foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=1, writable=true, enumerable=true, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=1, writable=true, enumerable=false, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=false, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=1, writable=true, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=false, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=1, writable=true, enumerable=true, configurable=false, typeof(get)=undefined, typeof(set)=undefined
===*/

/* Step 8: has current value, descriptor is not identical, descriptor is
 * generic (has no value, writable, get, or set; i.e. may only contain
 * configurable and enumerable).
 *
 * Step 7 already covered some cases; cases left are:
 *
 *   - previous is configurable, change configurability from true->false
 *   - previous is configurable, change enumerability from true->false or
 *     false->true
 *   - previous is configurable, change configurability from true->false
 *     and enumerability from true->false or false->true.
 *
 * Non-configurable cases (for generic descriptors) are already covered
 * fully; step 7 rejects any attempts to change configurability and
 * enumerability, so previous and new configurable and enumerable must
 * have the same values -- but that case is caught by step 6.
 */

print('step8');

function step8Test() {
    var obj;

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: true, configurable: true
    });
    testDef(obj, 'foo', { configurable: false });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: true, configurable: true
    });
    testDef(obj, 'foo', { enumerable: false });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: false, configurable: true
    });
    testDef(obj, 'foo', { enumerable: true });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: true, configurable: true
    });
    testDef(obj, 'foo', { configurable: false, enumerable: false });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: false, configurable: true
    });
    testDef(obj, 'foo', { configurable: false, enumerable: true });
}

try {
    step8Test();
} catch (e) {
    print(e);
}

/*===
step9
pre:  foo: value=1, writable=true, enumerable=true, configurable=false, typeof(get)=undefined, typeof(set)=undefined
TypeError
post: foo: value=1, writable=true, enumerable=true, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=undefined, writable=undefined, enumerable=true, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
TypeError
post: foo: value=undefined, writable=undefined, enumerable=true, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
pre:  foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[object Function], typeof(set)=[object Function]
pre:  foo: value=undefined, writable=undefined, enumerable=true, configurable=true, typeof(get)=[object Function], typeof(set)=[object Function]
object [object Object]
post: foo: value=1, writable=true, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=true, configurable=true, typeof(get)=[object Function], typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=[object Function]
pre:  foo: value=undefined, writable=undefined, enumerable=true, configurable=true, typeof(get)=[object Function], typeof(set)=[object Function]
object [object Object]
post: foo: value=undefined, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=undefined, writable=undefined, enumerable=true, configurable=true, typeof(get)=[object Function], typeof(set)=[object Function]
object [object Object]
post: foo: value=123, writable=false, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
===*/

/* Step 9: converting an accessor property to data property, or vice versa.
 * If existing property non-configurable, always rejected.
 */

print('step9');

function step9Test() {
    var obj;

    // non-configurable cases

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: true, configurable: false
    });
    testDef(obj, 'foo', { get: function(){}, set: function(){} });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: function(){}, set: function(){}, enumerable: true, configurable: false
    });
    testDef(obj, 'foo', { value: 1 });

    // configurable case: all attributes specified

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: true, configurable: true
    });
    testDef(obj, 'foo', { get: function(){}, set: function(){}, enumerable: false, configurable: false });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: function(){}, set: function(){}, enumerable: true, configurable: true
    });
    testDef(obj, 'foo', { value: 1, writable: true, enumerable: false, configurable: false });

    // configurable case: attribute defaults

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: true, configurable: true
    });
    testDef(obj, 'foo', { get: function(){} });  // set defaults to undefined

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: true, configurable: true
    });
    testDef(obj, 'foo', { set: function(){} });  // get defaults to undefined

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: function(){}, set: function(){}, enumerable: true, configurable: true
    });
    testDef(obj, 'foo', { writable: true });  // value defaults to undefined

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: function(){}, set: function(){}, enumerable: true, configurable: true
    });
    testDef(obj, 'foo', { value: 123 });  // writable defaults to false
}

try {
    step9Test();
} catch (e) {
    print(e);
}

/*===
step10
pre:  foo: value=1, writable=true, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=1, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
TypeError
post: foo: value=1, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=1, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
TypeError
post: foo: value=1, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=0, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
TypeError
post: foo: value=0, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=false, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=1, writable=false, enumerable=false, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=false, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=false, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=1, writable=true, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=false, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=2, writable=true, enumerable=false, configurable=true, typeof(get)=undefined, typeof(set)=undefined
pre:  foo: value=1, writable=true, enumerable=false, configurable=true, typeof(get)=undefined, typeof(set)=undefined
object [object Object]
post: foo: value=2, writable=false, enumerable=true, configurable=false, typeof(get)=undefined, typeof(set)=undefined
===*/

/* Step 10: modifying a data property.  If existing property non-configurable,
 * allow change from writable to non-writable (but not vice versa), and no-op
 * if new and old values match with SameValue (otherwise reject).
 */

print('step10');

function step10Test() {
    var obj;

    // non-configurable: change from writable to non-writable (allowed)

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: false, configurable: false
    });
    testDef(obj, 'foo', { writable: false });

    // non-configurable: attempt to change from non-writable to writable (rejected)

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: false, enumerable: false, configurable: false
    });
    testDef(obj, 'foo', { writable: true });

    // non-configurable: value is SameValue (allowed)

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: false, enumerable: false, configurable: false
    });
    testDef(obj, 'foo', { value: 1 });

    // non-configurable: value is not SameValue (rejected)

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: false, enumerable: false, configurable: false
    });
    testDef(obj, 'foo', { value: 2 });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: +0, writable: false, enumerable: false, configurable: false
    });
    testDef(obj, 'foo', { value: -0 });  // rejected

    // configurable -> allow all changes

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: false, configurable: true
    });
    testDef(obj, 'foo', { writable: false });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: false, configurable: true
    });
    testDef(obj, 'foo', { enumerable: true });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: false, configurable: true
    });
    testDef(obj, 'foo', { configurable: false });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: false, configurable: true
    });
    testDef(obj, 'foo', { value: 2 });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 1, writable: true, enumerable: false, configurable: true
    });
    testDef(obj, 'foo', { value: 2, writable: false, enumerable: true, configurable: false }); // all at the same time

    // Note: enumerability changes handled in earlier cases (steps 7-8)
}

try {
    step10Test();
} catch (e) {
    print(e);
}

/*===
step11
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
TypeError
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
TypeError
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
TypeError
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
TypeError
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
TypeError
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=true, typeof(get)=[function f1], typeof(set)=[function f2]
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=true, typeof(get)=[function f3], typeof(set)=[function f2]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=true, typeof(get)=[function f1], typeof(set)=[function f2]
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=true, typeof(get)=[function f1], typeof(set)=[function f3]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=true, typeof(get)=[function f1], typeof(set)=[function f2]
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=true, configurable=true, typeof(get)=[function f1], typeof(set)=[function f2]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=true, typeof(get)=[function f1], typeof(set)=[function f2]
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=false, configurable=false, typeof(get)=[function f1], typeof(set)=[function f2]
pre:  foo: value=undefined, writable=undefined, enumerable=false, configurable=true, typeof(get)=[function f1], typeof(set)=[function f2]
object [object Object]
post: foo: value=undefined, writable=undefined, enumerable=true, configurable=false, typeof(get)=[function f3], typeof(set)=[function f3]
===*/

/* Step 11: modifying an accessor property.  If existing property
 * non-configurable, no-op if both set and/or get (whatever is present)
 * match existing values with SameValue (otherwise reject).
 */

print('step11');

function step11Test() {
    var obj;

    function f1() { }
    function f2() { }
    function f3() { }

    f1.funcName = 'f1';
    f2.funcName = 'f2';
    f3.funcName = 'f3';

    // non-configurable: allow get/set if they are the same

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: f1, set: f2, enumerable: false, configurable: false
    });
    testDef(obj, 'foo', { set: f2 });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: f1, set: f2, enumerable: false, configurable: false
    });
    testDef(obj, 'foo', { get: f1 });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: f1, set: f2, enumerable: false, configurable: false
    });
    testDef(obj, 'foo', { get: f1, set: f2 });

    // non-configurable: reject if get/set and not same

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: f1, set: f2, enumerable: false, configurable: false
    });
    testDef(obj, 'foo', { get: f2 });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: f1, set: f2, enumerable: false, configurable: false
    });
    testDef(obj, 'foo', { set: f1 });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: f1, set: f2, enumerable: false, configurable: false
    });
    testDef(obj, 'foo', { get: f1, set: f3 });  // get OK, set not

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: f1, set: f2, enumerable: false, configurable: false
    });
    testDef(obj, 'foo', { get: f3, set: f2 });  // set OK, get not

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: f1, set: f2, enumerable: false, configurable: false
    });
    testDef(obj, 'foo', { get: f3, set: f3 });

    // configurable: accept all changes

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: f1, set: f2, enumerable: false, configurable: true
    });
    testDef(obj, 'foo', { get: f3 });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: f1, set: f2, enumerable: false, configurable: true
    });
    testDef(obj, 'foo', { set: f3 });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: f1, set: f2, enumerable: false, configurable: true
    });
    testDef(obj, 'foo', { enumerable: true });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: f1, set: f2, enumerable: false, configurable: true
    });
    testDef(obj, 'foo', { configurable: false });

    obj = {};
    Object.defineProperty(obj, 'foo', {
        get: f1, set: f2, enumerable: false, configurable: true
    });
    testDef(obj, 'foo', { get: f3, set: f3, enumerable: true, configurable: false });
}

try {
    step11Test();
} catch (e) {
    print(e);
}

/*===
step12
===*/

/* Step 12-13: modify attributes, return true if not rejected. */

print('step12');

function step12Test() {
    var obj;

    // XXX: anything to test here which hasn't been covered above?
}

try {
    step12Test();
} catch (e) {
    print(e);
}

// XXX: test special behavior (like arrays, arguments object, etc)
