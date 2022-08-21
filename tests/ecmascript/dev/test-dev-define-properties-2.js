/*===
FOO GET
BAR GET
===*/

/* A pretty basic test: use Object.defineProperties() to create two
 * getter properties.
 */

var obj={};
Object.defineProperties(obj, {
    foo: { get: function() { print("FOO GET"); } },
    bar: { get: function() { print("BAR GET"); } }
});

void obj.foo;
void obj.bar;
