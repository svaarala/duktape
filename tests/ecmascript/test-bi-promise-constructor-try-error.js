/*---
{
    "skip": true
}
---*/

/*===
done
reject: RangeError: aiee
===*/

var P = Promise.try(function () {
    throw new RangeError('aiee');
});
P.then(function (v) {
    print('fulfill:', v);
}, function (e) {
    print('reject:', String(e));
});
print('done');
