/*---
{
    "skip": true
}
---*/

/*===
done
reject: object
thenable object
===*/

// Thenable objects are not recognized by rejection.

var P = Promise.reject({
    then: function (resolve, reject) {
        print('thenable called');
        reject(new Error('aiee'));
    },
    who: 'thenable object'
});
P.then(function (v) {
    print('fulfill:', v);
}, function (e) {
    print('reject:', typeof e);
    print(e.who);
});

print('done');
