function retTrue(val, key, obj) {
    print(typeof this, this, typeof val, val, typeof key, key, typeof obj, obj);
    return true;
}

function test(this_value, args) {
    var t;

    try {
        t = Array.prototype.every.apply(this_value, args);
        print(typeof t, t);
    } catch (e) {
        print(e.name);
    }
}

/*===
basic
boolean true
object [object global] number 1 number 0 object 1
boolean true
object [object global] number 1 number 0 object 1,2
object [object global] number 2 number 1 object 1,2
boolean true
object [object global] number 1 number 0 object 1,2,3,4,5
object [object global] number 2 number 1 object 1,2,3,4,5
object [object global] number 3 number 2 object 1,2,3,4,5
object [object global] number 4 number 3 object 1,2,3,4,5
object [object global] number 5 number 4 object 1,2,3,4,5
boolean true
object [object global] number 1 number 0 object 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,3
object [object global] number 2 number 50 object 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,3
object [object global] number 3 number 100 object 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,3
boolean true
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 5 object [object Object]
object [object global] string quux number 20 object [object Object]
boolean true
callback 0
callback 1
callback 2
callback 3
boolean false
callback 1
CallbackError
nonstrict object
boolean false
nonstrict object
boolean false
nonstrict object
boolean false
strict undefined undefined
boolean false
strict object null
boolean false
strict string foo
boolean false
===*/

print('basic');

function basicTest() {
    var obj;
    var count;

    // simple cases

    test([], [ retTrue ]);
    test([1], [ retTrue ]);
    test([1,2], [ retTrue ]);

    // dense

    test([1,2,3,4,5], [ retTrue ]);

    // sparse

    obj = [1];
    obj[100] = 3;
    obj[50] = 2;
    test(obj, [ retTrue ]);

    // non-array

    obj = { '0': 'foo', '5': 'bar', '20': 'quux', '100': 'baz', length: 35 };
    test(obj, [ retTrue ]);

    // first false terminates; return value is ToBoolean coerced (use ints here)

    count = 3;
    test([1,2,3,4,5,6,7,8,9,10], [ function(val, key, obj) {
        print('callback', key); if (count == 0) { return 0; }; count--; return 1;
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
        return false;
    }]);

    test([1,2,3], [ function(val, key, obj) {
        print('nonstrict', typeof this);
        return false;
    }, null]);

    test([1,2,3], [ function(val, key, obj) {
        print('nonstrict', typeof this);
        return false;
    }, 'foo']);

    test([1,2,3], [ function(val, key, obj) {
        'use strict';
        print('strict', typeof this, this);
        return false;
    }]);

    test([1,2,3], [ function(val, key, obj) {
        'use strict';
        print('strict', typeof this, this);
        return false;
    }, null]);  // Note: typeof null -> 'object'

    test([1,2,3], [ function(val, key, obj) {
        'use strict';
        print('strict', typeof this, this);
        return false;
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
boolean true
foo 0 foo,bar,quux
quux 2 foo,,quux
boolean true
foo 0 [object Object]
bar 1 [object Object]
quux 2 [object Object]
boolean true
foo 0 [object Object]
quux 2 [object Object]
boolean true
===*/

print('mutation');

function mutationTest() {
    var obj;

    // added element not recognized

    obj = [ 'foo', 'bar', 'quux' ];
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        obj[3] = 'baz';
        return true;
    }]);

    // deleted element not processed

    obj = [ 'foo', 'bar', 'quux' ];
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        delete obj[1];
        return true;
    }]);

    // same for non-array

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 3 };
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        obj[4] = 'quuux';
        obj.length = 10;
        return true;
    }]);

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 3 };
    test(obj, [ function (val, key, obj) {
        print(val, key, obj);
        delete obj[3]; delete obj[1];
        obj.length = 0;
        return true;
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
boolean true
boolean true
boolean true
object [object global] string f number 0 object foo
object [object global] string o number 1 object foo
object [object global] string o number 2 object foo
boolean true
object [object global] number 1 number 0 object 1,2,3
object [object global] number 2 number 1 object 1,2,3
object [object global] number 3 number 2 object 1,2,3
boolean true
boolean true
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
boolean true
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
boolean true
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
object [object global] string baz number 3 object [object Object]
boolean true
length valueOf
object [object global] string foo number 0 object [object Object]
object [object global] string bar number 1 object [object Object]
object [object global] string quux number 2 object [object Object]
boolean true
length valueOf
TypeError
callback 1 0 1,2,3,4,5,6,7,8,9,10
callback 2 1 1,2,3,4,5,6,7,8,9,10
callback 3 2 1,2,3,4,5,6,7,8,9,10
callback 4 3 1,2,3,4,5,6,7,8,9,10
boolean false
===*/

print('coercion');

function coercionTest() {
    var obj;

    // this

    test(undefined, [ retTrue ]);
    test(null, [ retTrue ]);
    test(true, [ retTrue ]);
    test(false, [ retTrue ]);
    test(123, [ retTrue ]);
    test('foo', [ retTrue ]);
    test([1,2,3], [ retTrue ]);
    test({ foo: 1, bar: 2 }, [ retTrue ]);

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

    // ToBoolean of callback return value

    test([1,2,3,4,5,6,7,8,9,10], [ function (val, key, obj) {
        print('callback', val, key, obj);
        if (key == 0) { return 1.0; }  /*true*/
        else if (key == 1) { return 'foo'; }  /*true*/
        else if (key == 2) {
            // Note: object is always 'true', no coercion related calls are made
            return {
                toString: function() { print('callback retval toString'); return 0; },
                valueOf: function() { print('callback retval valueOf'); return key == 1 ? '' /*false*/ : 'foo' /*true*/; }
            };
        } else {
            return '';  /*false*/
        }
    } ]);
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
