// ES2015 guarantees that each Job Queue executes in strict FIFO order
// (but provides no guarantees of relative ordering between different
// Job Queues).
//
// For Promises, which all use the "PromiseJobs" queue, this means
// for example that for Promise.race() the first resolved argument
// Promise "wins", and its consequences get queued (but *not* executed)
// first.
//
// For Promises one important consideration is that Promise reactions
// are all enqueued when the Promise becomes settled.  Only after all
// the reactions have been queued will they be executed, potentially
// queueing further reactions which will always be queued after all the
// "first level" reactions get a chance to execute.
//
// This is why some of the seemingly innocent details in the specification
// are important; they may have observable consequences on the reaction
// ordering guaranteed by ES2015.

/*---
{
    "skip": true
}
---*/

/*===
P1 executor
P2 executor
P3 executor
create Promise.race()
resolve 3
resolve 2
reject 1
done
P3.then fulfill 1: 234
P3.then fulfill 2: 234
P3.then fulfill 3: 234
P2.then fulfill 1: 123
P2.then fulfill 2: 123
P2.then fulfill 3: 123
P1.then reject 1: Error: aiee
P1.then reject 2: Error: aiee
P1.then reject 3: Error: aiee
PRACE fulfill 1: 234
===*/

var resolve1, resolve1;
var resolve2, resolve2;
var resolve3, resolve3;
var P1 = new Promise(function (resolve, reject) {
    print('P1 executor');
    resolve1 = resolve; reject1 = reject;
});
var P2 = new Promise(function (resolve, reject) {
    print('P2 executor');
    resolve2 = resolve; reject2 = reject;
});
var P3 = new Promise(function (resolve, reject) {
    print('P3 executor');
    resolve3 = resolve; reject3 = reject;
});

P1.then(function (v) {
    print('P1.then fulfill 1:', v);
}, function (e) {
    print('P1.then reject 1:', String(e));
});
P2.then(function (v) {
    print('P2.then fulfill 1:', v);
}, function (e) {
    print('P2.then reject 1:', String(e));
});
P3.then(function (v) {
    print('P3.then fulfill 1:', v);
}, function (e) {
    print('P3.then reject 1:', String(e));
});

print('create Promise.race()');
var PRACE = Promise.race([ P1, P2, P3 ]);
PRACE.then(function (v) {
    print('PRACE fulfill 1:', v);
}, function (e) {
    print('PRACE reject 1:', String(e));
});

P1.then(function (v) {
    print('P1.then fulfill 2:', v);
}, function (e) {
    print('P1.then reject 2:', String(e));
});
P2.then(function (v) {
    print('P2.then fulfill 2:', v);
}, function (e) {
    print('P2.then reject 2:', String(e));
});
P3.then(function (v) {
    print('P3.then fulfill 2:', v);
}, function (e) {
    print('P3.then reject 2:', String(e));
});

print('resolve 3');
resolve3(234);
P3.then(function (v) {
    print('P3.then fulfill 3:', v);
}, function (e) {
    print('P3.then reject 3:', String(e));
});

print('resolve 2');
resolve2(123);
P2.then(function (v) {
    print('P2.then fulfill 3:', v);
}, function (e) {
    print('P2.then reject 3:', String(e));
});

print('reject 1');
reject1(new Error('aiee'));
P1.then(function (v) {
    print('P1.then fulfill 3:', v);
}, function (e) {
    print('P1.then reject 3:', String(e));
});

print('done');
