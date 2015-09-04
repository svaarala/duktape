/*===
1 2
undefined 2
undefined undefined
===*/

var obj = {foo:1, bar:2};

print(obj.foo, obj.bar);
delete obj.foo;
print(obj.foo, obj.bar);
delete obj['bar'];
print(obj.foo, obj.bar);

/*===
true
false
false
true
===*/

/* Attempting to delete an unresolvable reference is 'true' in non-strict mode.
 * Deleting a resolvable reference returns 'true' or 'false', depending on
 * whether the target is deletable.
 */

this.deletable_ref = 1;  /* configurable property in global object -> deletable */

try {
    /* unresolvable (at run time) */

    /* Note: Rhino will throw a TypeError:
     * "js: uncaught JavaScript runtime exception: TypeError: Cannot delete property "foo" of null"
     *
     * V8 works correctly.
     */

    print(delete foo);
} catch (e) {
    print(e.name);
}

try {
    /* resolvable, maps to global object, cannot be deleted */
    print(delete NaN);  /* this.NaN is a non-configurable property */
} catch (e) {
    print(e.name);
}

try {
    /* resolvable, a non-deletable variable binding, cannot be deleted */
    eval("function f_vardel() { var x; print(delete x); }; f_vardel();");
} catch (e) {
    print(e.name);
}

try {
    /* resolvable, maps to global object, can be deleted */
    print(delete deletable_ref);
} catch (e) {
    print(e.name);
}


/*===
SyntaxError
SyntaxError
SyntaxError
SyntaxError
===*/

/* Attempting to delete an unresolvable reference or any identifier
 * in strict mode is a compile-time SyntaxError.
 */

try {
    eval("function f1(x) { 'use strict'; delete x; }; f1();");
} catch (e) {
    print(e.name);
}

try {
    eval("function f2() { 'use strict'; var x = 1; delete x; }; f2();");
} catch (e) {
    print(e.name);
}

try {
    eval("function f3() { 'use strict'; function x() {}; delete x; }; f3();");
} catch (e) {
    print(e.name);
}

try {
    // parseInt resides in the global object, which is bound to the global
    // environment record
    eval("function f4() { 'use strict'; delete parseInt; }; f4();");
} catch (e) {
    print(e.name);
}

try {
} catch (e) {
    print(e.name);
}

/*===
true
test
true
===*/

/* Deleting a non-reference is always 'true', even in strict mode. */

try {
    print(delete 1);
} catch (e) {
    print(e.name);
}

try {
    print(delete print('test'));
} catch (e) {
    print(e.name);
}

/*===
1
true
true
false
undefined
===*/

/* Parenthesis must not affect behavior */

obj = {foo:1};
Object.defineProperty(obj, 'bar', { value: 123, writable: false, enumerable: false, configurable: false });

try {
    print(obj.foo);
    print(delete ((obj['foo'])));
    print(delete ((obj['nonexistent'])));  // undefined property -> 'true'
    print(delete ((obj['bar'])));  // non-conigurable -> 'false'
    print(obj.foo);
} catch (e) {
    print(e.name);
}
