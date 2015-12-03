/*
 *  Test the second finalizer argument, a boolean which is true if we're in
 *  heap destruction and cannot rescue the object.
 */

/*---
{
    "custom": true
}
---*/

/*===
finalizer 2: object boolean false
finalizer 1: object boolean true
===*/

var obj1 = {};
Duktape.fin(obj1, function myFinalizer(o, heapDestruct) {
    print('finalizer 1:', typeof o, typeof heapDestruct, heapDestruct);
});

var obj2 = {};
Duktape.fin(obj2, function myFinalizer(o, heapDestruct) {
    print('finalizer 2:', typeof o, typeof heapDestruct, heapDestruct);
});

obj2 = null;
Duktape.gc();  // Force obj2 collection before heap destruct

// Let heap destruction handle obj1.
