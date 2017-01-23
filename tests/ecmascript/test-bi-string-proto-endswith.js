/*
 *  String.prototype.endsWith()
 */

/*===
argument regexp
TypeError
argument ToString() coercion
true
true
true
true
true
true
toString called
true
true
pos ToInteger() coercion
true
true
true
true
false
true
true
empty string
true
true
true
true
true
true
this binding coercion
0 TypeError
1 TypeError
2 false
2 true
3 false
3 true
4 false
4 true
5 false
5 true
6 false
6 true
7 false
7 true
true
properties and attributes
function true true false
string endsWith false true false
number 1 false true false
false
false
all done
===*/

function test() {
    var pd;

    // RegExp is explicitly rejected, as reserved for future use.
    print('argument regexp');
    try {
        print('foobar'.endsWith(/foo/));
    } catch (e) {
        print(e.name);
    }

    // Anything else is coerced using ToString().
    print('argument ToString() coercion');
    print('123undefined'.endsWith());  // undefined
    print('123undefined'.endsWith(void 0));
    print('123null'.endsWith(null));
    print('123true'.endsWith(true));
    print('1234567'.endsWith(567));
    print('1230.1'.endsWith(0.1));
    print('foobar'.endsWith({
        valueOf: function () { print('valueOf called'); return 'xyz'; },
        toString: function () { print('toString called'); return 'bar'; }
    }));
    print('1,2,3,4'.endsWith([2,3,4]));

    // Position is ToInteger() coerced and clamped to [0,this.length].
    // Undefined defaults to this.length; null coerces to zero.
    // The position is a character index.
    print('pos ToInteger() coercion');
    print('\uabcd\u1234\ucafefoo\udcba'.endsWith('foo\udcba', 1/0));  // same as this.length
    print('\uabcd\u1234\ucafefoo\udcba'.endsWith('foo\udcba', 10));  // same as this.length
    print('\uabcd\u1234\ucafefoo\udcba'.endsWith('foo\udcba', 7));
    print('\uabcd\u1234\ucafefoo\udcba'.endsWith('\ucafefoo', 6));
    print('\uabcd\u1234\ucafefoo\udcba'.endsWith('\uabcd', -1000));  // same as 0 (false)
    print('\uabcd\u1234\ucafefoo\udcba'.endsWith('', -1000));  // same as 0; empty string DOES match
    print('\uabcd\u1234\ucafefoo\udcba'.endsWith('\ucafefoo', 6.99));

    // Empty string is found in any location, including Infinity, NaN, etc.
    print('empty string');
    print('\ucafefoo'.endsWith('', -1/0));
    print('\ucafefoo'.endsWith('', 1/0));
    print('\ucafefoo'.endsWith('', 0/0));
    print('\ucafefoo'.endsWith('', -100));
    print('\ucafefoo'.endsWith('', 100));
    print(''.endsWith('', 10));

    // The 'this' binding is object coercibility checked (undefined, null rejected),
    // otherwise ToString coerced.
    print('this binding coercion');
    [ void 0, null, true, false, 123, 'foo', { foo: 123 }, [ 1, 2, 3 ] ].forEach(function (v, i) {
        try {
            print(i, String.prototype.endsWith.call(v, 'zzzz'));  // not found
            print(i, String.prototype.endsWith.call(v, String(v)));
        } catch (e) {
            print(i, e.name);
        }
    });
    print(String.prototype.endsWith.call(Math.PI, '3589793'));

    // Function and its properties/attributes.
    print('properties and attributes');
    pd = Object.getOwnPropertyDescriptor(String.prototype, 'endsWith');
    print(typeof pd.value, pd.writable, pd.configurable, pd.enumerable);
    pd = Object.getOwnPropertyDescriptor(String.prototype.endsWith, 'name');
    print(typeof pd.value, pd.value, pd.writable, pd.configurable, pd.enumerable);
    pd = Object.getOwnPropertyDescriptor(String.prototype.endsWith, 'length');
    print(typeof pd.value, pd.value, pd.writable, pd.configurable, pd.enumerable);

    // Some 32-bit and 64-bit wraparound tests.
    print('foobar'.endsWith('foo', 0x100000000 + 3));  // test that 2^32 + 3 won't wrap to 3
    print('foobar'.endsWith('foo', 0x10000000000000000 + 3));  // test that 2^64 + 3 won't wrap to 3

    // Some memory safety tests (no output).
    var values = [
        '', 'foo', '\ucafefoo\uabcdbar', 'bar', '\ucafe'
    ];
    values.forEach(function (v1, i1) {
        values.forEach(function (v2, i2) {
            for (var i = -100; i <= 100; i++) {
                String.prototype.endsWith.call(v1, v2, i);
            }
        });
    });

    print('all done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
