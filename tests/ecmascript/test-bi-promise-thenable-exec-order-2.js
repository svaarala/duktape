/*---
{
    "skip": true
}
---*/

/*===
create promise
executor called, resolve with thenable
call .then()
done
thenable.then() called
2 function function
reject: 123
===*/

// Thenable .then() is executed via the job queue.

print('create promise');
var P = new Promise(function (resolve, reject) {
    print('executor called, resolve with thenable');
    resolve({
        then: function (resolve, reject) {
            print('thenable.then() called');
            print(arguments.length, typeof resolve, typeof reject);
            reject(123);
        }
    });
});
print('call .then()');
P.then(function (v) {
    print('fulfill:', v);
}, function (e) {
    print('reject:', e);
});

print('done');
