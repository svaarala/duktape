/*---
{
    "skip": true
}
---*/

/*===
create promise
call .then()
done
return thenable
thenable.then() called
2 function function
reject: 123
===*/

// Thenable .then() is executed via the job queue.

print('create promise');
var P = Promise.resolve().then(function () {
    print('return thenable');
    return {
        then: function (resolve, reject) {
            print('thenable.then() called');
            print(arguments.length, typeof resolve, typeof reject);
            reject(123);
        }
    };
});
print('call .then()');
P.then(function (v) {
    print('fulfill:', v);
}, function (e) {
    print('reject:', e);
});

print('done');
