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

var resolveFn;

var P = new Promise(function (resolve, reject_unused) {
    print(typeof resolve);
    resolveFn = resolve;
});
P.then(function (val) {
    print('fulfill:', val);
}, function (err) {
    print('reject:', err);
});

try {
    var tmp = new resolveFn(123);
    print('constructor result:', typeof tmp);
} catch (e) {
    print(e.name);
}

print('done');
