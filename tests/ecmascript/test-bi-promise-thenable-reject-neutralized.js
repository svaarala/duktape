/*---
{
    "skip": true
}
---*/

/*===
done
2 function function
reject: Error: aiee
false
false
===*/

// Thenable is rejected, check that other resolve/reject calls are no-op.

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
            reject2(new Error('aiee'));
            resolve2(321);  // nop
            reject2(new Error('barf'));  // nop
            throw 123;  // ignored because already resolved
        }
    });
    reject1(new Error('aiee'));  // nop
    resolve1(123);  // nop
});
P.then(function (v) {
    print('fulfill:', v);
}, function (e) {
    print('reject:', String(e));
}).then(function () {
    print(resolve1 === resolve2);
    print(reject1 === reject2);
});

print('done');
