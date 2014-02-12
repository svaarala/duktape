/*---
{
    "custom": true
}
---*/

/*===
finalizer
object
finalizer
object
finalizer
===*/

a={};

// The finalizer rescues the reference (= makes it reachable again)
Duktape.fin(a, function(x) { print('finalizer'); a=x });

a = null;
print(typeof a);
a = null;
print(typeof a);

// At this point 'a' is reachable again, and when the heap is destroyed,
// the finalizer runs once more.
