/*---
{
    "nonstandard": true
}
---*/

// indirect eval -> this is bound to the global object, E5 Section 10.4.2, step 1.a.
var g = (function () { var e = eval; return e('this'); } )();

/*===
[object global]
===*/

/* [[Class]] implementation defined, but we expect 'global' */

try {
    print(Object.prototype.toString.call(g));
} catch (e) {
    print(e.name);
}
