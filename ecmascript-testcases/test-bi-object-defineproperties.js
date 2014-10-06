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
        return prop + ': non-object (' + Object.prototype.toString.call(obj) + ')';
    }

    pd = Object.getOwnPropertyDescriptor(obj, prop);

    if (pd === undefined) {
        return prop + ': undefined';
    }

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

/*===
coercion
[object Undefined] [object Undefined] TypeError
[object Undefined] [object Object] TypeError
[object Null] [object Object] TypeError
[object Boolean] [object Object] TypeError
[object Boolean] [object Object] TypeError
[object Number] [object Object] TypeError
[object String] [object Object] TypeError
[object Array] [object Object] ok
[object Object] [object Object] ok
[object Object] [object Undefined] TypeError
[object Object] [object Undefined] TypeError
[object Object] [object Null] TypeError
[object Object] [object Boolean] ok
[object Object] [object Boolean] ok
[object Object] [object Number] ok
[object Object] [object String] ok
[object Object] [object Array] ok
[object Object] [object Object] ok
===*/

/* Test coercion of 'O' and 'Properties' arguments. */

print('coercion');

function coercionTest() {
    function test(o, p, arg_count) {
        var t;

        try {
            if (arg_count === 0) {
                t = Object.defineProperties();
            } else if (arg_count === 1) {
                t = Object.defineProperties(o);
            } else {
                t = Object.defineProperties(o, p);
            }
            print(Object.prototype.toString.call(o),
                  Object.prototype.toString.call(p), 'ok');
        } catch (e) {
            print(Object.prototype.toString.call(o),
                  Object.prototype.toString.call(p), e.name);
        }
    }

    // coercion of 'O'
    test(undefined, undefined, 0);
    test(undefined, {});
    test(null, {});
    test(true, {});
    test(false, {});
    test(123, {});
    test('quux', {});
    test([1,2], {});
    test({ foo: 1, bar: 2 }, {});

    // coercion of 'P'
    test({}, undefined, 1);
    test({}, undefined);
    test({}, null);
    test({}, true);
    test({}, false);
    test({}, 123);
    test({}, '');  // <-- careful to avoid TypeError from ToPropertyDescriptor()
    test({}, []);  //     (use empty string, array, object)
    test({}, {});  //

    // coercion of 'O' and 'P'; 'O' is checked first -- how to test this?
}

try {
    coercionTest();
} catch (e) {
    print(e);
}

/*===
proplist
prop1
prop2
proto-prop
proto-prop: undefined
nonenum-prop: undefined
prop1: value=prop1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
prop2: value=prop2, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
prop1
prop2
===*/

/* Property list is created from own, enumerable properties of 'Properties'
 * argument.
 *
 * This test expects enum order to match insertion order (but the testcase
 * is not marked custom).
 */

print('proplist');

function propListTest() {
    var proto = {};
    var props;
    var obj;
    var i;

    props = Object.create(proto);

    // enumerable, but not 'own property'
    Object.defineProperty(proto, 'proto-prop', {
        value: { value: 'from-proto', writable: true, enumerable: true, configurable: true },
        writable: true, enumerable: true, configurable: true
    });

    // own property but not enumerable
    Object.defineProperty(props, 'nonenum-prop', {
        value: { value: 'nonenum', writable: true, enumerable: true, configurable: true },
        writable: true, enumerable: false, configurable: true
    });

    // own property, enumerable -> will appear
    Object.defineProperty(props, 'prop1', {
        value: { value: 'prop1', writable: true, enumerable: true, configurable: true },
        writable: true, enumerable: true, configurable: true
    });
    Object.defineProperty(props, 'prop2', {
        value: { value: 'prop2', writable: true, enumerable: true, configurable: true },
        writable: false, enumerable: true, configurable: false
    });

    // for-in will enumerate also inherited properties
    for (i in props) {
        print(i);
    }

    obj = {};
    Object.defineProperties(obj, props);

    printDesc(obj, 'proto-prop');
    printDesc(obj, 'nonenum-prop');
    printDesc(obj, 'prop1');
    printDesc(obj, 'prop2');

    // prop1 and prop2 addition order should match order in 'props'
    for (i in obj) {
        print(i);
    }
}

try {
    propListTest();
} catch (e) {
    print(e);
}

/*===
topropdesc
foo: value=2, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
TypeError
foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
===*/

/* All ToPropertyDescriptor() coercions and checks happen before any object
 * properties/attributes are changed.
 */

print('topropdesc');

function toPropDescTest() {
    var obj;

    // base case: works
    obj = { foo: 1 };
    Object.defineProperties(obj, {
        foo: { value: 2 }
    });
    printDesc(obj, 'foo');

    // invalid desc -> nothing is changed
    obj = { foo: 1 };
    try {
        Object.defineProperties(obj, {
            foo: { value: 3 },  // ok
            bar: { set: function(){}, value: 123 }  // conflict
        });
    } catch (e) {
        print(e.name);
    }
    printDesc(obj, 'foo');
}

try {
    toPropDescTest();
} catch (e) {
    print(e);
}

/*===
multiple
foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
bar: value=2, writable=false, enumerable=true, configurable=false, typeof(get)=undefined, typeof(set)=undefined
foo
bar
TypeError
foo: value=1, writable=true, enumerable=true, configurable=true, typeof(get)=undefined, typeof(set)=undefined
bar: value=immutable, writable=false, enumerable=false, configurable=false, typeof(get)=undefined, typeof(set)=undefined
foo
===*/

/* Multiple changes in one go.  Note that since 'Properties' is an Object,
 * the same property cannot be edited twice in one defineProperties() call.
 *
 * Unlike the ToPropertyDescriptor() step, the first TypeError (or other
 * error) causes defineProperties() to bail out.
 */

print('multiple');

function multiplePropsTest() {
    var obj;
    var i;

    // base case
    obj = {};
    Object.defineProperties(obj, {
        foo: { value: 1, writable: true, enumerable: true, configurable: true },
        bar: { value: 2, writable: false, enumerable: true, configurable: false }
    });
    printDesc(obj, 'foo');
    printDesc(obj, 'bar');
    for (i in obj) {
        // demonstrate enum order
        print(i);
    }

    // first edit bails out, so second one is not made
    obj = {};
    Object.defineProperty(obj, 'foo', {
        value: 'configurable', writable: false, enumerable: false, configurable: true
    });
    Object.defineProperty(obj, 'bar', {
        value: 'immutable', writable: false, enumerable: false, configurable: false
    });
    try {
        Object.defineProperties(obj, {
            foo: { value: 1, writable: true, enumerable: true, configurable: true },
            bar: { value: 2, writable: false, enumerable: true, configurable: false }
        });
    } catch (e) {
        print(e.name);
    }
    printDesc(obj, 'foo');
    printDesc(obj, 'bar');
    for (i in obj) {
        // demonstrate enum order
        print(i);
    }
}

try {
    multiplePropsTest();
} catch (e) {
    print(e);
}

/*===
return
true
===*/

/* Return value is 'O'. */

print('return');

function returnValueTest() {
    var obj = {};
    var ret;

    ret = Object.defineProperties(obj, {
        foo: { value: 1, writable: true, enumerable: true, configurable: true }
    });
    print(ret === obj);
}

try {
    returnValueTest();
} catch (e) {
    print(e);
}
