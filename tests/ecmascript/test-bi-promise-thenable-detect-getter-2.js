/*---
{
    "skip": true
}
---*/

/*===
done
.then getter
fulfill: object ret
===*/

var P = Promise.resolve().then(function (v) {
    var ret = { who: 'ret' };
    Object.defineProperty(ret, 'then', {
        get: function () {
            print('.then getter');
            return 'non-callable';
        },
        enumerable: false, configurable: true
    });
    return ret;
}).then(function (v) {
    print('fulfill:', typeof v, v.who);
}, function (e) {
    print('reject:', e);
});

print('done');
