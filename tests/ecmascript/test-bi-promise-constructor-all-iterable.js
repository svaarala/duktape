/*---
{
    "skip": true
}
---*/

/*===
next called, done: false
next called, done: false
next called, done: false
next called, done: true
done
all fulfill: foo,bar,quux
===*/

var iterable = {};
var index = 0;
var values = [
    // Plain values, Promise, a Promise returning a generic thenable.
    'foo',
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

var P = Promise.all(iterable);
P.then(function (v) {
    print('all fulfill:', String(v));
}, function (e) {
    print('all reject:', String(e));
});

print('done');
