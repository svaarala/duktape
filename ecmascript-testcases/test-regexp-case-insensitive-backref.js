/*
 *  Spot check of case insensitive backref matching.
 */

var m;

/*===
fooFOO foo FOO
===*/

m = /^(...)(\1)$/i.exec('fooFOO');
print(m[0], m[1], m[2]);
