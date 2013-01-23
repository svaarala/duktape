
/*===
nulling
nulled
finalizer, foo -> 123
===*/

a = { foo: 123 };
b = {}

__duk__.setFinalizer(a, function (x) {
    print('finalizer, foo ->', x.foo);
});

// circular reference prevents refcount collection
a.bar = b;
b.bar = a;

// refcount does not finalize here
print('nulling')
a = null;
b = null;
print('nulled')

// mark-and-sweep finalizing happens here if refcount disabled
__duk__.gc();

