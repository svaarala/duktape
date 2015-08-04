function retFalse(val, key, obj) {
    print(typeof this, this, typeof val, val, typeof key, key, typeof obj, obj);
    return false;
}

function test(this_value, args) {
    var t;

    try {
        t = Array.prototype.some.apply(this_value, args);
        print(typeof t, t);
    } catch (e) {
        print(e.name);
    }
}

/*===
basic
boolean false
object [object global] number 1 number 0 object 1
boolean false
object [object global] number 1 number 0 object 1,2
object [object global] number 2 number 1 object 1,2
boolean false
object [object global] number 1 number 0 object 1,2,3,4,5
object [object global] number 2 number 1 object 1,2,3,4,5
object [object global] number 3 number 2 object 1,2,3,4,5
object [object global] number 4 number 3 object 1,2,3,4,5
object [object global] number 5 number 4 object 1,2,3,4,5
boolean false
object [object global] number 1 number 0 object 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,3
object [object global] number 2 number 50 object 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,3
object [object global] number 3 number 100 object 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,3
boolean false
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 5 object [object Object]
object [object global] string quux number 20 object [object Object]
boolean false
callback 0
callback 1
callback 2
callback 3
boolean true
callback 1
CallbackError
nonstrict object
boolean true
nonstrict object
boolean true
nonstrict object
boolean true
strict undefined undefined
boolean true
strict object null
boolean true
strict string foo
boolean true
===*/

print('basic');

function basicTest() {
    var obj;
    var count;

    // simple cases

    test([], [ retFalse ]);
    test([1], [ retFalse ]);
    test([1,2], [ retFalse ]);

    // dense

    test([1,2,3,4,5], [ retFalse ]);

    // sparse

    obj = [1];
    obj[100] = 3;
    obj[50] = 2;
    test(obj, [ retFalse ]);

    // non-array

    obj = { '0': 'foo', '5': 'bar', '20': 'quux', '100': 'baz', length: 35 };
    test(obj, [ retFalse ]);

    // first true terminates; return value is ToBoolean coerced (use ints here)

    count = 3;
    test([1,2,3,4,5,6,7,8,9,10], [ function(val, key, obj) {
        print('callback', key); if (count == 0) { return 1; }; count--; return 0;
    }]);

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
        return true;
    }]);

    test([1,2,3], [ function(val, key, obj) {
        print('nonstrict', typeof this);
        return true;
    }, null]);

    test([1,2,3], [ function(val, key, obj) {
        print('nonstrict', typeof this);
        return true;
    }, 'foo']);

    test([1,2,3], [ function(val, key, obj) {
        'use strict';
        print('strict', typeof this, this);
        return true;
    }]);

    test([1,2,3], [ function(val, key, obj) {
        'use strict';
        print('strict', typeof this, this);
        return true;
    }, null]);  // Note: typeof null -> 'object'

    test([1,2,3], [ function(val, key, obj) {
        'use strict';
        print('strict', typeof this, this);
        return true;
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
boolean false
foo 0 foo,bar,quux
quux 2 foo,,quux
boolean false
foo 0 [object Object]
bar 1 [object Object]
quux 2 [object Object]
boolean false
foo 0 [object Object]
quux 2 [object Object]
boolean false
===*/

print('mutation');

function mutationTest() {
    var obj;

    // added element not recognized

    obj = [ 'foo', 'bar', 'quux' ];
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        obj[3] = 'baz';
        return false;
    }]);

    // deleted element not processed

    obj = [ 'foo', 'bar', 'quux' ];
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        delete obj[1];
        return false;
    }]);

    // same for non-array

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 3 };
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        obj[4] = 'quuux';
        obj.length = 10;
        return false;
    }]);

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 3 };
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        delete obj[3]; delete obj[1];
        obj.length = 0;
        return false;
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
boolean false
boolean false
boolean false
object [object global] string f number 0 object foo
object [object global] string o number 1 object foo
object [object global] string o number 2 object foo
boolean false
object [object global] number 1 number 0 object 1,2,3
object [object global] number 2 number 1 object 1,2,3
object [object global] number 3 number 2 object 1,2,3
boolean false
boolean false
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
boolean false
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
boolean false
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
object [object global] string baz number 3 object [object Object]
boolean false
length valueOf
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
boolean false
length valueOf
TypeError
callback 1 0 1,2,3,4,5,6,7,8,9,10
callback 2 1 1,2,3,4,5,6,7,8,9,10
callback 3 2 1,2,3,4,5,6,7,8,9,10
boolean true
===*/

print('coercion');

function coercionTest() {
    var obj;

    // this

    test(undefined, [ retFalse ]);
    test(null, [ retFalse ]);
    test(true, [ retFalse ]);
    test(false, [ retFalse ]);
    test(123, [ retFalse ]);
    test('foo', [ retFalse ]);
    test([1,2,3], [ retFalse ]);
    test({ foo: 1, bar: 2 }, [ retFalse ]);

    // length

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', '4': 'quux', length: '3.9' };
    test(obj, [ retFalse ]);
    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', '4': 'quux', length: 256*256*256*256 + 3.9 };  // coerces to 3
    test(obj, [ retFalse ]);
    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', '4': 'quux', length: -256*256*256*256 + 3.9 };  // coerces to 4
    test(obj, [ retFalse ]);

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
    test(obj, [ retFalse ]);

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

    // ToBoolean of callback return value

    test([1,2,3,4,5,6,7,8,9,10], [ function (val, key, obj) {
        print('callback', val, key, obj);
        if (key == 0) { return 0.0; }  /*false*/
        else if (key == 1) { return ''; }  /*false*/
        else if (key == 2) {
            // Note: object is always 'true', no coercion related calls are made
            return {
                toString: function() { print('callback retval toString'); return 0; },
                valueOf: function() { print('callback retval valueOf'); return key == 1 ? '' /*false*/ : 'foo' /*true*/; }
            };
        }
    } ]);
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
