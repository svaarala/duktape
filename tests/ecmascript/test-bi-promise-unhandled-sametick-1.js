// Rejected promise with rejection handled after Promise settled during
// same tick.  Test for both callable and non-callable handler.
//
// Rejected Promises are created in the first T fulfill reaction, and
// resolved in the next.  Two callbacks (promise jobs) are involved, but
// since they're both "runnable" at the same time, they are considered to
// happen in the same "tick", suppressing the unhandled rejection notify.
// (At least in Node.js.)

/*---
{
    "skip": true
}
---*/

/*@include util-promise.js@*/

/*===
T1.then
T2.then
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
var T = Promise.resolve();
T.name = 'T';

var T1 = T.then(function () {
    print('T1.then');
    P = Promise.reject(123);
    P.name = 'P';
    Q = Promise.reject(234);
    Q.name = 'Q';
    R = Promise.reject(345);
    R.name = 'R';
});
T1.name = 'T1';

var T2 = T.then(function () {
    print('T2.then');
    P.catch(nop);
    Q.catch(nop);

    // Handler is not callable.  R itself is considered handled because
    // it is directly bound to R2.  However, R2 will be unhandled.
    var R2 = R.catch(123)
    R2.name = 'R2';
});
T2.name = 'T2';

promiseNextTick(function (cb) {
    print('tick');
});
