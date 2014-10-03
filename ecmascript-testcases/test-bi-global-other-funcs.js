// indirect eval -> this is bound to the global object, E5 Section 10.4.2, step 1.a.
var g = (function () { var e = eval; return e('this'); } )();

/*===
3
===*/

try {
    // very simple eval test, there are stricter eval tests elsewhere
    print(g.eval('1+2'));
} catch (e) {
    print(e.name);
}

/*===
true
false
false
false
false
false
false
false
false
true
false
true
false
false
false
true
true
===*/

try {
    print(g.isNaN(undefined));
    print(g.isNaN(null));
    print(g.isNaN(false));
    print(g.isNaN(true));
    print(g.isNaN(Number.NEGATIVE_INFINITY));
    print(g.isNaN(-123.0));
    print(g.isNaN(-0.0));
    print(g.isNaN(+0.0));
    print(g.isNaN(Number.POSITIVE_INFINITY));
    print(g.isNaN(0 / 0));
    print(g.isNaN(''));  // empty string -> 0
    print(g.isNaN('foo'));
    print(g.isNaN('123'));
    print(g.isNaN([]));  // empty string -> 0
    print(g.isNaN([1])); // "1" -> 1
    print(g.isNaN([1,2])); // "1,2" -> NaN
    print(g.isNaN({}));  // -> NaN
} catch (e) {
    print(e.name, e);
}

/*===
false
true
true
true
false
true
true
true
false
false
true
false
true
true
true
false
false
===*/

try {
    print(g.isFinite(undefined));
    print(g.isFinite(null));
    print(g.isFinite(false));
    print(g.isFinite(true));
    print(g.isFinite(Number.NEGATIVE_INFINITY));
    print(g.isFinite(-123.0));
    print(g.isFinite(-0.0));
    print(g.isFinite(+0.0));
    print(g.isFinite(Number.POSITIVE_INFINITY));
    print(g.isFinite(0 / 0));
    print(g.isFinite(''));  // empty string -> 0
    print(g.isFinite('foo'));
    print(g.isFinite('123'));
    print(g.isFinite([]));  // empty string -> 0
    print(g.isFinite([1])); // "1" -> 1
    print(g.isFinite([1,2])); // "1,2" -> NaN
    print(g.isFinite({}));  // -> NaN
} catch (e) {
    print(e.name, e);
}
