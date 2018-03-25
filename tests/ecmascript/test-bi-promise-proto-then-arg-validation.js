// .then() arguments are checked for callability.  If they are not callable,
// they will behave the same as a missing value with no TypeError.
// https://www.ecma-international.org/ecma-262/6.0/#sec-performpromisethen

/*---
{
    "skip": true
}
---*/

/*===
done
P fulfill 1 123
P fulfill 2 123
P fulfill 3 123
P fulfill 4 123
P fulfill 5 123
P fulfill 6 123
P fulfill 7 123
P fulfill 8 123
P fulfill 9 123
P fulfill 10 123
P fulfill 11 123
P fulfill 12 123
P fulfill 13 123
P fulfill 14 123
P fulfill 15 123
P fulfill 16 123
P fulfill 17 123
P fulfill 18 123
P fulfill 19 123
P fulfill 20 123
P fulfill 21 123
P fulfill 22 123
P fulfill 23 123
P fulfill 24 123
P fulfill 25 123
P fulfill 26 123
Q reject 1 321
Q reject 2 321
Q reject 3 321
Q reject 4 321
Q reject 5 321
Q reject 6 321
Q reject 7 321
Q reject 8 321
Q reject 9 321
Q reject 10 321
Q reject 11 321
Q reject 12 321
Q reject 13 321
Q reject 14 321
Q reject 15 321
Q reject 16 321
Q reject 17 321
Q reject 18 321
Q reject 19 321
Q reject 20 321
Q reject 21 321
Q reject 22 321
Q reject 23 321
Q reject 24 321
Q reject 25 321
Q reject 26 321
===*/

var resolveP, rejectP;
var P = new Promise(function (resolve, reject) {
    resolveP = resolve;
    rejectP = reject;
});

var resolveQ, rejectQ;
var Q = new Promise(function (resolve, reject) {
    resolveQ = resolve;
    rejectQ = reject;
});

var values = [
    void 0, null, true, false, -1/0, -123, -0, 0, 123, 1/0, 0/0,
    { foo: 'bar' }, [ 1, 2, 3 ]
];

var counter = 0;
values.forEach(function (v) {
    var idx = ++counter;
    var prom = P.then(v).then(function (v) {
        print('P fulfill', idx, v);
    }, function (e) {
        print('P reject', idx, e);
    });
    var prom = Q.then(v).then(function (v) {
        print('Q fulfill', idx, v);
    }, function (e) {
        print('Q reject', idx, e);
    });
});

values.forEach(function (v) {
    var idx = ++counter;
    var prom = P.then(void 0, v).then(function (v) {
        print('P fulfill', idx, v);
    }, function (e) {
        print('P reject', idx, e);
    });
    var prom = Q.then(void 0, v).then(function (v) {
        print('Q fulfill', idx, v);
    }, function (e) {
        print('Q reject', idx, e);
    });
});

resolveP(123);
rejectQ(321);

print('done');
