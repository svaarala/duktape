/*
 *  RegExp called both as a function and as a constructor.
 *
 *  Argument coercions are tested specifically (E5 Section 15.10.3, 15.10.4),
 *  because 'undefined' has special treatment.
 */

var r, t;

/*
 *  RegExp called as a function
 */

/*===


true
true
TypeError
Foo
===*/

r = /foo/;

try {
    /* same as new RegExp(), which is the same as new RegExp('', ''); */
    t = RegExp();
    t = t.exec('');
    print(t[0]);
} catch (e) {
    print(e.name);
}

try {
    /* same as new RegExp(), which is the same as new RegExp('', ''); */
    /* NB: Rhino and Smjs apparently treat this as new RegExp('undefined', '') */
    t = RegExp(undefined, undefined);
    t = t.exec('');
    print(t[0]);
} catch (e) {
    print(e.name);
}

try {
    /* not the same as new RegExp(), special behavior */
    t = RegExp(r);
    print(r === t);  /* must be the *same* object */
} catch (e) {
    print(e.name);
}

try {
    /* not the same as new RegExp(), special behavior */
    t = RegExp(r, undefined);  /* must be the *same* object */
    print(r === t);
} catch (e) {
    print(e.name);
}

try {
    t = RegExp(r, 1);
    print('no error');
} catch (e) {
    print(e.name);
}

try {
    t = RegExp('foo', 'i');
    t = t.exec('Foo');
    print(t[0]);
} catch (e) {
    print(e.name);
}

/*
 *  RegExp called as a constructor
 */

/*===
false
TypeError
Foo
SyntaxError
SyntaxError
SyntaxError
SyntaxError
===*/

try {
    /* Note: different from RegExp(r) */
    t = new RegExp(r);
    print(r === t);  /* must NOT be the same object */
} catch (e) {
    print(e.name);
}

try {
    t = new RegExp(r, 1);
    print('no error');
} catch (e) {
    print(e.name);
}

try {
    t = new RegExp('foo', 'i');
    t = t.exec('Foo');
    print(t[0]);
} catch (e) {
    print(e.name);
}

try {
    t = new RegExp('foo', 'bar');
    print('no error');
} catch (e) {
    print(e.name);
}

try {
    t = new RegExp('foo', 'gg');
    print('no error');
} catch (e) {
    print(e.name);
}

try {
    t = new RegExp('foo', 'ii');
    print('no error');
} catch (e) {
    print(e.name);
}

try {
    t = new RegExp('foo', 'mm');
    print('no error');
} catch (e) {
    print(e.name);
}

/*
 *  Specific coercion tests
 */

/*===


null
SyntaxError
Foo Fo
===*/

/* Note: Rhino and Smjs coerce an explicitly given 'undefined' into "undefined",
 * so that e.g. new RegExp(undefined) is equivalent to /undefined/.  E5 requires
 * undefined to be coerced to an empty string in the first paragraph of Section
 * 15.10.4.1.
 */
try {
    t = new RegExp(undefined, undefined);  /* same as new RegExp("", "") */
    t = t.exec('');  /* should match */
    print(t[0]);
} catch (e) {
    print(e.name);
}

try {
    t = new RegExp();  /* same as above */
    t = t.exec('');  /* should match */
    print(t[0]);
} catch (e) {
    print(e.name);
}

try {
     t = new RegExp(null);  /* same as new RegExp("null", "") */
     t = t.exec('nullx');
     print(t[0]);
} catch (e) {
    print(e.name);
}

try {
    /* should cause SyntaxError, invalid flags */
    t = new RegExp('foo', null);  /* same as new RegExp("foo", "null") */
    print('no error');
} catch (e) {
    print(e.name);
}

try {
    /* ensure ToString(a) = "(fo)o", ToString(b) = "i" */
    var a = {};
    a.toString = function() { return '(fo)o'; };
    var b = {};
    b.toString = function() { return 'i'; };

    t = new RegExp(a, b);
    t = t.exec('Foo');
    print(t[0], t[1]);
} catch (e) {
    print(e.name);
}
