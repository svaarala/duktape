/*---
{
    "skip": true
}
---*/

/*===
- step 1
TypeError
TypeError
TypeError
TypeError
- step 2
TypeError
object
true
- step 5
here
- step 9
object true
undefined undefined
object aiee
string aiee
done
===*/

var GLOBAL = new Function('return this;')();

// ES2015 Section 25.4.3.1 step 1.
print('- step 1');
try {
    Promise();
    print('never here');
} catch (e) {
    print(e.name);
}
try {
    Promise(function () {});
    print('never here');
} catch (e) {
    print(e.name);
}
try {
    Reflect.construct(Promise, [], void 0);
    print('never here');
} catch (e) {
    print(e.name);
}
try {
    Reflect.construct(Promise, [ function () {} ], void 0);
    print('never here');
} catch (e) {
    print(e.name);
}

// Step 2.
print('- step 2');
try {
    var p = new Promise(123);
    print('never here');
} catch (e) {
    print(e.name);
}
try {
    var p = new Promise(function () {});
    print(typeof p);
    print(p instanceof Promise);  // Covers step 11, return value is a Promise.
} catch (e) {
    print(e);
}

// Step 3:
// - https://www.ecma-international.org/ecma-262/6.0/#sec-ordinarycreatefromconstructor
// - internal slots list is interesting, but cannot be verified from behavior alone?

// Step 4, n/a

// Step 5, promise is pending at return (unless error thrown in executor).
print('- step 5');
try {
    var p = new Promise(function () {});
    p.then(function () {
        print('fulfill: NEVER CALLED');
    }, function () {
        print('reject: NEVER CALLED');
    });
    print('here');
} catch (e) {
    print(e);
}

// Step 6 and 7:
// - internal reactions lists are empty, but cannot be verified from behavior alone?

// Step 9:
// - called with 'undefined' this (check strict coercion)
// - bound function case
print('- step 9');
try {
    var p = new Promise(function () {
        print(typeof this, this === GLOBAL);
    });
    var p = new Promise(function () {
        'use strict';
        print(typeof this, this);
    });
    var p = new Promise(function () {
        print(typeof this, String(this));
    }.bind('aiee'));
    var p = new Promise(function () {
        'use strict';
        print(typeof this, this);
    }.bind('aiee'));
} catch (e) {
    print(e);
}

print('done');
