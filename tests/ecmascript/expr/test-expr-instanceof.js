/*
 *  'instanceof' operator (E5 Section 11.8.6).
 */

/*===
true
TypeError
false
===*/

var t;

print(new Error('test') instanceof Error);

// RHS must be callable
try {
    t = new Error('test');
    print(t instanceof t);
} catch (e) {
    print(e.name);
}

// an object is not an instance of itself (must be callable for
// this test to make sense)
function foo() {}
print(foo instanceof foo);

/* More coverage is provided by: test-expr-relop.js */
