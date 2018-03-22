/*---
{
    "skip": true
}
---*/

/*===
done
2 function function
2 function function
false
false
reject: Error: aiee
false
false
false
false
false
false
===*/

// Two level thenable with rejection, with neutralization checks.

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

                    print(resolve2 === resolve3);
                    print(reject2 === reject3);

                    resolve1('aiee');  // neutralized already
                    resolve2('aiee');  // neutralized already

                    reject3(new Error('aiee'));
                    resolve3(321);
                    reject3(new Error('barf'));
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
    print('reject:', String(e));
}).then(function () {
    print(resolve1 === resolve2);
    print(reject1 === reject2)
    print(resolve1 === resolve3);
    print(reject1 === reject3);
    print(resolve2 === resolve3);
    print(reject2 === reject3);
});

print('done');
