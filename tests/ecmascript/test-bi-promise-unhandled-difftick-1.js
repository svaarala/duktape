// Rejected promise with rejection handled after Promise settled during
// a different tick.  Test for both callable and non-callable handler.
//
// Handle rejection in a different tick which generates both reject and
// handle callbacks.

/*---
{
    "skip": true
}
---*/

/*@include util-promise.js@*/

/*===
tick 1
unhandled/reject: P
unhandled/reject: Q
unhandled/reject: R
tick 2
nop called
nop called
unhandled/handle: P
unhandled/handle: Q
unhandled/handle: R
unhandled/reject: R2
tick 3
tick 4
===*/

var P, Q, R;
var calls = [];

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

T.then(function () {
    print('tick 1');
    P = Promise.reject(123);
    P.name = 'P';
    Q = Promise.reject(234);
    Q.name = 'Q';
    R = Promise.reject(345);
    R.name = 'R';
});

promiseNextTick(function () {
    print('tick 2');
    P.catch(nop);
    Q.catch(nop);

    // Here R has already had its 'reject' event and it gets handled by
    // forwarding the rejection to R2.  So, R gets a 'handle' event, but
    // R2 is unhandled and gets a 'reject' event.
    var R2 = R.catch(123);
    R2.name = 'R2';

    promiseNextTick(function () {
        print('tick 3');
        promiseNextTick(function () {
            print('tick 4');
        });
    });
});
