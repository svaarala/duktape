// A nop handler ('Thrower') doesn't count as being handled.

/*---
{
    "skip": true
}
---*/

/*@include util-promise.js@*/

/*===
234
unhandled/reject: P2
tick
===*/

var P, Q, P2, Q2;

setupPromiseUnhandledCallbacks(function (p) {
    print('unhandled/reject:', p.name);
}, function (p) {
    print('unhandled/handle:', p.name);
});

P = Promise.reject(123);
P.name = 'P';
Q = Promise.reject(234);
Q.name = 'Q';

// When .then() is applied to P, P's reject handler will be set to P2's
// reject() function, so P's rejection is considered handled.  However,
// P2 itself is unhandled because its onRejected is a Thrower.
P2 = P.then(function (v) {
    print(v);
}, void 0);
P2.name = 'P2';
Q2 = Q.then(function (v) {
    print(v);
}, function (e) {
    print(e);
});
Q2.name = 'Q2';

promiseNextTick(function () {
    print('tick');
});
