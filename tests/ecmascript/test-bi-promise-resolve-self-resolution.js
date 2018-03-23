/*---
{
    "skip": true
}
---*/

/*===
done
reject: TypeError
===*/

var resolveFn;
var P = new Promise(function (resolve, reject) {
    resolveFn = resolve;
});

P.then(function (v) {
    print('fulfill:', v);
}, function (e) {
    print('reject:', e.name);
});

try {
    resolveFn(P);
} catch (e) {
    print('should not happen:', e);
}

print('done');
