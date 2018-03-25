/*---
{
    "skip": true
}
---*/

/*===
done
.then getter, throw
reject: 123
===*/

var P = Promise.resolve().then(function (v) {
    var ret = {};
    Object.defineProperty(ret, 'then', {
        get: function () {
            print('.then getter, throw');
            throw 123;
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
