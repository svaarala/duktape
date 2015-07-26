/*---
{
    "custom": true
}
---*/

/*===
finalizer, foo -> 123
===*/

var a;

/* Note: inside function to ensure that no reference to the value can
 * be active in temporary registers.
 */
function init() {
    a = { foo: 123 };

    Duktape.fin(a, function (x) {
        print('finalizer, foo ->', x.foo);
    });
}

init();

// refcount finalizing happens here
a = null;

// mark-and-sweep finalizing happens here if refcount disabled
Duktape.gc();
