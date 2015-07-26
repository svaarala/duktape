function test(this_value, separator, arg_count) {
    var t;

    try {
        if (arg_count === 0) {
            t = Array.prototype.join.call(this_value);
        } else {
            t = Array.prototype.join.call(this_value, separator);
        }
        print(typeof t, t);
    } catch (e) {
        print(e.name);
    }
}

/*===
basic
string 
string 
string 
string 1
string 
string foo
string ,,true,false,123,foo,1,2,[object Object]
string foo:bar:quux
string 1,2,3
string 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,3
string 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,3
===*/

print('basic');

function basicTest() {
    var t;

    test([]);
    test({});  // object without 'length' -> ToUint32(undefined) = 0
    test({ '0': 'foo' });

    test([1]);
    test({ length: 1 });
    test({ '0': 'foo', length: 1 });

    test([ undefined, null, true, false, 123, 'foo', [1, 2], { foo: 1, bar: 2 } ]);

    test([ 'foo', 'bar', 'quux' ], ':');

    // dense array join

    t = [1,2,3];
    test(t);

    // sparse array join

    t = [1];
    t[100] = 3;
    t[50] = 2;
    test(t);

    // non-array join

    t = {
        '0': 1,
        '100': 3,
        '50': 2,
        length: 101
    };
    test(t);
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
coercion
TypeError
TypeError
TypeError
string 
string 
string 
string f,o,o
string 1,2
string 
string foo,bar,,quux,,,,,,
string foo,bar,quux
string foonullbarnullquux
string footruebartruequux
string foofalsebarfalsequux
string foo123bar123quux
string fooFOObarFOOquux
string foo1,2bar1,2quux
string foo[object Object]bar[object Object]quux
separator toString
length getter
length retvalue valueOf
0 getter
0 retvalue toString
1 getter
1 retvalue toString
2 getter
2 retvalue toString
string foo_bar_quux
string foo_bar_quux_baz
string foo_bar_quux
length valueOf
string foo_bar_quux
===*/

print('coercion');

function coercionTest() {
    var t;

    // this coercion

    test(undefined, undefined, 1);
    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('foo');
    test([1,2]);
    test({ foo: 1, bar: 2 });
    test({ '0': 'foo', '1': 'bar', '3': 'quux', length: 10 });

    // separator is tostring coerced

    test([ 'foo', 'bar', 'quux' ], undefined);
    test([ 'foo', 'bar', 'quux' ], null);
    test([ 'foo', 'bar', 'quux' ], true);
    test([ 'foo', 'bar', 'quux' ], false);
    test([ 'foo', 'bar', 'quux' ], 123);
    test([ 'foo', 'bar', 'quux' ], 'FOO');
    test([ 'foo', 'bar', 'quux' ], [1,2]);
    test([ 'foo', 'bar', 'quux' ], { foo: 1, bar: 2 });

    // values are ToString coerced (already tested above in basicTest)

    // coercion: ToObject(this) (no side effects here), [[Get]] "length",
    // ToUint32(length), ToString(separator), [[Get]] "0", ToString(0),
    // [[Get]] "1", ToString(1) ...

    Array.prototype.join.call({
        get length() {
            print('length getter');
            return {
                toString: function() { print('length retvalue toString'); return '4'; },
                valueOf: function() { print('length retvalue valueOf'); return 3; }
            };
        },
        set length() { print('length setter'); },

        get 0() {
            print('0 getter');
            return {
                toString: function() { print('0 retvalue toString'); return 'str0'; },
                valueOf: function() { print('0 retvalue toString'); return 'val0'; },
            };
        },
        set 0() { print('0 setter'); },

        get 1() {
            print('1 getter');
            return {
                toString: function() { print('1 retvalue toString'); return 'str1'; },
                valueOf: function() { print('1 retvalue toString'); return 'val1'; },
            };
        },
        set 1() { print('1 setter'); },

        get 2() {
            print('2 getter');
            return {
                toString: function() { print('2 retvalue toString'); return 'str2'; },
                valueOf: function() { print('2 retvalue toString'); return 'val2'; },
            };
        },
        set 2() { print('2 setter'); },
    }, {
        toString: function() { print('separator toString'); return ':' },
        valueOf: function() { print('separator valueOf'); return ';' }
    });

    // length coercion: fractions, string, etc.  Coercion is through ToUint32();
    // this is easiest to test with a non-array, because array 'length' has
    // special behavior (RangeError for invalid array length)

    t = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 3.9 };
    test(t, '_');

    t = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: -256*256*256*256 + 3.9 };  // coerces to 4, rounding is towards zero before modulo
    test(t, '_');

    t = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: '3.9' };
    test(t, '_');

    t = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: {
        toString: function() { print('length toString'); return 4.9; },
        valueOf: function() { print('length valueOf'); return 3.9; }
    } };
    test(t, '_');
}

try {
    coercionTest();
} catch (e) {
    print(e);
}

/*===
join size test
empty sep ok
comma sep ok
===*/

/* The join() implementation performs intermediate joins to work around
 * value stack limits.  Test that joins work correctly at least to 10000
 * elements.
 */

print('join size test');

function joinSizeTest(joinChar) {
    var i, j, sz;
    var arr;
    var expect, joined;
    var ch;

    // The mid-join limit is 4096 now, test around the multiples.
    var sizes = [ 0, 1, 2, 4094, 4095, 4096, 4097, 4098, 8089, 8090, 8091, 8092, 8093, 8094 ];

    for (i = 0; i <= sizes.length; i++) {
        sz = sizes[i];
        arr = [];
        expect = '';
        for (j = 0; j < sz; j++) {
            ch = String.fromCharCode(0x41 + (j % 26));
            arr[j] = ch;
            if (j > 0) { expect += joinChar; }
            expect += ch;  // slow, but we want to avoid array join to create expect string
        }

        joined = arr.join(joinChar);
        if (expect !== joined) {
            print('FAILED');
            print(sz);
            print(expect);
            print(joined);
            throw new Error('join size test failed for size ' + sz);
        }
    }

}

try {
    joinSizeTest('');
    print('empty sep ok');
    joinSizeTest(',');
    print('comma sep ok');
} catch (e) {
    print(e.stack || e);
}
