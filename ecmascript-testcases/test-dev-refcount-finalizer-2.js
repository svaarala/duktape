/*---
{
    "custom": true
}
---*/

/*===
object
Finalizer
object
Finalizer's finalizer
undefined
===*/

/* Finalizer of a finalizer */

var func1 = function(x) { print("Finalizer's finalizer"); };
var func2 = function(x) { print("Finalizer"); obj = x; /*rescue*/ };
var obj = {};

Duktape.fin(obj, func2);
Duktape.fin(func2, func1);
func1 = null;
func2 = null;

print(typeof obj);
obj = null;  // func2 will rescue
print(typeof obj);

// Refcount of func2 should drop to zero, which should cause func'2
// finalizer, func1, to be executed.  Once func2 is freed, func1's
// refcount should also drop to zero, and func1 should be freed
// (without a finalizer call).
//
// However, this does not happen right now because every function
// closure is by default participates in a reference loop through
// its automatic prototype (e.g. f.prototype.constructor === f).

Duktape.fin(obj, null);

// Explicit GC causes the finalizer's finalizer to run.

Duktape.gc();

obj = undefined;
print(typeof obj);
