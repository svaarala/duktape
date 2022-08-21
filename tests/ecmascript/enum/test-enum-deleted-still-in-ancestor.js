/*
 *  Enumeration: field deleted but still present in ancestor.
 */

/*===
bar skip
foo inherited
===*/

/* The 'delete a.foo' will delete 'foo' from the 'a' object, not
 * the ancestor.  So, 'foo' should still enumerate.
 */

function F() {};
F.prototype = { "foo": "inherited" };

var a = new F();
a.bar = "skip";
a.foo = "own";  // overlapping field

// enumeration order: "bar", "foo"
for (var i in a) {
    delete a.foo;  // only affects 'a', not F.prototype
    print(i, a[i]);
}
