/*---
{
    "skip": true
}
---*/

/*===
done
.then getter
then() called
fulfill: 123
===*/

var P = Promise.resolve().then(function (v) {
    var ret = {};
    Object.defineProperty(ret, 'then', {
        get: function () {
            print('.then getter');
            return function (resolve, reject) {
                print('then() called');
                resolve(123);
            };
        },
        enumerable: false, configurable: true
    });
    return ret;
}).then(function (v) {
    print('fulfill:', v);
}, function (e) {
    print('reject:', e);
});

print('done');
