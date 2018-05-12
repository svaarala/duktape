/*---
{
    "skip": true
}
---*/

/*@include util-base.js@*/

/*===
function
TypeError
done
===*/

var rejectFn;

var P = new Promise(function (resolve_unused, reject) {
    print(typeof reject);
    rejectFn = reject;
});
P.then(function (val) {
    print('fulfill:', val);
}, function (err) {
    print('reject:', err);
});

try {
    var tmp = new rejectFn(123);
    print('constructor result:', typeof tmp);
} catch (e) {
    print(e.name);
}

print('done');
