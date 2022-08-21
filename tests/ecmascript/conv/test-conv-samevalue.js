/*
 *  SameValue() (E5 Section 9.12).
 *
 *  SameValue() is difficult to test indirectly.  It appears in E5 Section
 *  8.12.9, [[DefineOwnProperty]] several times.
 *
 *  One relatively simple approach is to create a non-configurable, non-writable
 *  property, and attempt to use Object.defineProperty() to set a new value for
 *  the property.  If SameValue(oldValue,newValue), no exception is thrown.
 *  Otherwise, reject (TypeError); see E5 Section 8.12.9, step 10.a.ii.1.
 */

function sameValue(x,y) {
    var obj = {};

    try {
        Object.defineProperty(obj, 'test', {
            writable: false,
            enumerable: false,
            configurable: false,
            value: x
        });

        Object.defineProperty(obj, 'test', {
            value: y
        });
    } catch (e) {
        if (e.name === 'TypeError') {
            return false;
        } else {
            throw e;
        }
    }

    return true;
}

function test(x,y) {
    print(sameValue(x,y));
}

/*===
test: different types, first undefined
false
false
false
false
false
false
===*/

/* Different types, first is undefined */

print('test: different types, first undefined')
test(undefined, null);
test(undefined, true);
test(undefined, false);
test(undefined, 123.0);
test(undefined, 'foo');
test(undefined, {});

/*===
test: different types, first null
false
false
false
false
false
false
===*/

/* Different types, first is null */

print('test: different types, first null')
test(null, undefined);
test(null, true);
test(null, false);
test(null, 123.0);
test(null, 'foo');
test(null, {});

/*===
test: different types, first boolean
false
false
false
false
false
false
false
false
false
false
===*/

/* Different types, first is boolean */

print('test: different types, first boolean')
test(true, undefined);
test(true, null);
test(true, 123.0);
test(true, 'foo');
test(true, {});

test(false, undefined);
test(false, null);
test(false, 123.0);
test(false, 'foo');
test(false, {});

/*===
test: different types, first number
false
false
false
false
false
false
===*/

/* Different types, first is number */

print('test: different types, first number')
test(123.0, undefined);
test(123.0, null);
test(123.0, true);
test(123.0, false);
test(123.0, 'foo');
test(123.0, {});

/*===
test: different types, first string
false
false
false
false
false
false
===*/

/* Different types, first is string */

print('test: different types, first string')
test('foo', undefined);
test('foo', null);
test('foo', true);
test('foo', false);
test('foo', 123.0);
test('foo', {});

/*===
test: different types, first object
false
false
false
false
false
false
===*/

/* Different types, first is object */

print('test: different types, first object')
test({}, undefined);
test({}, null);
test({}, true);
test({}, false);
test({}, 123.0);
test({}, 'foo');

/*===
test: same types, undefined
true
===*/

/* Same types: undefined */

print('test: same types, undefined')
test(undefined, undefined);

/*===
test: same types, null
true
===*/

/* Same types: null */

print('test: same types, null')
test(null, null);

/*===
test: same types, boolean
true
false
false
true
===*/

/* Same types: boolean */

print('test: same types, boolean')
test(true, true);
test(true, false);
test(false, true);
test(false, false);

/*===
test: same types, number
true
true
false
false
true
true
false
false
true
true
true
===*/

/* Same types: number */

print('test: same types, number')
test(NaN, NaN);
test(-0, -0);
test(-0, +0);
test(+0, -0);
test(+0, +0);
test(Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY);
test(Number.NEGATIVE_INFINITY, Number.POSITIVE_INFINITY);
test(Number.POSITIVE_INFINITY, Number.NEGATIVE_INFINITY);
test(Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY);
test(-123.0, -123.0);
test(123.0, 123.0);

/*===
test: same types, string
true
false
false
true
===*/

/* Same types: string */

print('test: same types, string')
test('', '');
test('foo', '')
test('', 'foo');
test('foo', 'foo');

/*===
test: same types, object
true
false
false
true
===*/

/* Same types: object */

var obj1 = {};
var obj2 = {};

print('test: same types, object')
test(obj1, obj1);
test(obj1, obj2);
test(obj2, obj1);
test(obj2, obj2);
