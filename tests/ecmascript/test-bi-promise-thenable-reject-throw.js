/*---
{
    "skip": true
}
---*/

/*===
done
2 function function
reject: 123
===*/

// Error thrown by thenable.then() is mapped to rejection.

var P = Promise.resolve().then(function () {
    return {
        then: function (resolve, reject) {
            print(arguments.length, typeof resolve, typeof reject);
            throw 123;
        }
    };
});
P.then(function (v) {
    print('fulfill:', v);
}, function (e) {
    print('reject:', e);
});

print('done');
