/*
 *  If a mark-and-sweep finalizer creates a new reachable object, the object
 *  must not be collected even though it has not been marked reachable.
 *  This is handled correctly by the implementation: finalizers run after
 *  the sweep pass has been completed and new elements and references created
 *  by the finalizer are considered in the next pass.
 */

/*===
removing references
obj1 and obj2 unreachable now
forced gc
finalizer obj1
bar
forced gc
bar
forced gc
bar
===*/

var newobj = null;

var obj1 = { name: 'obj1' };
Duktape.fin(obj1, function (o) {
    print('finalizer', o.name);
    newobj = { foo: 'bar' };
});
var obj2 = { name: 'obj2' };

obj1.ref = obj2;  // use circular reference to avoid refcount
obj2.ref = obj1;  // based collection
print('removing references');
obj1 = null;
obj2 = null;
print('obj1 and obj2 unreachable now');

print('forced gc');
Duktape.gc();  // finalizer executes and creates 'newobj'
print(newobj.foo);

print('forced gc');
Duktape.gc();  // 'newobj' survives collection
print(newobj.foo);

print('forced gc');
Duktape.gc();
print(newobj.foo);
