/*
 *  String.prototype.startsWith()
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
string startsWith false true false
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
        print('foobar'.startsWith(/foo/));
    } catch (e) {
        print(e.name);
    }

    // Anything else is coerced using ToString().
    print('argument ToString() coercion');
    print('undefined123'.startsWith());  // undefined
    print('undefined123'.startsWith(void 0));
    print('null123'.startsWith(null));
    print('true123'.startsWith(true));
    print('1234567'.startsWith(123));
    print('0.123'.startsWith(0.1));
    print('foobar'.startsWith({
        valueOf: function () { print('valueOf called'); return 'xyz'; },
        toString: function () { print('toString called'); return 'foo'; }
    }));
    print('1,2,3,4'.startsWith([1,2,3]));

    // Position is ToInteger() coerced and clamped to [0,this.length].
    // Undefined coerces to 0.  The position is a character index.
    print('pos ToInteger() coercion');
    print('\uabcd\u1234\ucafefoo\udcba'.startsWith('\uabcd\u1234', -1/0));  // same as 0
    print('\uabcd\u1234\ucafefoo\udcba'.startsWith('\uabcd\u1234', -10));  // same as 0
    print('\uabcd\u1234\ucafefoo\udcba'.startsWith('\uabcd\u1234', 0));
    print('\uabcd\u1234\ucafefoo\udcba'.startsWith('\u1234\ucafe', 1));
    print('\uabcd\u1234\ucafefoo\udcba'.startsWith('\u1234\ucafe', 1000));  // same as this.length (false)
    print('\uabcd\u1234\ucafefoo\udcba'.startsWith('', 1000));  // same as this.length; empty string DOES match
    print('\uabcd\u1234\ucafefoo\udcba'.startsWith('\u1234\ucafe', 1.99));

    // Empty string is found in any location, including Infinity, NaN, etc.
    print('empty string');
    print('\ucafefoo'.startsWith('', -1/0));
    print('\ucafefoo'.startsWith('', 1/0));
    print('\ucafefoo'.startsWith('', 0/0));
    print('\ucafefoo'.startsWith('', -100));
    print('\ucafefoo'.startsWith('', 100));
    print(''.startsWith('', 10));

    // The 'this' binding is object coercibility checked (undefined, null rejected),
    // otherwise ToString coerced.
    print('this binding coercion');
    [ void 0, null, true, false, 123, 'foo', { foo: 123 }, [ 1, 2, 3 ] ].forEach(function (v, i) {
        try {
            print(i, String.prototype.startsWith.call(v, 'zzzz'));  // not found
            print(i, String.prototype.startsWith.call(v, String(v)));
        } catch (e) {
            print(i, e.name);
        }
    });
    print(String.prototype.startsWith.call(Math.PI, '3.14'));

    // Function and its properties/attributes.
    print('properties and attributes');
    pd = Object.getOwnPropertyDescriptor(String.prototype, 'startsWith');
    print(typeof pd.value, pd.writable, pd.configurable, pd.enumerable);
    pd = Object.getOwnPropertyDescriptor(String.prototype.startsWith, 'name');
    print(typeof pd.value, pd.value, pd.writable, pd.configurable, pd.enumerable);
    pd = Object.getOwnPropertyDescriptor(String.prototype.startsWith, 'length');
    print(typeof pd.value, pd.value, pd.writable, pd.configurable, pd.enumerable);

    // Some 32-bit and 64-bit wraparound tests.
    print('foobar'.startsWith('foo', 0x100000000));  // test that 2^32 won't wrap to 0
    print('foobar'.startsWith('foo', 0x10000000000000000));  // test that 2^64 won't wrap to 0

    // Some memory safety tests (no output).
    var values = [
        '', 'foo', '\ucafefoo\uabcdbar', 'bar', '\ucafe'
    ];
    values.forEach(function (v1, i1) {
        values.forEach(function (v2, i2) {
            for (var i = -100; i <= 100; i++) {
                String.prototype.startsWith.call(v1, v2, i);
            }
        });
    });

    print('all done');
}

test();
