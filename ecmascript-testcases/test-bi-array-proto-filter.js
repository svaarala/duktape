function retEvenIndices(val, key, obj) {
    print(typeof this, this, typeof val, val, typeof key, key, typeof obj, obj);
    return (Number(key) % 2) == 0;
}

function retTrue(val, key, obj) {
    print(typeof this, this, typeof val, val, typeof key, key, typeof obj, obj);
    return true;
}

function test(this_value, args) {
    var t;

    try {
        t = Array.prototype.filter.apply(this_value, args);
        print(typeof t, t);
    } catch (e) {
        print(e.name);
    }
}

/*===
basic
object 
object [object global] number 1 number 0 object 1
object 1
object [object global] number 1 number 0 object 1,2
object [object global] number 2 number 1 object 1,2
object 1
object [object global] number 1 number 0 object 1,2,3,4,5
object [object global] number 2 number 1 object 1,2,3,4,5
object [object global] number 3 number 2 object 1,2,3,4,5
object [object global] number 4 number 3 object 1,2,3,4,5
object [object global] number 5 number 4 object 1,2,3,4,5
object 1,3,5
object [object global] number 1 number 0 object 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,3,4,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,5
object [object global] number 2 number 50 object 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,3,4,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,5
object [object global] number 3 number 51 object 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,3,4,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,5
object [object global] number 4 number 52 object 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,3,4,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,5
object [object global] number 5 number 100 object 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,3,4,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,5
object 1,2,4,5
object [object global] number 1 number 0 object 1,2,,,3,4,,
object [object global] number 2 number 1 object 1,2,,,3,4,,
object [object global] number 3 number 4 object 1,2,,,3,4,,
object [object global] number 4 number 5 object 1,2,,,3,4,,
object 1,3
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 5 object [object Object]
object [object global] string quux number 20 object [object Object]
object foo,quux
callback 1
CallbackError
nonstrict object
nonstrict object
nonstrict object
object 2,3
nonstrict object
nonstrict object
nonstrict object
object 2,3
nonstrict object
nonstrict object
nonstrict object
object 2,3
strict undefined undefined
strict undefined undefined
strict undefined undefined
object 2,3
strict object null
strict object null
strict object null
object 2,3
strict string foo
strict string foo
strict string foo
object 2,3
===*/

print('basic');

function basicTest() {
    var obj;

    // simple cases

    test([], [ retEvenIndices ]);
    test([1], [ retEvenIndices ]);
    test([1,2], [ retEvenIndices ]);

    // dense

    test([1,2,3,4,5], [ retEvenIndices ]);

    // sparse

    obj = [1];
    obj[100] = 5;
    obj[50] = 2;
    obj[51] = 3;
    obj[52] = 4;
    test(obj, [ retEvenIndices ]);

    // trailing non-existent elements

    obj = [ 1, 2, , , 3, 4 , , , ];
    test(obj, [ retEvenIndices ]);

    // non-array

    obj = { '0': 'foo', '5': 'bar', '20': 'quux', '100': 'baz', length: 35 };
    test(obj, [ retEvenIndices ]);

    // error in callback propagates outwards

    test([1,2,3], [ function(val, key, obj) {
        var e;
        print('callback', val);
        e = new Error('callback error');
        e.name = 'CallbackError';
        throw e;
    }]);

    // this binding, non-strict callbacks gets a coerced binding

    test([1,2,3], [ function(val, key, obj) {
        print('nonstrict', typeof this);
        return val != 1;
    }]);

    test([1,2,3], [ function(val, key, obj) {
        print('nonstrict', typeof this);
        return val != 1;
    }, null]);

    test([1,2,3], [ function(val, key, obj) {
        print('nonstrict', typeof this);
        return val != 1;
    }, 'foo']);

    test([1,2,3], [ function(val, key, obj) {
        'use strict';
        print('strict', typeof this, this);
        return val != 1;
    }]);

    test([1,2,3], [ function(val, key, obj) {
        'use strict';
        print('strict', typeof this, this);
        return val != 1;
    }, null]);  // Note: typeof null -> 'object'

    test([1,2,3], [ function(val, key, obj) {
        'use strict';
        print('strict', typeof this, this);
        return val != 1;
    }, 'foo']);
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
mutation
foo 0 foo,bar,quux
bar 1 foo,bar,quux,baz
quux 2 foo,bar,quux,baz
object foo,quux
foo 0 foo,bar,quux
quux 2 foo,,quux
object foo,quux
foo 0 [object Object]
bar 1 [object Object]
quux 2 [object Object]
object foo,quux
foo 0 [object Object]
quux 2 [object Object]
object foo,quux
===*/

print('mutation');

function mutationTest() {
    var obj;

    // added element not recognized

    obj = [ 'foo', 'bar', 'quux' ];
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        obj[3] = 'baz';
        return val != 'bar';
    }]);

    // deleted element not processed

    obj = [ 'foo', 'bar', 'quux' ];
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        delete obj[1];
        return val != 'bar';
    }]);

    // same for non-array

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 3 };
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        obj[4] = 'quuux';
        obj.length = 10;
        return val != 'bar';
    }]);

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 3 };
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        delete obj[3]; delete obj[1];
        obj.length = 0;
        return val != 'bar';
    }]);
}

try {
    mutationTest();
} catch (e) {
    print(e);
}

/*===
coercion
TypeError
TypeError
object 
object 
object 
object [object global] string f number 0 object foo
object [object global] string o number 1 object foo
object [object global] string o number 2 object foo
object f,o
object [object global] number 1 number 0 object 1,2,3
object [object global] number 2 number 1 object 1,2,3
object [object global] number 3 number 2 object 1,2,3
object 1,3
object 
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
object foo,bar,quux
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
object foo,bar,quux
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
object [object global] string baz number 3 object [object Object]
object foo,bar,quux,baz
length valueOf
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
object foo,bar,quux
length valueOf
TypeError
TypeError
callback 1 0 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
callback 2 1 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
callback 3 2 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
callback 4 3 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
callback 5 4 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
callback 6 5 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
callback 7 6 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
callback 8 7 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
callback 9 8 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
callback 10 9 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
callback 11 10 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
callback 12 11 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
callback 13 12 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
callback 14 13 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
callback 15 14 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
object 3,6,8,9,10
===*/

print('coercion');

function coercionTest() {
    var obj;

    // this

    test(undefined, [ retEvenIndices ]);
    test(null, [ retEvenIndices ]);
    test(true, [ retEvenIndices ]);
    test(false, [ retEvenIndices ]);
    test(123, [ retEvenIndices ]);
    test('foo', [ retEvenIndices ]);
    test([1,2,3], [ retEvenIndices ]);
    test({ foo: 1, bar: 2 }, [ retEvenIndices ]);

    // length

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', '4': 'quux', length: '3.9' };
    test(obj, [ retTrue ]);
    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', '4': 'quux', length: 256*256*256*256 + 3.9 };  // coerces to 3
    test(obj, [ retTrue ]);
    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', '4': 'quux', length: -256*256*256*256 + 3.9 };  // coerces to 4
    test(obj, [ retTrue ]);

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', 'length': {
        toString: function() {
            print('length toString');
            return 4;
        },
        valueOf: function() {
            print('length valueOf');
            return 3;
        }
    }};
    test(obj, [ retTrue ]);

    // callable check is done after length coercion

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', 'length': {
        toString: function() {
            print('length toString');
            return 4;
        },
        valueOf: function() {
            print('length valueOf');
            return 3;
        }
    }};
    test(obj, [ null ]);

    // callable check is done even with no elements to process
    test([], [ null ]);

    // return value of callback is ToBoolean() coerced; this has no
    // side effects, but test each Ecmascript type

    var testvalues = [ undefined, null, true, false, 0, 123, '', 'foo', [1,2], { foo: 1, bar: 2 } ];
    test([1,2,3,4,5,6,7,8,9,10,11,12,13,14,15], [ function (val, key, obj) {
        print('callback', val, key, obj);
        return testvalues[key];
    } ]);
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
