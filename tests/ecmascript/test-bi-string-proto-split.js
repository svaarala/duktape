/*===
basic
object 3 ["foo"," bar"," quux"]
object 1 ["foo - undefined - bar"]
object 1 ["foo - undefined - bar"]
object 6 ["f","o","o","b","a","r"]
object 4 97 837 4660 65244
object 6 ["f","o","o","b","a","r"]
object 6 ["f","o","o","b","a","r"]
object 7 ["","","","","","",""]
object 3 ["foo","bar","quux"]
object 0 []
object 1 [""]
object 0 []
object 1 [""]
object 10 ["foo",";"," ","bar",";","","quux",";"," ","baz"]
object 7 ["foo",";",null," bar",null,","," baz"]
object 2 ["a","b"]
object 2 ["","b"]
object 13 ["A",null,"B","bold","/","B","and",null,"CODE","coded","/","CODE",""]
object 2 ["foo",""]
object 1 ["foo"]
object 2 ["foo",""]
object 3 1 291 1 4660 1 65244
object 3 1 291 1 4660 1 65244
===*/

print('basic');

function basicTest() {
    var t;

    function p(x) {
        print(typeof x, x.length, JSON.stringify(x));
    }

    // Basic test

    p('foo, bar, quux'.split(','));

    // Undefined separator results in an array containing just 'this' as a
    // string; it must not be coerced to 'undefined' (and then used as a
    // separator string).

    p(new String('foo - undefined - bar').split());
    p(new String('foo - undefined - bar').split(undefined));

    // Empty separator splits a string into individual characters

    p('foobar'.split(''));

    // Unicode characters cause different processing internally

    t = 'a\u0345\u1234\ufedc'.split('');
    print(typeof t, t.length, t[0].charCodeAt(0), t[1].charCodeAt(0), t[2].charCodeAt(0), t[3].charCodeAt(0));

    // Empty RegExp, also splits into individual characters

    p('foobar'.split(/(?:)/));

    // Only first RegExp match matters at a given position; this is
    // equivalent to an empty regexp

    p('foobar'.split(/(?:)|./));  // first alt matches
    p('foobar'.split(/.|(?:)/));

    // Simple two character separator

    p('foo::bar::quux'.split('::'));

    // If 'this' coerces to empty string, result depends on whether separator
    // can match the empty string

    p(''.split(''));     // can match -> no elems
    p(''.split('a'));    // cannot match -> ['']
    p(''.split(/a*/));   // can match -> no elems
    p(''.split(/a+/));   // cannot match -> ['']

    // RegExp captures are made part of the result array

    p('foo; bar;quux; baz'.split(/(;)(\s?)/));
    p('foo; bar, baz'.split(/(;)|(,)/));  // unmatched -> undefined -> JSON null

    // Specific example from spec: evaluates to ['a','b']

    p('ab'.split(/a*?/));

    // Specific example from spec: evaluates to ['', 'b']

    p('ab'.split(/a*/));

    // Specific example from spec: note that 'undefined' values join as empty

    p('A<B>bold</B>and<CODE>coded</CODE>'.split(/<(\/)?([^<>]+)>/));

    // Trailer is added as an empty string if last match coincides with
    // the end of the string.

    p('foobar'.split('bar'));

    // Trailer may hit the limit

    p('foobar'.split('bar', 1));  // trailer dropped
    p('foobar'.split('bar', 2));  // trailer fits

    // Some non-BMP tests; important because both byte and char offsets
    // are used internally.

    t = '\u0123\u1234\ufedc'.split('');
    print(typeof t, t.length,
          t[0].length, t[0].charCodeAt(0),
          t[1].length, t[1].charCodeAt(0),
          t[2].length, t[2].charCodeAt(0));

    t = '\u0123\u1234\ufedc'.split(/(?:)/);
    print(typeof t, t.length,
          t[0].length, t[0].charCodeAt(0),
          t[1].length, t[1].charCodeAt(0),
          t[2].length, t[2].charCodeAt(0));

    // XXX: add more non-BMP tests
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
limit
object 1 ["foo"]
object 0 []
object 3 ["foo","bar","quux"]
object 3 ["foo","bar","quux"]
object 2 ["foo","bar"]
object 1 ["foo"]
object 0 []
object 0 []
object 1 ["foo"]
object 2 ["foo","bar"]
object 4 ["foo","bar","quux","baz"]
object 1 ["foo"]
object 2 ["foo","bar"]
object 2 ["foo","bar"]
object 3 ["foo","bar","quux"]
object 9 ["foo:","1","2","3",":bar:","3","2","1",":quux"]
object 9 ["foo:","1","2","3",":bar:","3","2","1",":quux"]
object 8 ["foo:","1","2","3",":bar:","3","2","1"]
object 7 ["foo:","1","2","3",":bar:","3","2"]
object 6 ["foo:","1","2","3",":bar:","3"]
object 5 ["foo:","1","2","3",":bar:"]
object 4 ["foo:","1","2","3"]
object 3 ["foo:","1","2"]
object 2 ["foo:","1"]
object 1 ["foo:"]
object 0 []
===*/

/* Limit tests.  Limit may affect the result at any point; in particular,
 * RegExp captures may need be limited from the middle.
 *
 * The default limit of 2^32 - 1 is achievable with captures, but requires
 * a huge amount of memory.  It is not tested now.
 */

print('limit');

function limitTest() {
    var i;

    function p(x) {
        print(typeof x, x.length, JSON.stringify(x));
    }

    // zero limit

    p('foo'.split(undefined));     // undefined limit -> 2**32-1
    p('foo'.split(undefined, 0));  // normally returns array with string as is, limit 0 -> return []

    // limit, normal case

    p('foo,bar,quux'.split(','));
    p('foo,bar,quux'.split(',', 3));
    p('foo,bar,quux'.split(',', 2));
    p('foo,bar,quux'.split(',', 1));
    p('foo,bar,quux'.split(',', 0));

    // huge positive limit wraps through ToUint32()
    p('foo,bar,quux,baz'.split(',', 4294967296));  // = 0
    p('foo,bar,quux,baz'.split(',', 4294967297));  // = 1
    p('foo,bar,quux,baz'.split(',', 4294967298));  // = 2

    // negative limit goes through ToUint32() and becomes a positive one
    // -1 -> 4294967295
    // -4294967295 -> 1
    // -4294967294 -> 2

    p('foo,bar,quux,baz'.split(',', -1));
    p('foo,bar,quux,baz'.split(',', -4294967295));
    p('foo,bar,quux,baz'.split(',', -4294967294));

    // ToUint32() coerces fractions towards zero
    p('foo,bar,quux,baz'.split(',', 2.4));                      // limit 2
    p('foo,bar,quux,baz'.split(',', 2.4 - (256*256*256*256)));  // limit 3 (!)

    // limit, happens in the middle of a capture
    p('foo:123:bar:321:quux'.split(/(\d)(\d)(\d)/));
    for (i = 9; i >= 0; i--) {
        p('foo:123:bar:321:quux'.split(/(\d)(\d)(\d)/, i));
    }
}

try {
    limitTest();
} catch (e) {
    print(e);
}

/*===
regexp
before object string 12345
object 3 ["foo","bar","quux"]
after object string 12345
before object string 12345
object 5 ["foo","z","bar","Z","quux"]
after object string 12345
before object string 12345
object 1 ["foo\nbar\nquux"]
after object string 12345
before object string 12345
object 3 ["foo\n","bar\n","quux"]
after object string 12345
before object string 12345
object 3 ["foo","bar","quux"]
after object string 12345
===*/

/* A RegExp separator is used through the internal [[Match]] method.  This
 * executes the regexp pattern, taking ignoreCase and multiline flags into
 * account but ignoring the global flag.  The RegExp instance properties
 * (such as lastIndex) are neither read or written.
 */

print('regexp');

function regExpTest() {
    var re;

    function test(this_val, sep_val, limit_val) {
        var t;

        print('before', typeof sep_val, typeof sep_val.lastIndex, sep_val.lastIndex);
        try {
            t = String.prototype.split.call(this_val, sep_val, limit_val);
            print(typeof t, t.length, JSON.stringify(t));
        } catch (e) {
            print(e.name);
        }
        print('after', typeof sep_val, typeof sep_val.lastIndex, sep_val.lastIndex);
    }

    // ignoreCase matcher
    re = /z/i;
    re.lastIndex = '12345';
    test('foozbarZquux', re);

    // ignoreCase + captures; captures have original casing
    re = /(z)/i;
    re.lastIndex = '12345';
    test('foozbarZquux', re);

    // multiline matcher; use multiline '^' to detect
    re = /^/;  // single line match, does not match start of line
    re.lastIndex = '12345';
    test('foo\nbar\nquux', re);

    re = /^/m;  // matches start of line, empty match, newlines will be part of match
    re.lastIndex = '12345';
    test('foo\nbar\nquux', re);

    // global matcher, no effect
    re = /z/gi;
    re.lastIndex = '12345';
    test('foozbarZquux', re);
}

try {
    regExpTest();
} catch (e) {
    print(e);
}

/*===
coercion
TypeError
TypeError
TypeError
object 1 ["true"]
object 1 ["false"]
object 1 ["123"]
object 1 ["quux"]
object 3 ["1","2","3"]
object 2 ["[object","Object]"]
object 3 ["foo","bar","quux"]
object 1 ["fooundefinedbarundefinedquux"]
object 3 ["foo","bar","quux"]
object 3 ["foo","bar","quux"]
object 3 ["foo","bar","quux"]
object 3 ["foo","bar","quux"]
object 3 ["foo","bar","quux"]
object 3 ["foo","bar","quux"]
object 3 ["foo","bar","quux"]
toString() this
valueOf() limit
toString() sep
object 2 ["foo","bar"]
===*/

print('coercion');

function coercionTest() {
    function test(this_val, sep_val, limit_val, arg_count) {
        var t;

        try {
            if (arg_count === 0) {
                t = String.prototype.split.call();
            } else if (arg_count === 1) {
                t = String.prototype.split.call(this_val);
            } else if (arg_count === 2) {
                t = String.prototype.split.call(this_val, sep_val);
            } else {
                t = String.prototype.split.call(this_val, sep_val, limit_val);
            }

            print(typeof t, t.length, JSON.stringify(t));
        } catch (e) {
            print(e.name);
        }
    }

    // this coercion check
    test(undefined, undefined, undefined, 0);
    test(undefined, ',', 3);
    test(null, ',', 3);
    test(true, ',', 3);
    test(false, ',', 3);
    test(123, ',', 3);
    test('quux', ',', 3);
    test([1,2,3], ',', 3);
    test({ foo: 1, bar: 2 }, ' ', 3);

    // limit coercion (most limit checks already done above)
    test('foo,bar,quux,baz', ',', '3.9000e0');

    // separator coercion
    test('fooundefinedbarundefinedquux', undefined);   // undefined separator -> return input as only array elem (E5.1 Section 15.4.14, step 10)
    test('foonullbarnullquux', null);
    test('footruebartruequux', true);
    test('foofalsebarfalsequux', false);
    test('foo123bar123quux', 123);
    test('fooXbarXquux', 'X');
    test('foo1,2bar1,2quux', [1,2]);
    test('foo[object Object]bar[object Object]quux', { foo: 1, bar: 2 });

    // coercion order: 'this', limit, separator
    test({
        toString: function() { print('toString() this'); return 'foo,bar,quux,baz'; },
        valueOf: function() { print('valueOf() this'); return 'Foo,Bar,Quux,Baz'; }
    }, {
        toString: function() { print('toString() sep'); return ','; },
        valueOf: function() { print('valueOf() sep'); return ';'; }
    }, {
        toString: function() { print('toString() limit'); return 3; },
        valueOf: function() { print('valueOf() limit'); return 2; }
    });
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
