/*===
basic
number 6
number 9
number 6
number 9
number 0
number 3
number -1
number -1
number 0
number 0
number 0
number 0
number -1
number -1
number -1
number -1
===*/

print('basic');

function basicTest() {
    var str = new String('foobarfoobar');

    function p(x) {
        print(typeof x, x);
    }

    p(str.lastIndexOf('foo'));
    p(str.lastIndexOf('bar'));
    p(str.lastIndexOf('foo', 6));   // last foo
    p(str.lastIndexOf('bar', 9));  // last bar
    p(str.lastIndexOf('foo', 5));   // first foo
    p(str.lastIndexOf('bar', 8));   // first bar
    p(str.lastIndexOf('bar', 2));  // not found

    p(str.lastIndexOf('quux'));

    // empty string; empty search matches with any initial position,
    // non-empty search never matches

    str = new String('');
    p(str.indexOf(''));
    p(str.indexOf('', -1));
    p(str.indexOf('', 0));
    p(str.indexOf('', 1));
    p(str.indexOf('foo'));
    p(str.indexOf('foo', -1));
    p(str.indexOf('foo', 0));
    p(str.indexOf('foo', 1));
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
empty
number 6
number 0
number 0
number 0
number 0
number 1
number 2
number 3
number 4
number 5
number 6
number 6
number 6
number 6
number 0
number 6
number 6
===*/

/* An empty search string is always found immediately at the starting
 * position.  Thus, the return value is the input position, clamped to
 * to range [0,len(str)].
 *
 * There is some variance among implementations.  Rhino will always
 * return a value in the range [0,6] for the test below, but for
 * indexOf() it will return -1 (not found) if initial position is 7
 * or above.
 */

print('empty');

function emptyTest() {
    var str = new String('foobar');
    var i;

    function p(x) {
        print(typeof x, x);
    }

    p(str.lastIndexOf(''));
    for (i = -3; i < 10; i++) {
        p(str.lastIndexOf('', i));
    }

    p(str.lastIndexOf('', Number.NEGATIVE_INFINITY));
    p(str.lastIndexOf('', Number.POSITIVE_INFINITY));
    p(str.lastIndexOf('', Number.NaN));  // coerces to 0
}

try {
    emptyTest();
} catch (e) {
    print(e);
}

/*===
position
number 5
number 0
number 0
number 0
number 0
number 0
number 0
number 0
number 1
number 2
number 3
number 4
number 5
number 5
number 5
number 5
number 5
number 5
number 5
number 3
number 3
number 3
number 3
number 0
number 0
number 0
number 0
number -1
number -1
===*/

/* Position handling.  The position is coerced with ToInteger(), then
 * clamped to the range [0, len(str)].  ToInteger(NaN) is 0.
 */

print('position');

function positionTest() {
    // test string, 'x' always found if index < 6
    var str = 'xxxxxx';

    function p(x) {
        print(typeof x, x);
    }

    p(str.lastIndexOf('x'));
    p(str.lastIndexOf('x', Number.NEGATIVE_INFINITY));
    p(str.lastIndexOf('x', -123));
    p(str.lastIndexOf('x', -3));
    p(str.lastIndexOf('x', -2));
    p(str.lastIndexOf('x', -1));
    p(str.lastIndexOf('x', -0));
    p(str.lastIndexOf('x', 0));
    p(str.lastIndexOf('x', 1));
    p(str.lastIndexOf('x', 2));
    p(str.lastIndexOf('x', 3));
    p(str.lastIndexOf('x', 4));
    p(str.lastIndexOf('x', 5));
    p(str.lastIndexOf('x', 6));
    p(str.lastIndexOf('x', 7));
    p(str.lastIndexOf('x', 8));
    p(str.lastIndexOf('x', 123));
    p(str.lastIndexOf('x', Number.POSITIVE_INFINITY));
    p(str.lastIndexOf('x', Number.NaN));

    // fractions
    p(str.lastIndexOf('x', 3.0));
    p(str.lastIndexOf('x', 3.1));
    p(str.lastIndexOf('x', 3.5));
    p(str.lastIndexOf('x', 3.9));
    p(str.lastIndexOf('x', -3.0));
    p(str.lastIndexOf('x', -3.1));
    p(str.lastIndexOf('x', -3.5));
    p(str.lastIndexOf('x', -3.9));

    // if string is empty, even zero/NaN will yield -1 (not found)
    str = new String('');
    p(str.lastIndexOf('x', 0));
    p(str.lastIndexOf('x', Number.NaN));
}

try {
    positionTest();
} catch (e) {
    print(e);
}

/*===
argument coercion
number 7
number 7
number 17
number 22
number 27
number 33
number 37
number 41
number 45
toString()
number 37
toString()
valueOf()
number 33
===*/

/* Argument coercion.  Argument is coerced with ToString. */

print('argument coercion');

function argumentTest() {
    var str = new String('foobar undefined null true false 123 baz 1,2 [object Object]');

    function p(x) {
        print(typeof x, x);
    }

    p(str.lastIndexOf());  // argument not given
    p(str.lastIndexOf(undefined));
    p(str.lastIndexOf(null));
    p(str.lastIndexOf(true));
    p(str.lastIndexOf(false));
    p(str.lastIndexOf(123));
    p(str.lastIndexOf('baz'));
    p(str.lastIndexOf([1,2]));
    p(str.lastIndexOf({ foo: 1, bar: 2 }));

    // coercion side effect
    p(str.lastIndexOf({
        toString: function() { print('toString()'); return 'baz'; },
        valueOf: function() { print('valueOf()'); return '123'; }
    }));

    // ToString() coercion falls back from toString() to valueOf() if toString()
    // returns a non-primitive value
    p(str.lastIndexOf({
        toString: function() { print('toString()'); return []; },
        valueOf: function() { print('valueOf()'); return '123'; }
    }));
}

try {
    argumentTest();
} catch (e) {
    print(e);
}

/*===
this coercion
TypeError
TypeError
number 0
number 2
number 1
number 1
number 1
number 8
toString() this
toString() arg
number 5
===*/

/* This coercion, the method is generic. */

print('this coercion');

function thisCoercionTest() {
    function test(x,y) {
        var t;
        try {
            t = String.prototype.lastIndexOf.call(x, y);
            print(typeof t, t);
        } catch (e) {
            print(e.name);
        }
    }

    test(undefined, 'undefined');  // TypeError
    test(null, 'null');  // TypeError

    test(true, 'true');
    test(false, 'lse');
    test(123, '23');
    test('quux', 'uux');
    test([1,2], ',2');
    test({ foo: 1, bar: 2 }, 'Object');

    // coercion order if both this and searchString are coerced
    test({
        toString: function() { print('toString() this'); return 'test string'; },
        valueOf: function() { print('valueOf() this'); return 'another value'; }
    }, {
        toString: function() { print('toString() arg'); return 'stri'; },
        valueOf: function() { print('valueOf() arg'); return 'foo'; }
    });
}

try {
    thisCoercionTest();
} catch (e) {
    print(e);
}

/*===
non-bmp
number 0
number 3
number 7
number 7
number 7
number -1
===*/

/* Because search happens internally using a byte search, a few simple tests
 * with Unicode characters of varying length.
 */

print('non-bmp');

function nonBmpTest() {
    var str = new String('foo\u1234bar\udeadquux\ubeefbaz');

    function p(x) {
        print(typeof x, x);
    }

    p(str.lastIndexOf('foo'));
    p(str.lastIndexOf('\u1234'));
    p(str.lastIndexOf('\udead'));
    p(str.lastIndexOf('\udead', 7));  // found
    p(str.lastIndexOf('\udead', 8));  // found
    p(str.lastIndexOf('\udead', 6));  // not found
}

try {
    nonBmpTest();
} catch (e) {
    print(e);
}
