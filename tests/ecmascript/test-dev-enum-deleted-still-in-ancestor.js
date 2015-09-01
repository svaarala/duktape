/*
 *  Property deleted from enumeration target is still found from
 *  ancestor after deletion.  Should it be enumerated or not?
 */

/*===
bar skip
foo inherited
===*/

// Duktape and V8 enumerate 'foo', Rhino does not.

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
