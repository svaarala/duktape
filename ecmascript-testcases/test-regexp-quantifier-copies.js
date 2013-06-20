
var r, t;

/*===
abbababbab,b object
===*/

try {
    r = eval("/^(a|b){10}$/");
    t = r.exec('abbababbab');
    print(t, typeof t);
} catch (e) {
    print(e.name);
}

/*===
null object
===*/

try {
    r = eval("/^(a|b){1000}$/");
    t = r.exec('abbababbab');
    print(t, typeof t);
} catch (e) {
    print(e.name);
}

/*===
Error
===*/

/* This is supposed to work by the specification, but the implementation
 * imposes an internal maximum on atom copy count.
 *
 * So we test for that behavior.  Rhino and V8 (and others) will thus
 * intentionally fail this test.
 */

/* FIXME: Error -> InternalError or RangeError? */

try {
    r = eval("/^(a|b){100000}$/");
    t = r.exec('abbababbab');
    print(t, typeof t);
} catch (e) {
    print(e.name);
}

