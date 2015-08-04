/*
 *  Duktape 0.11.0 bug: if a Proxy is reachable at heap destruction and the
 *  target object has a finalizer, the finalizer will be executed both for
 *  the target object and the Proxy object.  Normal run-time refcount and
 *  mark-and-sweep finalizer code will not finalize a Proxy.
 *
 *  See: https://github.com/svaarala/duktape/issues/45
 */

/*===
about to exit
finalize [object Object]
===*/

function finalizer(o) {
    print('finalize', o);
}

var obj = { name: 'obj' };
Duktape.fin(obj, finalizer);
var proxy = new Proxy(obj, {});
// proxy = null;  // would trigger refzero finalization, which ignores Proxy objects

/* Both 'obj' and 'proxy' are reachable here, and will only be collected
 * at heap destruction, being registered to the global object.
 *
 * The heap destruction code will forcibly execute finalizers for all
 * objects remaining on the heap, even for reachable ones.  Unlike the
 * normal finalization path, it would execute a finalizer for the Proxy
 * object too in Duktape 0.11.0, so that the finalizer gets executed twice.
 *
 * This was fixed in Duktape 0.12.0.
 */

print('about to exit');
