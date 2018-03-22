/*---
{
    "skip": true
}
---*/

/*===
done
2 function function
fulfill: 123
===*/

// Basic thenable: Promise is resolved with a plain object with a callable
// .then value.

var P = Promise.resolve().then(function () {
    return {
        then: function (resolve, reject) {
            print(arguments.length, typeof resolve, typeof reject);
            resolve(123);
        }
    };
});
P.then(function (v) {
    print('fulfill:', v);
}, function (e) {
    print('reject:', e);
});

print('done');
