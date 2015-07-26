/*===
basic
object 1 foo 0 foobarfoo
object 1 foo 0 foobarfoo
object true
object 1 foo 0 foobarfoo
object true
object 2 foo foo
object true
object 1  0 foobarfoo
object 1  0 foobarfoo
object 10 0 0 0 0 0 0 0 0 0 0
object 1 49 64974 1
object 1 49 64974 1
object 7 0 0 0 0 0 0 0
===*/

print('basic');

function basicTest() {
    var s = new String('foobarfoo');
    var m;

    // 'foo' coerces to /foo/, non-global regexp; result is the same as if
    // /foo/.exec() had been called

    m = s.match('foo');
    print(typeof m, m.length, m[0], m.index, m.input);

    // a string argument creates a RegExp, and plain string match cannot be done

    m = s.match('f..');
    print(typeof m, m.length, m[0], m.index, m.input);

    // no match

    m = s.match('quux');
    print(typeof m, m === null);

    // explicit non-global regexps, for comparison

    m = s.match(/foo/);
    print(typeof m, m.length, m[0], m.index, m.input);

    m = s.match(/quux/);
    print(typeof m, m === null);

    // global regexp: return list of matches

    m = s.match(/foo/g);
    print(typeof m, m.length, m[0], m[1]);

    // global regexp, no match; null instead of empty array

    m = s.match(/quux/g);
    print(typeof m, m === null);

    // empty string or empty regexp match (internally the same, because a
    // string is used to create a regexp matching an empty string) -> match
    // once at the beginning

    m = s.match('');
    print(typeof m, m.length, m[0], m.index, m.input);
    m = s.match(/(?:)/);
    print(typeof m, m.length, m[0], m.index, m.input);

    // global regexp matching an empty string -> match once between each
    // character, once before the first char, and once after the last char

    m = s.match(/(?:)/g);
    print(typeof m, m.length, m[0].length, m[1].length, m[2].length, m[3].length,
          m[4].length, m[5].length, m[6].length, m[7].length, m[8].length, m[9].length);

    // Some non-BMP tests; important because implementation uses both char
    // and byte offsets.  These tests are similar to the ones above.

    s = new String('\u1234\u0031\ufdce\u1234\u0031\ufdce');

    m = s.match('\u0031\ufdce');
    print(typeof m, m.length, m[0].charCodeAt(0), m[0].charCodeAt(1), m.index);

    m = s.match('\u0031.');
    print(typeof m, m.length, m[0].charCodeAt(0), m[0].charCodeAt(1), m.index);

    m = s.match(/(?:)/g);
    print(typeof m, m.length, m[0].length, m[1].length, m[2].length, m[3].length,
          m[4].length, m[5].length, m[6].length);
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
regexp
before [object RegExp], source=zoo, global=false, ignoreCase=false, multiline=false, lastIndex=1 (string)
[object Null]
after [object RegExp], source=zoo, global=false, ignoreCase=false, multiline=false, lastIndex=0 (number)
valueOf() lastIndex
before [object RegExp], source=foo, global=false, ignoreCase=false, multiline=false, lastIndex=1 (object)
valueOf() lastIndex
[object Array] 1 foo input=foobarfoobar index=0
valueOf() lastIndex
after [object RegExp], source=foo, global=false, ignoreCase=false, multiline=false, lastIndex=1 (object)
before [object RegExp], source=zoo, global=true, ignoreCase=false, multiline=false, lastIndex=1 (string)
[object Null]
after [object RegExp], source=zoo, global=true, ignoreCase=false, multiline=false, lastIndex=0 (number)
before [object RegExp], source=foo, global=true, ignoreCase=false, multiline=false, lastIndex=0 (number)
[object Array] 1 foo
after [object RegExp], source=foo, global=true, ignoreCase=false, multiline=false, lastIndex=0 (number)
before [object RegExp], source=foo, global=true, ignoreCase=false, multiline=false, lastIndex=0 (number)
[object Array] 2 foo foo
after [object RegExp], source=foo, global=true, ignoreCase=false, multiline=false, lastIndex=0 (number)
before [object RegExp], source=foo, global=true, ignoreCase=true, multiline=false, lastIndex=0 (number)
[object Array] 2 Foo fOO
after [object RegExp], source=foo, global=true, ignoreCase=true, multiline=false, lastIndex=0 (number)
before [object RegExp], source=foo, global=true, ignoreCase=true, multiline=false, lastIndex=1 (string)
[object Array] 3 Foo fOO FOO
after [object RegExp], source=foo, global=true, ignoreCase=true, multiline=false, lastIndex=0 (number)
===*/

/* For non-global RegExp instances, match() logic is the same as normal
 * RegExp exec(): E5.1 Section 15.5.4.10, step 7.a calls exec() directly,
 * returning the exec() return value from match().
 *
 * Global RegExp instances get an entirely different handling in match(),
 * though exec() is used as a component.  In essence, match() finds all
 * matches, and creates a result array containing the matching strings
 * (match[0]) for each match.  'lastIndex' of the RegExp is updated in
 * the process.  If no matches are found, null is returned instead of an
 * empty array.  See E5.1 Section 15.5.4.10, step 8.
 *
 * match() algorithm initializes lastIndex to zero (without ever reading or
 * coercing it).  The exec() calls in match() step 8.f.i will update
 * lastIndex for both matching and non-matching case.  For the non-matching
 * case, lastIndex is written to zero (exec() step 9.a).  For the matching
 * case, lastIndex is written to end of match (exec() step 11.a).  The
 * match() algorithm has a special case for an empty match, which will
 * advance the scan index and also update lastIndex; step 8.f.iii.2.  Note
 * that since the global case match() loop only exits by a non-matching
 * exec() call, lastIndex should *always* be left to zero: a non-matching
 * exec() always leaves a zero in lastIndex (exec() step 9.a).
 *
 * In summary:
 *
 *   1) non-global match() essentially wraps a single regexp exec() call;
 *      lastIndex is written to zero if regexp does not match, and is not
 *      not touched if it matches.  It is always ToInteger() coerced.
 *   2) global match() resets lastIndex to zero (step 8.a) before it is
 *      read or coerced, so no lastIndex side effects should occur.
 *   3) global match() always leaves lastIndex as zero in the end.
 *
 * Note that 'lastIndex' could be an accessor property in principle and
 * this would expose the intermediate values used during match().  However,
 * 'lastIndex' is always initialized as a data property and is always
 * non-configurable so it cannot be changed into an accessor.
 *
 * Note that the expected match logic described above does not seem to match
 * V8 and Rhino (e.g. V8 seems to never update lastIndex).  Perhaps the
 * reasoning above is wrong?
 */

print('regexp');

function getRegExpDump(re) {
    return Object.prototype.toString.call(re) +
           ', source=' + re.source +
           ', global=' + re.global +
           ', ignoreCase=' + re.ignoreCase +
           ', multiline=' + re.multiline +
           ', lastIndex=' + re.lastIndex + ' (' + typeof re.lastIndex + ')';
}

function regExpTest() {
    var re;

    function test(this_val, regexp) {
        var t;
        var tmp = [];

        print('before', getRegExpDump(regexp));

        try {
            t = String.prototype.match.call(this_val, regexp);
            tmp.push(Object.prototype.toString.call(t));
            if (t !== null) {
                tmp.push(t.length);
                for (i = 0; i < t.length; i++) {
                    tmp.push(t[i]);
                }
                if (typeof t.input !== 'undefined') {
                    tmp.push('input=' + t.input);
                }
                if (typeof t.index !== 'undefined') {
                    tmp.push('index=' + t.index);
                }
            }
            print(tmp.join(' '));
        } catch (e) {
            print(e.name, e);
        }

        print('after', getRegExpDump(regexp));
    }

    // Non-global match, match not found -> null
    //
    // RegExp.prototype.exec() will be called; it will read 'lastIndex'
    // and coerce it with ToInteger() but then ignore the result when
    // global flag is false (E5.1 Section 15.10.6.2, steps 4, 5, 7).
    // If the match fails, lastIndex will be written to zero (step 9.a).

    re = /zoo/;
    re.lastIndex = '1';
    test('foobarfoobar', re);

    // Non-global match, match found.
    //
    // In the matching case for a non-global regexp, exec() *won't* write
    // lastIndex at all.  Also check that lastIndex ToInteger() coercion
    // occurs even when it is unused.

    re = /foo/;
    re.lastIndex = {
        toString: function() { print('toString() lastIndex'); return '123'; },
        valueOf: function() { print('valueOf() lastIndex'); return 1; }
    };
    test('foobarfoobar', re);

    // Global match, match not found -> null.
    //
    // lastIndex should be left as zero (always for global matches).

    re = /zoo/g;
    re.lastIndex = '1';  // lastIndex start position is ignored
    test('foobar', re);

    // Global match, single match found -> array with one element.

    test('foobar', /foo/g);

    // Other global match cases

    test('foobarfoobar', /foo/g);

    test('FoobarfOObar', /foo/gi);

    re = /foo/gi;
    re.lastIndex = '1';  // lastIndex start position is ignored
    test('FoobarfOObarFOObar', re);

    // XXX: RegExp.prototype.exec() replaced; still calls original function
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
object "true" 1 "rue"
object "false" 2 "lse"
object "123" 1 "23"
object "foobar" 3 "bar"
object "1,2,3" 2 "2,3"
object "[object Object]" 8 "Object"
object "foo" 0 ""
object "foo" 0 ""
object "foonullfoo" 3 "null"
object "footruefoo" 3 "true"
object "foofalsefoo" 3 "false"
object "foo123foo" 3 "123"
object "foobarfoo" 3 "bar"
object "foo1,2foo" 3 "1,2"
object "xxxOyyy" 3 "O"
TypeError
===*/

print('coercion');

function coercionTest() {
    function test(this_val, regexp_val, arg_count) {
        var t;

        try {
            if (arg_count === 0) {
                t = String.prototype.match.call();
            } else if (arg_count === 1) {
                t = String.prototype.match.call(this_val);
            } else {
                t = String.prototype.match.call(this_val, regexp_val);
            }

            // result is a RegExp match result
            print(typeof t, '"' + t.input + '"', t.index, '"' + t[0] + '"');
        } catch (e) {
            print(e.name);
        }
    }

    // this coercion
    test(undefined, undefined, 0);
    test(undefined, 'foo');
    test(null, 'foo');
    test(true, 'rue');
    test(false, 'lse');
    test(123, '23');
    test('foobar', 'bar');
    test([1,2,3], '2,3');
    test({ foo: 1, bar: 2 }, 'Object');

    // regexp argument: if non-regexp, it is given to the RegExp constructor,
    // which coerces using ToString() except for undefined, which is explicitly
    // coerced to an empty string.

    test('foo', undefined, 1);  // undefined -> new RegExp('') -> match at index 0
    test('foo', undefined);
    test('foonullfoo', null);
    test('footruefoo', true);
    test('foofalsefoo', false);
    test('foo123foo', 123);
    test('foobarfoo', 'b.r');   // string is always in regexp syntax
    test('foo1,2foo', [1,2]);

    // object is interesting, it ToString() coerces to "[object Object]" which
    // is interpreted as a character class.
    test('xxxOyyy', { foo: 1, bar: 2 });
    test('xxxAyyy', { foo: 1, bar: 2 });  // no match
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
