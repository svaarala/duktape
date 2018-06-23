// Rejected promise with rejection handled before Promise settled.
// Test for both callable and non-callable handler.

/*---
{
    "skip": true
}
---*/

/*@include util-promise.js@*/

/*===
nop called
nop called
unhandled/reject: R2
tick
===*/

var P, Q, R;

setupPromiseUnhandledCallbacks(function (p) {
    print('unhandled/reject:', p.name);
}, function (p) {
    print('unhandled/handle:', p.name);
});

function nop() {
    print('nop called');
}

P = Promise.reject(123);
P.name = 'P';
Q = Promise.reject(234);
Q.name = 'Q';
R = Promise.reject(345);
R.name = 'R';

P.catch(nop);
Q.catch(nop);
// Handler is not callable.  R itself is considered handled because
// it is directly bound to R2.  However, R2 will be unhandled.
var R2 = R.catch(123)
R2.name = 'R2';

promiseNextTick(function (cb) {
    print('tick');
});
