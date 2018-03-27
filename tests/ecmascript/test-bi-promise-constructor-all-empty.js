/*---
{
    "skip": true
}
---*/

/*===
done
fulfill
object
[object Array]
0
===*/

// Empty list is an important corner case, and also exercises the specific
// .all() path where an immediate completion is checked for.

Promise.all([]).then(function (v) {
    print('fulfill');
    print(typeof v);
    print(Object.prototype.toString.call(v));
    print(v.length);
}, function (e) {
    print('reject:', e);
});

print('done');
