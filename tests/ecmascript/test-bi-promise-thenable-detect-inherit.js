/*---
{
    "skip": true
}
---*/

/*===
done
Object.prototype.then
Object.prototype.then
===*/

// The .then() function can be inherited.  Here, setting it in Object.prototype
// makes every object value thenable.
var O1 = {};
var O2 = {};
Object.prototype.then = function () { print('Object.prototype.then'); };
Promise.resolve().then(function () { return O1; });
Promise.resolve().then(function () { return O2; });

print('done');
