/*---
{
    "skip": true
}
---*/

/*===
overridden then() called
overridden then() called
done
then fulfill 1: 123
then reject 2: aiee
===*/

// Promise.prototype.then() may be overridden, which affects some opportunities
// for optimization.

var oldThen = Promise.prototype.then;
Promise.prototype.then = function (onFulfilled, onRejected) {
    print('overridden then() called');
    return oldThen.call(this, onFulfilled, onRejected);
}

var P = Promise.resolve(123);
P.then(function (v) {
    print('then fulfill 1:', v);
    throw 'aiee';
}, function (e) {
    print('then reject 1:', String(e));
}).then(function (v) {
    print('then fulfill 2:', v);
}, function (e) {
    print('then reject 2:', String(e));
});

print('done');
