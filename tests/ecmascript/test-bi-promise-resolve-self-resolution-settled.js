/*---
{
    "skip": true
}
---*/

/*===
ignored
done
fulfill: 123
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

resolveFn(123);

// Self resolution after already being settled is a no-op.
try {
    resolveFn(P);
    print('ignored');
} catch (e) {
    print('should not happen:', e);
}

print('done');
