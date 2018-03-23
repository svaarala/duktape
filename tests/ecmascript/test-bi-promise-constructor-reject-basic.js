/*---
{
    "skip": true
}
---*/

/*===
done
reject: 123
===*/

var P = Promise.reject(123);
P.then(function (v) {
    print('fulfill:', v);
}, function (e) {
    print('reject:', e);
});

print('done');
