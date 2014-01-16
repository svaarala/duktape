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
===*/

a={};

// The finalizer rescues the reference (= makes it reachable again)
Duktape.setFinalizer(a, function(x) { print('finalizer'); a=x });

a = null;
print(typeof a);
a = null;
print(typeof a);

