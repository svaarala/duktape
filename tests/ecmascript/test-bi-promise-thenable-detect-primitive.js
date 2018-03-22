/*---
{
    "skip": true
}
---*/

/*===
done
function
fulfill: number 123
===*/

// A primitive type is never 'thenable', even if its prototype includes
// .then().  Normally primitive values behave like corresponding objects
// but Promise 'thenable' is an exception.

Number.prototype.then = function (resolve, reject) {
    resolve('resolved-by-Number-prototype-then');
};

Promise.resolve().then(function (v) {
    var ret = 123;
    print(typeof ret.then);
    return ret;
}).then(function (v) {
    print('fulfill:', typeof v, v);
}, function (e) {
    print('reject:', e);
});

print('done');
