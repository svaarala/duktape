/*---
{
    "skip": true
}
---*/

/*===
done
then reject: 123
catch reject: 123
===*/

P = Promise.reject(123);
P.then(function (v) {
    print('then fulfill:', v);
}, function (e) {
    print('then reject:', e);
});
P.catch(function (e) {
    print('catch reject:', e);
});

print('done');
