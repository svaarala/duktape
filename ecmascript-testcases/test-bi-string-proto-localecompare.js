/*
 *  This is necessarily a custom test, because behavior is implementation
 *  dependent.  Technically the coercion behavior could be verified in an
 *  implemented independent manner, but that's not very useful.
 *
 *  The current implementation has no special localeCompare(), but will
 *  just use UTF-8 comparison.
 *
 *  Other implementations seem to vary.  Rhino returns a value from {-1,0,1}
 *  while V8 apparently returns a different of codepoints (e.g. -11094).
 */

/*---
{
    "custom": true
}
---*/

/*===
basic
number 1
number 0
number -1
number 1
number 1
number 0
number -1
number -1
number 1
number 1
number 0
number -1
number -1
number 1
number 0
number -1
number 1
number 0
number -1
number 0
number -1
number 1
===*/

print('basic');

function basicTest() {
    function test(x,y) {
        var s1 = new String(x);
        var s2 = new String(y);
        var t = s1.localeCompare(s2);
        print(typeof t, t);
    }

    // ascii baseline
    test('foo', 'bar');
    test('foo', 'foo');
    test('foo', 'quux');

    // unicode
    test('foo\u1234bar\u0300', 'foo\u1234bar\u003f');
    test('foo\u1234bar\u0300', 'foo\u1234bar\u02ff');
    test('foo\u1234bar\u0300', 'foo\u1234bar\u0300');
    test('foo\u1234bar\u0300', 'foo\u1234bar\u0301');
    test('foo\u1234bar\u0300', 'foo\u1234bar\u3456');

    test('foo\u1234bar\u0900', 'foo\u1234bar\u003f');
    test('foo\u1234bar\u0900', 'foo\u1234bar\u08ff');
    test('foo\u1234bar\u0900', 'foo\u1234bar\u0900');
    test('foo\u1234bar\u0900', 'foo\u1234bar\u0901');
    test('foo\u1234bar\u0900', 'foo\u1234bar\u3456');

    // length cases
    test('xx', 'x');
    test('xx', 'xx');
    test('xx', 'xxx');

    // embedded NUL characters
    test('foo\u0000f', 'foo\u0000e');
    test('foo\u0000f', 'foo\u0000f');
    test('foo\u0000f', 'foo\u0000g');

    // empty strings
    test('', '');
    test('', 'foo');
    test('foo', '');
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
coercion
this
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
number 0
number 1
number -1
number 0
number 1
number -1
number 0
number 1
number -1
number 0
number 1
number -1
number 0
number 1
number -1
number 0
number 1
number -1
argument
number 0
number -1
number 1
number 0
number -1
number 1
number 0
number -1
number 1
number 0
number -1
number 1
number 0
number -1
number 1
number 0
number -1
number 1
number 0
number -1
number 1
number 0
number -1
number 1
===*/

/* Argument and this coercion */

print('coercion');

function coercionTest() {
    function test(x,y) {
        var t;

        try {
            t = String.prototype.localeCompare.call(x, y);
            print(typeof t, t);
        } catch (e) {
            print(e.name);
        }
    }

    print('this');

    test(undefined, 'undefined');
    test(undefined, 'undefinec');
    test(undefined, 'undefinee');

    test(null, 'null');
    test(null, 'nulk');
    test(null, 'nulm');

    test(true, 'true');
    test(true, 'trud');
    test(true, 'truf');

    test(false, 'false');
    test(false, 'falsd');
    test(false, 'falsf');

    test(123, '123');
    test(123, '122');
    test(123, '124');

    test('foo', 'foo');
    test('foo', 'fon');
    test('foo', 'fop');

    test([1,2], '1,2');
    test([1,2], '1,1');
    test([1,2], '1,3');

    /*
        >>> ord(']')
        93
        >>> chr(92)
        '\\'
        >>> chr(94)
        '^'
    */
    test({ foo: 1, bar: 2 }, '[object Object]');
    test({ foo: 1, bar: 2 }, '[object Object\u005c');
    test({ foo: 1, bar: 2 }, '[object Object\u005e');

    print('argument');

    test('undefined', undefined);
    test('undefinec', undefined);
    test('undefinee', undefined);

    test('null', null);
    test('nulk', null);
    test('nulm', null);

    test('true', true);
    test('trud', true);
    test('truf', true);

    test('false', false);
    test('falsd', false);
    test('falsf', false);

    test('123', 123);
    test('122', 123);
    test('124', 123);

    test('foo', 'foo');
    test('fon', 'foo');
    test('fop', 'foo');

    test('1,2', [1,2]);
    test('1,1', [1,2]);
    test('1,3', [1,2]);

    test('[object Object]', { foo: 1, bar: 2 });
    test('[object Object\u005c', { foo: 1, bar: 2 });
    test('[object Object\u005e', { foo: 1, bar: 2 });
}

try {
    coercionTest();
} catch (e) {
    print(e);
}

/*===
no that
number 0
number -1
number 1
===*/

/* If no 'that' is given, should behave like undefined, i.e. coerce
 * to the string 'undefined'.
 *
 * This seems to vary with the implementation.  Rhino will produce
 * an 'undefined' and compare as expected.  V8 will return 0 for all
 * comparisons (i.e. all strings "match" an undefined match string).
 */

print('no that');

function noThatTest() {
    function test(x) {
        var t = String.prototype.localeCompare.call(x);
        print(typeof t, t);
    }

    test('undefined');
    test('undefinec');
    test('undefinee');
}

try {
    noThatTest();
} catch (e) {
    print(e);
}
