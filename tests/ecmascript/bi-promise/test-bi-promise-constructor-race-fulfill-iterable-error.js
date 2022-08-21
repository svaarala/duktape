/*---
skip: true
duktape_polyfills:
  promise: true
---*/

/*===
done
race reject: Error: aiee
===*/

var iterable = {};
iterable[Symbol.iterator] = function () {
    throw new Error('aiee');
};

// Error from iteration must cause a reject for P rather than bubbling out.
var P = Promise.race(iterable);
P.then(function (v) {
    print('race fulfill:', v);
}, function (e) {
    print('race reject:', String(e));
});

print('done');
