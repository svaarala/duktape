/*---
{
    "skip": true
}
---*/

/*===
done
2 function function
fulfill: 123
false
false
===*/

// Thenable, which first calls resolve() on the fresh resolve/reject pair.
// Calls to previous resolve/reject or calling the new resolve/reject again
// are no-op.  Also verify that a fresh resolve/reject function pair is
// created when the initial Promise is resolved with a thenable.

var resolve1, reject1;
var resolve2, reject2;
var P = new Promise(function (resolve, reject) {
    resolve1 = resolve;
    reject1 = reject;

    resolve1({
        then: function (resolve, reject) {
            print(arguments.length, typeof resolve, typeof reject);
            resolve2 = resolve;
            reject2 = reject;
            resolve2(123);
            reject2(new Error('aiee'));
            resolve2(321);
            throw 123;  // ignored because already resolved
        }
    });
    reject1(new Error('aiee'));
    resolve1(123);
});
P.then(function (v) {
    print('fulfill:', v);
}, function (e) {
    print('reject:', e);
}).then(function () {
    print(resolve1 === resolve2);
    print(reject1 === reject2);
});

print('done');
