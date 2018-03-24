/*---
{
    "skip": true
}
---*/

/*===
done
fulfill: 123
===*/

var P = Promise.try(function () {
    return 123;
});
P.then(function (v) {
    print('fulfill:', v);
}, function (e) {
    print('reject:', e);
});
print('done');
