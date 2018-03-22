/*---
{
    "skip": true
}
---*/

/*===
done
2 function function
2 function function
fulfill: 123
false
false
false
false
===*/

// Two-level thenable, resolve/reject reuse checks.
var resolve1, reject1;
var resolve2, reject2;
var resolve3, reject3;
var P = new Promise(function (resolve, reject) {
    resolve1 = resolve;
    reject1 = reject;

    resolve1({
        then: function (resolve, reject) {
            print(arguments.length, typeof resolve, typeof reject);
            resolve2 = resolve;
            reject2 = reject;
            resolve2({
                then: function (resolve, reject) {
                    print(arguments.length, typeof resolve, typeof reject);
                    resolve3 = resolve;
                    reject3 = reject;
                    resolve3(123);
                    reject3(new Error('aiee'));
                    resolve3(321);
                    throw 123;  // ignored because already resolved
                }
            });
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
    print(resolve2 === resolve3);
    print(reject2 === reject3);
});

print('done');
