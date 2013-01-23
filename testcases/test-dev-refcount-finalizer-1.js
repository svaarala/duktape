
/*===
finalizer, foo -> 123
===*/

a = { foo: 123 };

__duk__.setFinalizer(a, function (x) {
    print('finalizer, foo ->', x.foo);
});

// refcount finalizing happens here
a = null;

// mark-and-sweep finalizing happens here if refcount disabled
__duk__.gc();

