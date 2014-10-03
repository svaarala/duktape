/*
 *  Property deleted from enumeration target is still found from
 *  ancestor after deletion.  Should it be enumerated or not?
 */

/*---
{
    "skip": true
}
---*/

/*===
bar skip
foo inherited
===*/

function F() {};
F.prototype = { "foo": "inherited" };

var a = new F();
a.bar = "skip";
a.foo = "own";

// enumeration order: "bar", "foo"
for (var i in a) {
    delete a.foo;  // only affects 'a', not F.prototype
    print(i, a[i]);
}
