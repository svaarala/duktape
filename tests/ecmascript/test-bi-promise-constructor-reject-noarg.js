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

var P = Promise.reject();
var Q = Promise.reject(void 0);

var T1 = P.catch(function (e) {
    return e;
});
var T2 = Q.catch(function (e) {
    return e;
});

Promise.all([ T1, T2 ]).then(function (v) {
    print('fulfill, values match', v[0] === v[1]);
    print(v.length, v[0], v[1]);
}, function (e) {
    print('reject:', e);
});

print('done');
