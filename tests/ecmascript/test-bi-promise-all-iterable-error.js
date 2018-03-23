/*---
{
    "skip": true
}
---*/

/*===
done
all reject: Error: aiee
===*/

var iterable = {};
iterable[Symbol.iterator] = function () {
    throw new Error('aiee');
};

// Error from iteration must cause a reject for P rather than bubbling out.
try {
    var P = Promise.all(iterable);
} catch (e) {
    print(e.stack || e);
}
P.then(function (v) {
    print('all fulfill:', v);
}, function (e) {
    print('all reject:', String(e));
});

print('done');
