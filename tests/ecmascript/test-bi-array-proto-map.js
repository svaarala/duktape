// double a number or concatenate a string with itself
function retTwice(val, key, obj) {
    print(typeof this, this, typeof val, val, typeof key, key, typeof obj, obj);
    return val + val;
}

function test(this_value, args) {
    var t;

    try {
        t = Array.prototype.map.apply(this_value, args);
        print(typeof t, t);
    } catch (e) {
        print(e.name);
    }
}

/*===
basic
object 
object [object global] number 1 number 0 object 1
object 2
object [object global] number 1 number 0 object 1,2
object [object global] number 2 number 1 object 1,2
object 2,4
object [object global] number 1 number 0 object 1,2,3,4,5
object [object global] number 2 number 1 object 1,2,3,4,5
object [object global] number 3 number 2 object 1,2,3,4,5
object [object global] number 4 number 3 object 1,2,3,4,5
object [object global] number 5 number 4 object 1,2,3,4,5
object 2,4,6,8,10
object [object global] number 1 number 0 object 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,3
object [object global] number 2 number 50 object 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,3
object [object global] number 3 number 100 object 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,3
object 2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,4,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,6
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 5 object [object Object]
object [object global] string quux number 20 object [object Object]
object foofoo,,,,,barbar,,,,,,,,,,,,,,,quuxquux
callback 1
CallbackError
nonstrict object
nonstrict object
nonstrict object
object 101,102,103
nonstrict object
nonstrict object
nonstrict object
object 101,102,103
nonstrict object
nonstrict object
nonstrict object
object 101,102,103
strict undefined undefined
strict undefined undefined
strict undefined undefined
object 101,102,103
strict object null
strict object null
strict object null
object 101,102,103
strict string foo
strict string foo
strict string foo
object 101,102,103
callback 1 0 1,2,3,4,5,6,7,8,9,10
callback 2 1 1,2,3,4,5,6,7,8,9,10
callback 3 2 1,2,3,4,5,6,7,8,9,10
callback 4 3 1,2,3,4,5,6,7,8,9,10
callback 5 4 1,2,3,4,5,6,7,8,9,10
callback 6 5 1,2,3,4,5,6,7,8,9,10
callback 7 6 1,2,3,4,5,6,7,8,9,10
callback 8 7 1,2,3,4,5,6,7,8,9,10
callback 9 8 1,2,3,4,5,6,7,8,9,10
callback 10 9 1,2,3,4,5,6,7,8,9,10
object ,,true,false,123,foo,1,2,[object Object],,
===*/

print('basic');

function basicTest() {
    var obj;

    // simple cases

    test([], [ retTwice ]);
    test([1], [ retTwice ]);
    test([1,2], [ retTwice ]);

    // dense

    test([1,2,3,4,5], [ retTwice ]);

    // sparse

    obj = [1];
    obj[100] = 3;
    obj[50] = 2;
    test(obj, [ retTwice ]);

    // sparse with trailing non-existent elements: for this case standard and
    // real world behaviors differ, so this is covered by a separate test case:
    // test-bi-array-proto-map-nonstd-trailing.js

    // non-array; careful to avoid trailing non-existent elements (tested
    // separately)

    obj = { '0': 'foo', '5': 'bar', '20': 'quux', '100': 'baz', length: 21 };
    test(obj, [ retTwice ]);

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
        return val + 100;
    }]);

    test([1,2,3], [ function(val, key, obj) {
        print('nonstrict', typeof this);
        return val + 100;
    }, null]);

    test([1,2,3], [ function(val, key, obj) {
        print('nonstrict', typeof this);
        return val + 100;
    }, 'foo']);

    test([1,2,3], [ function(val, key, obj) {
        'use strict';
        print('strict', typeof this, this);
        return val + 100;
    }]);

    test([1,2,3], [ function(val, key, obj) {
        'use strict';
        print('strict', typeof this, this);
        return val + 100;
    }, null]);  // Note: typeof null -> 'object'

    test([1,2,3], [ function(val, key, obj) {
        'use strict';
        print('strict', typeof this, this);
        return val + 100;
    }, 'foo']);

    // return value is used as is; note that undefined and null both print
    // as empty string when array toString() is called

    var testvalues = [ undefined, null, true, false, 123, 'foo', [1,2], { foo: 1, bar: 2 } ];
    test([1,2,3,4,5,6,7,8,9,10], [ function (val, key, obj) {
        print('callback', val, key, obj);
        return testvalues[key];
    } ]);
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
object mapped-foo,mapped-bar,mapped-quux
foo 0 foo,bar,quux
quux 2 foo,,quux
object mapped-foo,,mapped-quux
foo 0 [object Object]
bar 1 [object Object]
quux 2 [object Object]
object mapped-foo,mapped-bar,mapped-quux
foo 0 [object Object]
quux 2 [object Object]
object mapped-foo,,mapped-quux
===*/

print('mutation');

function mutationTest() {
    var obj;

    // added element not recognized

    obj = [ 'foo', 'bar', 'quux' ];
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        obj[3] = 'baz';
        return 'mapped-' + val;
    }]);

    // deleted element not processed

    obj = [ 'foo', 'bar', 'quux' ];
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        delete obj[1];
        return 'mapped-' + val;
    }]);

    // same for non-array

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 3 };
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        obj[4] = 'quuux';
        obj.length = 10;
        return 'mapped-' + val;
    }]);

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 3 };
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        delete obj[3]; delete obj[1];
        obj.length = 0;
        return 'mapped-' + val;
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
object ff,oo,oo
object [object global] number 1 number 0 object 1,2,3
object [object global] number 2 number 1 object 1,2,3
object [object global] number 3 number 2 object 1,2,3
object 2,4,6
object 
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
object foofoo,barbar,quuxquux
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
object foofoo,barbar,quuxquux
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
object [object global] string baz number 3 object [object Object]
object foofoo,barbar,quuxquux,bazbaz
length valueOf
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
object foofoo,barbar,quuxquux
length valueOf
TypeError
===*/

print('coercion');

function coercionTest() {
    var obj;

    // this

    test(undefined, [ retTwice ]);
    test(null, [ retTwice ]);
    test(true, [ retTwice ]);
    test(false, [ retTwice ]);
    test(123, [ retTwice ]);
    test('foo', [ retTwice ]);
    test([1,2,3], [ retTwice ]);
    test({ foo: 1, bar: 2 }, [ retTwice ]);

    // length

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', '4': 'quux', length: '3.9' };
    test(obj, [ retTwice ]);
    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', '4': 'quux', length: 256*256*256*256 + 3.9 };  // coerces to 3
    test(obj, [ retTwice ]);
    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', '4': 'quux', length: -256*256*256*256 + 3.9 };  // coerces to 4
    test(obj, [ retTwice ]);

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
    test(obj, [ retTwice ]);

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
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
