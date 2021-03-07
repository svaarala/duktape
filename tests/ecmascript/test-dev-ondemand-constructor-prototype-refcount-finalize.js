/*---
{
    "custom": true
}
---*/

function FX() {
    print('finalize X');
}
function FY() {
    print('finalize Y');
}

print('- prototype in func');
var f = function () {};
print('prototype' in f);
Duktape.gc(); Duktape.gc();

print('- own property names');
var f = function () {};
print(Object.getOwnPropertyNames(f).join(','));
f.prototype.foo = 'bar';
print(Object.getOwnPropertyNames(f).join(','));
Duktape.gc(); Duktape.gc();

print('- refcount finalization, anonymous function');
var X = function () {};
Duktape.fin(X, FX);
print('setting X to null');
X = null;
print('X set to null');
Duktape.gc(); Duktape.gc();

print('- refcount finalization, named function');
var X = function foo() {};
Duktape.fin(X, FX);
print('setting X to null');
X = null;
print('X set to null');
Duktape.gc(); Duktape.gc();

print('- refcount finalization, anonymous inner function, reference not kept');
var X = function () {
    var Y = function () {};
    Duktape.fin(Y, FY);
    Y = null;
};
Duktape.fin(X, FX);
print('call X');
X();
print('setting X to null');
X = null;
print('X set to null');
Duktape.gc(); Duktape.gc();

print('- refcount finalization, anonymous inner function, reference kept');
var X = function () {
    var Y = function () {};
    Duktape.fin(Y, FY);
    // At this point the anonymous inner function points to the outer scope,
    // which holds a reference back to the function via 'Y', so cannot
    // collect via refcounting.
};
Duktape.fin(X, FX);
print('call X');
X();
print('setting X to null');
X = null;
print('X set to null');
Duktape.gc(); Duktape.gc();

print('done');
