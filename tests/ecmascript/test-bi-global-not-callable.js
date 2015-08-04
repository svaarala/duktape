// indirect eval -> this is bound to the global object, E5 Section 10.4.2, step 1.a.
var g = (function () { var e = eval; return e('this'); } )();

/*===
TypeError
TypeError
===*/

try {
    // not constructable -> TypeError
    new g();
} catch (e) {
    print(e.name);
}

try {
    // not callable -> TypeError
    new g();
} catch (e) {
    print(e.name);
}
