/*---
{
    "skip": true
}
---*/

function test(this_value) {
    var t;

    try {
        var t = Array.prototype.toLocaleString.call(this_value);
        print(typeof t, t);
    } catch (e) {
        print(e.name);
    }
}

/*===
basic
string 
string 1,2,3
string ,,true,false,123,foo,foo,bar,quux,FOO,BAR,[object Object]
string 1,2,,,,,,,,,,,,,,,,,,
string 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,3
string ,,,
string foo,,bar,
string foo,,bar,
string ,foo,,,,,,,,,,,bar,,,,,,,
===*/

print('basic');

function basicTest() {
    var t;

    // very basic tests

    test([]);
    test([1,2,3]);
    test([ undefined, null, true, false, 123, 'foo', [ 'foo', 'bar', 'quux', [ 'FOO', 'BAR' ] ], { foo: 1, bar: 2 } ]);

    t = [1,2];
    t.length = 20;
    test(t);

    // sparse array

    t = [1];
    t[100] = 3;
    t[50] = 2;
    test(t);

    // must also work on non-arrays

    test({ length: 4 });
    test({ '0': 'foo', '2': 'bar', length: 4 });
    test({ '0': 'foo', '2': 'bar', '4': 'quux', length: 4 });
    test({ '1': 'foo', '12': 'bar', length: 20 });
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
coercion
string foo,bar,quux
string foo,bar,quux
string foo,bar,quux,baz
length getter
0 getter
0 toLocaleString
0 retvalue toString
1 getter
1 toLocaleString
2 getter
2 toLocaleString
2 retvalue toString
string elem 0,123,elem 2
0 toLocaleString
0 retvalue toString
0 retvalue valueOf
1 toLocaleString
1 retvalue toString
1 retvalue valueOf
2 toLocaleString
2 retvalue toString
2 retvalue valueOf
string 123,undefined,true
0 toLocaleString
0 retvalue toString
0 retvalue valueOf
TypeError
===*/

print('coercion');

function coercionTest() {

    // ToUint32() coercion of length

    test({
        length: 256*256*256*256 + 3,  // ToUint32() of this -> 3
        '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz'
    });
    test({
        length: 3.9,
        '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz'
    });
    test({
        length: -256*256*256*256 + 3.9,   // round towards zero -> this becomes a 4
        '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz'
    });

    // coercion side effects test
    // (V8 triggers '0 getter' twice for some reason)

    test({
        get length() { print('length getter'); return 3; },
        set length() { print('length setter'); },

        get 0() {
            print('0 getter'); return {
                toLocaleString: function() {
                    print('0 toLocaleString');
                    // the ToString() coercion of the return value is not explicit in the spec
                    return {
                        toString: function() { print('0 retvalue toString'); return 'elem 0'; }
                    };
                }
            }
        },
        set 0() { print('0 setter'); },

        get 1() {
            print('1 getter'); return {
                toLocaleString: function() {
                    print('1 toLocaleString');
                    return '123';
                }
            }
        },
        set 1() { print('1 setter'); },

        get 2() {
            print('2 getter'); return {
                toLocaleString: function() {
                    print('2 toLocaleString');
                    return {
                        toString: function() { print('2 retvalue toString'); return 'elem 2'; }
                    };
                }
            }
        },
        set 2() { print('2 setter'); },
    });

    // Not really Array specific, but if toString() returns a non-primitive value,
    // valueOf() gets called.  If valueOf() also returns a non-primitive value,
    // a TypeError should occur.  If either returns a primitive value, ToString()
    // is applied to it to end up with a string value.

    test({
        length: 3,

        '0': {
            toLocaleString: function() {
                print('0 toLocaleString');
                return {
                    toString: function() { print('0 retvalue toString'); return []; },  // non-primitive -> fall back to valueOf()
                    valueOf: function() { print('0 retvalue valueOf'); return 123; },   // primitive -> 123 coerced to '123'
                };
            }
        },

        '1': {
            toLocaleString: function() {
                print('1 toLocaleString');
                return {
                    toString: function() { print('1 retvalue toString'); return []; },  // non-primitive -> fall back to valueOf()
                    valueOf: function() { print('1 retvalue valueOf'); return undefined; },   // primitive -> 'undefined'
                };
            }
        },

        '2': {
            toLocaleString: function() {
                print('2 toLocaleString');
                return {
                    toString: function() { print('2 retvalue toString'); return []; },  // non-primitive -> fall back to valueOf()
                    valueOf: function() { print('2 retvalue valueOf'); return true; },   // primitive -> 'true'
                };
            }
        }
    });

    test({
        length: 3,

        '0': {
            toLocaleString: function() {
                print('0 toLocaleString');
                return {
                    toString: function() { print('0 retvalue toString'); return []; },  // non-primitive -> fall back to valueOf()
                    valueOf: function() { print('0 retvalue valueOf'); return {}; }     // non-primitive -> TypeError
                };
            }
        }
    });
}

try {
    coercionTest();
} catch(e) {
    print(e);
}
