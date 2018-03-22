/*---
{
    "skip": true
}
---*/

/*===
- resolve() may be given a Promise
done
fulfill: 123
===*/

print('- resolve() may be given a Promise');
try {
    var otherResolve, otherReject;
    var other = new Promise(function (resolve, reject) {
        otherResolve = resolve;
        otherReject = reject;
    });
    var p = new Promise(function (resolve, reject) {
        resolve(other);
    });
    p.then(function (val) {
        print('fulfill:', val);
    }, function (err) {
        print('reject:', err);
    });
    otherResolve(123);
} catch (e) {
    print(e);
}

print('done');
