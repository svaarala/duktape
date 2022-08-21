/*
 *  Exec() coercion.
 */

var r1 = /undefined/;
var r2 = /null/;
var r3 = /5/;
var r4 = /foo/;
var t;

/*
 *  Coercion of exec() input is ToString() regardless of type.
 */

/*===
undefined
undefined
null
5
foo
===*/

/* Rhino and smjs fail this */
try {
    /* match against ToString(undefined) = 'undefined' */
    t = r1.exec();
    print(t[0]);
} catch (e) {
    print(e.name);
}

try {
    /* same as above */
    t = r1.exec(undefined);
    print(t[0]);
} catch (e) {
    print(e.name);
}

try {
    /* match against ToString(null) = 'null' */
    t = r2.exec(null);
    print(t[0]);
} catch (e) {
    print(e.name);
}

try {
    t = r3.exec(5);
    print(t[0]);
} catch (e) {
    print(e.name);
}

try {
    a = {};
    a.toString = function() { return 'foobar'; };
    t = r4.exec(a);
    print(t[0]);
} catch (e) {
    print(e.name);
}
