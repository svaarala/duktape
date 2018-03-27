/*---
{
    "skip": true
}
---*/

/*===
next called, done: false
next called, done: false
next called, done: false
next called, done: false
next called, done: true
done
race fulfill: foo
===*/

var iterable = {};
var index = 0;
var values = [
    // Plain values, Promise, a Promise returning a generic thenable.
    new Promise(function () {}),  // unresolved Promise
    'foo',  // first settled value -> becomes result
    Promise.resolve('bar'),
    Promise.resolve('dummy').then(function () {
        return {
            then: function (resolve, reject) {
                resolve('quux');
            }
        }
    })
];

iterable[Symbol.iterator] = function () {
    return {
        next: function () {
            var done = index >= values.length;
            var value = values[index++];
            print('next called, done:', done);
            return { value: value, done: done };
        }
    };
};

var P = Promise.race(iterable);
P.then(function (v) {
    print('race fulfill:', String(v));
}, function (e) {
    print('race reject:', String(e));
});

print('done');
