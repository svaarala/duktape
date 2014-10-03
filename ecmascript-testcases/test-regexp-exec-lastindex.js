/*
 *  Exec() lastIndex behavior is a bit peculiar, especially for non-global
 *  regexps (= regexps whose global flag is false).
 *
 *  For regexps with global=true, the behavior is quite straightforward:
 *  read lastIndex, and write it after matching regardless of success or
 *  failure of matching.
 *
 *  For regexps with global=false, it is a bit odd (E5 Section 15.10.6.2):
 *
 *    1. lastIndex is read from the object and coerced to integer (steps 4 and 5)
 *    2. the index is reset to zero if global=false (step 7)
 *    3. if the input is exhausted during matching, lastIndex is *written* to
 *       zero and a null is returned (step 9.a)
 *    4. if a match occurs, lastIndex is *not* written (step 11)
 */

var r, t;
var a, b;

/*
 *  Non-global regexp, normal use.
 */

/*===
0
0
0
0
0
===*/

try {
    r = /foo/;
    print(r.lastIndex);
    t = r.exec('bar');      /* no match -> reset to zero (but stays the same here, since it is already 0) */
    print(r.lastIndex);
    t = r.exec('foo');      /* match -> don't touch */
    print(r.lastIndex);
    t = r.exec('foofoo');   /* match -> don't touch */
    print(r.lastIndex);
    t = r.exec('foofoo');   /* match -> don't touch */
    print(r.lastIndex);
} catch (e) {
    print(e.name);
}

/*
 *  Non-global regexp, lastIndex should have no effect on matching
 *  and should only change if a match is not found.
 */

/*===
0
-1
foo
-1
0
9999
foo
9999
===*/

try {
    r = /foo/;
    print(r.lastIndex);
    r.lastIndex = -1;
    print(r.lastIndex);
    t = r.exec('foo');   /* match -> don't touch */
    print(t[0]);
    print(r.lastIndex);

    r = /foo/;
    print(r.lastIndex);
    r.lastIndex = 9999;
    print(r.lastIndex);
    t = r.exec('foo');   /* match -> don't touch */
    print(t[0]);
    print(r.lastIndex);
} catch (e) {
    print(e.name);
}


/*
 *  Non-global regexp, lastIndex is not written to if a match is found,
 *  even if lastIndex needs coercion.
 *
 *  Note: Smjs and Rhino seems to coerce lastIndex to number when
 *  it is written, so they fail this test.  lastIndex is just a
 *  normal writable property, see E5 Section 15.10.7.5.  It is
 *  coerced to integer *when used*.
 */

/*===
0 number
1 string
1 string
1 string
1 string
0 number
1 object
1 object
1 object
1 object
true
===*/

try {
    r = /foo/;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = "1";
    print(r.lastIndex, typeof r.lastIndex);
    r.exec('foo');
    print(r.lastIndex, typeof r.lastIndex);
    r.exec('foofoo');
    print(r.lastIndex, typeof r.lastIndex);
    r.exec('foofoo');
    print(r.lastIndex, typeof r.lastIndex);

    r = /foo/;
    print(r.lastIndex, typeof r.lastIndex);
    a = new String("1");
    r.lastIndex = a;
    print(r.lastIndex, typeof r.lastIndex);
    r.exec('foo');
    print(r.lastIndex, typeof r.lastIndex);
    r.exec('foofoo');
    print(r.lastIndex, typeof r.lastIndex);
    r.exec('foofoo');
    print(r.lastIndex, typeof r.lastIndex);
    print(r.lastIndex === a);   /* instance should still be the same */
} catch (e) {
    print(e.name);
}

/*
 *  Non-global regexp, more lastIndex behavior when match is found
 *  and when it is not.
 */

/*===
0 number
0 number
0 number
1 string
1 string
0 number
-1 number
-1 number
0 number
1000000000000 number
1000000000000 number
0 number
1 string
0 number
===*/

/* match: zero is not written to */
try {
    r = /foo/;
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('foobar');
    print(r.lastIndex, typeof r.lastIndex);
} catch (e) {
    print(e.name);
}

/* match: string '1' is not written to, and is ignored */
try {
    r = /foo/;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = '1';
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('foobar');
    print(r.lastIndex, typeof r.lastIndex);
} catch (e) {
    print(e.name);
}

/* match: number -1 is not written to, and is ignored */
try {
    r = /foo/;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = -1;
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('foobar');
    print(r.lastIndex, typeof r.lastIndex);
} catch (e) {
    print(e.name);
}

/* match: a large number is not written to, and is ignored */
try {
    r = /foo/;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = 1000000000000;  /* above 2^32 */
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('foobar');
    print(r.lastIndex, typeof r.lastIndex);
} catch (e) {
    print(e.name);
}

/* no match: previous value is overwritten with zero (step 9.a.i),
 * regardless of the global flag!
 *
 * Rhino and smjs fail this test.
 */
try {
    r = /foo/;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = '1';
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('bar');
    print(r.lastIndex, typeof r.lastIndex);
} catch (e) {
    print(e.name);
}

/*
 *  Global regexp, normal use.
 */

/*===
0
foo
3
null
0
0
foo
3
Foo
6
FOO
9
null
0
===*/

try {
    r = /foo/g;
    print(r.lastIndex);
    t = r.exec('foo');
    print(t[0]);
    print(r.lastIndex);
    t = r.exec('foo');
    print(t);
    print(r.lastIndex);

    r = /foo/gi;
    print(r.lastIndex);
    t = r.exec('fooFooFOO');
    print(t[0]);
    print(r.lastIndex);
    t = r.exec('fooFooFOO');
    print(t[0]);
    print(r.lastIndex);
    t = r.exec('fooFooFOO');
    print(t[0]);
    print(r.lastIndex);
    t = r.exec('fooFooFOO');
    print(t);
    print(r.lastIndex);
} catch (e) {
    print(e.name);
}

/*
 *  Global regexp, change input string while lastIndex changes,
 *  just a sanity check.
 */

/*===
0
foo
3
FOO
6
null
0
===*/

try {
    r = /foo/gi;
    print(r.lastIndex);

    t = r.exec('foo');    /* match, leave at 3 */
    print(t[0]);
    print(r.lastIndex);

    t = r.exec('barFOO');
    print(t[0]);
    print(r.lastIndex);

    t = r.exec('foo');    /* starts at 6, out of bounds => null and 0 */
    print(t);
    print(r.lastIndex);
} catch (e) {
    print(e.name);
}

/*
 *  Coercion and update of a non-number lastIndex.  Out of bounds lastIndex
 *  causes exec() to return null, and reset lastIndex to zero.
 */

/*===
0
-1
null
0
999
null
0
object
2
Foo
3
6
number
===*/

try {
    r = /foo/gi;
    print(r.lastIndex);

    r.lastIndex = -1;
    print(r.lastIndex);
    t = r.exec('foo');
    print(t);
    print(r.lastIndex);

    r.lastIndex = 999;
    print(r.lastIndex);
    t = r.exec('foo');
    print(t);
    print(r.lastIndex);

    a = {};
    a.valueOf = function() { return 2; };
    r.lastIndex = a;
    print(typeof r.lastIndex);
    print(r.lastIndex.valueOf());
    t = r.exec('fooFoo');   /* start matching at 2, find match at 3 for 'Foo' */
    print(t[0]);
    print(t.index);
    print(r.lastIndex);
    print(typeof r.lastIndex);
} catch (e) {
    print(e.name);
}

/*
 *  lastIndex is coerced with ToInteger() which -allows- values
 *  larger than 32 bits.  For instance, 0x100000000 must NOT be
 *  confused with 0x00000000 as would happen if lastIndex were
 *  coerced with ToUint32() for instance.
 */

/*===
0 number
foo
4294967297 number
0 number
null
0 number
===*/

try {
    /* Non-global regexp: lastIndex is ignored (matching starts from char
     * index 0) -> match.  lastIndex is not updated for non-global regexps
     * when a match happens.
     */
    r = /foo/;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = 4294967297.0;   /* 0x100000001 */
    t = r.exec('foofoofoo');
    print(t);
    print(r.lastIndex, typeof r.lastIndex);
} catch (e) {
    print(e.name);
}

try {
    /* Global regexp: respects lastIndex -> no match.  On a non-match
     * lastIndex is zeroed.
     */
    r = /foo/g;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = 4294967297.0;   /* 0x100000001 */
    t = r.exec('foofoofoo');   /* no match, lastIndex is reset to zero */
    print(t);
    print(r.lastIndex, typeof r.lastIndex);
} catch (e) {
    print(e.name);
}

/*
 *  lastIndex can be NAN or +/- Infinity.  These have well defined
 *  behavior.  If the global flag is not set, they are ignored (but
 *  overwritten if there is no match).  If the global flag is set,
 *  NAN is coerced to +0.0 by ToInteger(), while +/- Infinity is used
 *  as is.  It will cause match to fail at step 9.a, and lastIndex to
 *  be overwritten with zero.
 *
 *  The sign of a zero is preserved by ToInteger().  It must be preserved
 *  for non-global regexps which -do match-.
 */

/*===
0 number
NaN number
foo
NaN number
null
0 number Infinity
0 number
NaN number
foo
3 number
null
0 number Infinity
===*/

try {
    r = /foo/;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = NaN;
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('foo');   /* match -> don't update */
    print(t[0]);
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('bar');   /* no match -> overwrite with zero */
    print(t);
    print(r.lastIndex, typeof r.lastIndex, 1.0 / r.lastIndex);   /* check sign too */
} catch (e) {
    print(e.name);
}

try {
    r = /foo/g;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = NaN;
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('foo');   /* match -> update */
    print(t[0]);
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('bar');   /* no match -> overwrite with zero */
    print(t);
    print(r.lastIndex, typeof r.lastIndex, 1.0 / r.lastIndex);   /* check sign too */
} catch (e) {
    print(e.name);
}

/*===
0 number
Infinity number
foo
Infinity number
null
0 number Infinity
0 number
Infinity number
null
0 number Infinity
===*/

try {
    r = /foo/;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = Infinity;
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('foo');   /* match -> don't update */
    print(t[0]);
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('bar');   /* no match -> overwrite with zero */
    print(t);
    print(r.lastIndex, typeof r.lastIndex, 1.0 / r.lastIndex);   /* check sign too */
} catch (e) {
    print(e.name);
}

try {
    r = /foo/g;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = Infinity;
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('foo');   /* no match (since we start from after end of string -> overwrite */
    print(t);
    print(r.lastIndex, typeof r.lastIndex, 1.0 / r.lastIndex);   /* check sign too */
} catch (e) {
    print(e.name);
}

/*===
0 number
-Infinity number
foo
-Infinity number
null
0 number Infinity
0 number
-Infinity number
null
0 number Infinity
===*/

try {
    r = /foo/;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = -Infinity;
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('foo');   /* match -> don't update */
    print(t[0]);
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('bar');   /* no match -> overwrite with zero */
    print(t);
    print(r.lastIndex, typeof r.lastIndex, 1.0 / r.lastIndex);   /* check sign too */
} catch (e) {
    print(e.name);
}

try {
    r = /foo/g;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = -Infinity;
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('foo');   /* no match (since we start from after end of string -> overwrite */
    print(t);
    print(r.lastIndex, typeof r.lastIndex, 1.0 / r.lastIndex);   /* check sign too */
} catch (e) {
    print(e.name);
}

/*===
0 number Infinity
0 number -Infinity
foo
0 number -Infinity
null
0 number Infinity
0 number Infinity
0 number -Infinity
foo
3 number
null
0 number Infinity
===*/

try {
    r = /foo/;
    print(r.lastIndex, typeof r.lastIndex, 1.0 / r.lastIndex);
    r.lastIndex = -0.0;
    print(r.lastIndex, typeof r.lastIndex, 1.0 / r.lastIndex);
    t = r.exec('foo');   /* match -> don't update */
    print(t[0]);
    print(r.lastIndex, typeof r.lastIndex, 1.0 / r.lastIndex);
    t = r.exec('bar');   /* no match -> overwrite with zero (positive) */
    print(t);
    print(r.lastIndex, typeof r.lastIndex, 1.0 / r.lastIndex);   /* check sign too */
} catch (e) {
    print(e.name);
}

try {
    r = /foo/g;
    print(r.lastIndex, typeof r.lastIndex, 1.0 / r.lastIndex);
    r.lastIndex = -0.0;
    print(r.lastIndex, typeof r.lastIndex, 1.0 / r.lastIndex);
    t = r.exec('foo');   /* match -> update */
    print(t[0]);
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('bar');   /* no match -> overwrite with zero (positive) */
    print(t);
    print(r.lastIndex, typeof r.lastIndex, 1.0 / r.lastIndex);   /* check sign too */
} catch (e) {
    print(e.name);
}

/*
 *  The lastIndex value is floored.
 */

/*===
0 number
0.9 number
foo
0.9 number
0 number
0.9 number
foo
3 number
===*/

try {
    r = /foo/;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = 0.9;
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('foo');
    print(t[0]);
    print(r.lastIndex, typeof r.lastIndex);
} catch (e) {
    print(e.name);
}

try {
    r = /foo/g;
    print(r.lastIndex, typeof r.lastIndex);
    r.lastIndex = 0.9;
    print(r.lastIndex, typeof r.lastIndex);
    t = r.exec('foo');
    print(t[0]);
    print(r.lastIndex, typeof r.lastIndex);
} catch (e) {
    print(e.name);
}
