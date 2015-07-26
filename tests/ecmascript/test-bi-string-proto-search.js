function test(this_val, regexp_val, arg_count) {
    var t;

    try {
        if (arg_count === 0) {
            t = String.prototype.search.call();
        } else if (arg_count === 1) {
            t = String.prototype.search.call(this_val);
        } else {
            t = String.prototype.search.call(this_val, regexp_val);
        }
        print(typeof t, t);
    } catch (e) {
        print(e.name);
    }
}

/*===
basic
number 3
number -1
number 5
number 0
number 3
string 12345
number -1
string 12345
number 3
string 12345
number -1
string 12345
number 4
string 12345
number -1
number -1
number 3
number 0
===*/

print('basic');

function basicTest() {
    // basic searches
    test('foobar', 'bar');
    test('foobar', 'quux');

    // non-BMP, verify that char index is returned
    test('\u1234\u0123foobar\udead\ubeef', 'bar');

    // a string is always interpreted as a regexp, not literally
    test('foo', '.');

    // RegExp global flag and lastIndex are ignored and unaffected
    re = /bar/;
    re.lastIndex = '12345';
    test('foobar', re);
    print(typeof re.lastIndex, re.lastIndex);

    re = /bar/;
    re.lastIndex = '12345';
    test('foobaz', re);
    print(typeof re.lastIndex, re.lastIndex);

    // RegExp multiline and ignoreCare work as expected
    re = /bar/i;
    re.lastIndex = '12345';
    test('fooBaR', re);
    print(typeof re.lastIndex, re.lastIndex);

    re = /^bar/;  // single line, does not match
    re.lastIndex = '12345';
    test('foo\nbar', re);
    print(typeof re.lastIndex, re.lastIndex);

    re = /^bar/m;
    re.lastIndex = '12345';
    test('foo\nbar', re);
    print(typeof re.lastIndex, re.lastIndex);

    // if regexp argument is not a RegExp, a fresh one is created; it will have
    // no flags, so regexp flags will be: global=false, multiline=false, ignoreCase=false

    test('Foo', 'foo');        // case sensitive -> no match
    test('foo\nbar', '^bar');  // single line -> no match

    // multiple matches -> report first
    test('foobarfoobarfoo', 'bar');

    // empty string and empty search string
    test('', '');
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
number 1
number 2
number 1
number 1
number 1
number 8
number 0
number 0
number 3
number 3
number 3
number 3
number 3
number 3
number 1
number -1
===*/

print('coercion');

function coercionTest() {
    // this coercion
    test(undefined, undefined, 0);
    test(undefined, 'foo');
    test(null, 'foo');
    test(true, 'rue');
    test(false, 'lse');
    test(123, '2');
    test('foo', 'oo');
    test([1,2], ',2');
    test({ foo: 1, bar: 2 }, 'Object');

    // regexp coercion -- RegExps are accepted as is, everything else is
    // handed over to the RegExp constructor (without coercion).  The RegExp
    // constructor coerces with ToString(), except that undefined values are
    // coerced to the empty string (E5.1 Section 15.10.4.1).
    //
    // The undefined case should be same as an empty search string, i.e.
    // match at index 0.  Rhino will work correctly for the first test
    // but if an explicit undefined argument is given, it will coerce it
    // to 'undefined' search string.

    test('fooundefinedbar', undefined, 1);  // coerces to empty, match at 0
    test('fooundefinedbar', undefined);     // same
    test('foonullbar', null);
    test('footruebar', true);
    test('foofalsebar', false);
    test('foo123bar', 123);
    test('fooquuxbar', 'quux');
    test('foo1,2bar', [1,2]);

    // interesting case: ToString(obj) will be '[object Object]' which is
    // a RegExp class expression!
    test('abcdefgh', { foo: 1, bar: 2 });  // match at offset 1 ('b')
    test('xxxP', { foo: 1, bar: 2 });      // no match
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
