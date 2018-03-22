/*---
{
    "skip": true
}
---*/

/*===
- step 10
executor called
new Promise() returned
done
reject: Error: aiee
===*/

// Step 10:
// - fn throws, catch, call reject function
// - anything chaining on the promise gets called
print('- step 10');
try {
    var p = new Promise(function (resolve, reject) {
        print('executor called');
        throw new Error('aiee');
    });
    print('new Promise() returned');
    p.then(function (val) {
        print('fulfill:', val);
    }, function (err) {
        print('reject:', err);
    });
} catch (e) {
    print(e);
}

// Step 11:
// - return value is a Promise, already covered above.

print('done');
