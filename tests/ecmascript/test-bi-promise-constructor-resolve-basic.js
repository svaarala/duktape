/*---
{
    "skip": true
}
---*/

/*===
done
fulfill: 123
===*/

var P = Promise.resolve(123);
P.then(function (v) {
    print('fulfill:', v);
}, function (e) {
    print('reject:', String(e));
});

print('done');
