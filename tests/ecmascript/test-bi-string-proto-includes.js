/*
 *  String.prototype.includes()
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
string includes false true false
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
        print('foobar'.includes(/foo/));
    } catch (e) {
        print(e.name);
    }

    // Anything else is coerced using ToString().
    print('argument ToString() coercion');
    print('undefined123'.includes());  // undefined
    print('undefined123'.includes(void 0));
    print('null123'.includes(null));
    print('true123'.includes(true));
    print('1234567'.includes(123));
    print('0.123'.includes(0.1));
    print('foobar'.includes({
        valueOf: function () { print('valueOf called'); return 'xyz'; },
        toString: function () { print('toString called'); return 'foo'; }
    }));
    print('1,2,3,4'.includes([1,2,3]));

    // Position is ToInteger() coerced and clamped to [0,this.length].
    // Undefined coerces to 0.  The position is a character index.
    print('pos ToInteger() coercion');
    print('\uabcd\u1234\ucafefoo\udcba'.includes('\uabcd\u1234', -1/0));  // same as 0
    print('\uabcd\u1234\ucafefoo\udcba'.includes('\uabcd\u1234', -10));  // same as 0
    print('\uabcd\u1234\ucafefoo\udcba'.includes('\uabcd\u1234', 0));
    print('\uabcd\u1234\ucafefoo\udcba'.includes('\u1234\ucafe', 1));
    print('\uabcd\u1234\ucafefoo\udcba'.includes('\u1234\ucafe', 1000));  // same as this.length (false)
    print('\uabcd\u1234\ucafefoo\udcba'.includes('', 1000));  // same as this.length; empty string DOES match
    print('\uabcd\u1234\ucafefoo\udcba'.includes('\u1234\ucafe', 1.99));

    // Empty string is found in any location, including Infinity, NaN, etc.
    print('empty string');
    print('\ucafefoo'.includes('', -1/0));
    print('\ucafefoo'.includes('', 1/0));
    print('\ucafefoo'.includes('', 0/0));
    print('\ucafefoo'.includes('', -100));
    print('\ucafefoo'.includes('', 100));
    print(''.includes('', 10));

    // The 'this' binding is object coercibility checked (undefined, null rejected),
    // otherwise ToString coerced.
    print('this binding coercion');
    [ void 0, null, true, false, 123, 'foo', { foo: 123 }, [ 1, 2, 3 ] ].forEach(function (v, i) {
        try {
            print(i, String.prototype.includes.call(v, 'zzzz'));  // not found
            print(i, String.prototype.includes.call(v, String(v)));
        } catch (e) {
            print(i, e.name);
        }
    });
    print(String.prototype.includes.call(Math.PI, '3.14'));

    // Function and its properties/attributes.
    print('properties and attributes');
    pd = Object.getOwnPropertyDescriptor(String.prototype, 'includes');
    print(typeof pd.value, pd.writable, pd.configurable, pd.enumerable);
    pd = Object.getOwnPropertyDescriptor(String.prototype.includes, 'name');
    print(typeof pd.value, pd.value, pd.writable, pd.configurable, pd.enumerable);
    pd = Object.getOwnPropertyDescriptor(String.prototype.includes, 'length');
    print(typeof pd.value, pd.value, pd.writable, pd.configurable, pd.enumerable);

    // Some 32-bit and 64-bit wraparound tests.
    print('foobar'.includes('foo', 0x100000000));  // test that 2^32 won't wrap to 0
    print('foobar'.includes('foo', 0x10000000000000000));  // test that 2^64 won't wrap to 0

    // Some memory safety tests (no output).
    var values = [
        '', 'foo', '\ucafefoo\uabcdbar', 'bar', '\ucafe'
    ];
    values.forEach(function (v1, i1) {
        values.forEach(function (v2, i2) {
            for (var i = -100; i <= 100; i++) {
                String.prototype.includes.call(v1, v2, i);
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
