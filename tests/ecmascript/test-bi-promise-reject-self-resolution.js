/*---
{
    "skip": true
}
---*/

/*===
done
reject: I'm the Promise
===*/

var rejectFn;
var P = new Promise(function (resolve, reject) {
    rejectFn = reject;
});
P.who = "I'm the Promise";

P.then(function (v) {
    print('fulfill:', v);
}, function (e) {
    //print('reject:', e);
    print('reject:', e.who);
});

// Reject allows self-resolution to Promise.
try {
    rejectFn(P);
} catch (e) {
    print('should not happen:', e);
}

print('done');
