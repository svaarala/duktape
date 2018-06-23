// For tick N, the list of potentially unhandled rejections cause a callback
// for each such Promise.  If new unhandled rejections are created by those
// callbacks, and are not handled by the end of tick N, they also require
// callbacks.
//
// Also, if a promise is unhandled when the unhandled rejections checking
// begins, one unhandled rejection callback may handle it before we get to
// it.  In this case no callback is desired.

// P1: base case, unhandled rejection, gets callback.
// P2: unhandled rejection, but callback for P1 handles it.
// P3: base case, unhandled rejection, gets callback (needed for other tests).
// P4: created by P1 unhandled rejection, remains unhandled to tick 2; handled in tick 2
// P5: created by P1 unhandled rejection, handled by P3 unhandled rejection

/*---
{
    "skip": true
}
---*/

/*@include util-promise.js@*/

/*===
TICK 1
unhandled/reject: P1
unhandled/reject: P3
unhandled/reject: P4
P2.catch: 234
P5.catch: 567
TICK 2
P4.catch: 456
unhandled/handle: P4
TICK 3
done
===*/

var P1, P2, P3, P4, P5;

function unhandledRejection(promise) {
    print('unhandled/reject:', promise.name);
    if (promise === P1) {
        P2.catch(function (e) {
            print('P2.catch:', e);
        });
        P4 = Promise.reject(456);
        P4.name = 'P4';
        P5 = Promise.reject(567);
        P5.name = 'P5';
    } else if (promise === P3) {
        P5.catch(function (e) {
            print('P5.catch:', e);
        });
    }
}
function rejectionHandled(promise) {
    print('unhandled/handle:', promise.name);
}

setupPromiseUnhandledCallbacks(unhandledRejection, rejectionHandled);

function tick1() {
    print('TICK 1');
    P1 = Promise.reject(123);
    P1.name = 'P1';
    P2 = Promise.reject(234);
    P2.name = 'P2';
    P3 = Promise.reject(345);
    P3.name = 'P3';
    promiseNextTick(tick2);
}

function tick2() {
    print('TICK 2');
    P4.catch(function (e) {
        print('P4.catch:', e);
    });
    promiseNextTick(tick3);
}

function tick3() {
    print('TICK 3');
    print('done');
}

tick1();
