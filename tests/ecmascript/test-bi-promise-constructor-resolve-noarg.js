/*---
{
    "skip": true
}
---*/

/*===
done
fulfill, values match true
2 undefined undefined
===*/

// Missing argument and void 0 are handled the same.

var P = Promise.resolve();
var Q = Promise.resolve(void 0);

Promise.all([ P, Q ]).then(function (v) {
    print('fulfill, values match', v[0] === v[1]);
    print(v.length, v[0], v[1]);
}, function (e) {
    print('reject:', e);
});

print('done');
