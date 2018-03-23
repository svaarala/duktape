/*---
{
    "skip": true
}
---*/

/*===
done
thenable called
reject: Error: aiee
===*/

var P = Promise.resolve({
    then: function (resolve, reject) {
        print('thenable called');
        reject(new Error('aiee'));
    }
});
P.then(function (v) {
    print('fulfill:', v);
}, function (e) {
    print('reject:', String(e));
});

print('done');
