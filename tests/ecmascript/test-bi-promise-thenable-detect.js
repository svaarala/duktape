/*---
{
    "skip": true
}
---*/

/*===
done
O1.then
===*/

// Thenable objects are detected purely by duck typing: if the resolve value is
// an object with a callable .then(), it's treated as a thenable.  The .then()
// property may be own or inherited.

var O1 = { then: function () { print('O1.then'); } };
var O2 = { them: function () { print('O1.them'); } };
var O3 = { then: 123 };
Promise.resolve().then(function () { return O1; });
Promise.resolve().then(function () { return O2; });
Promise.resolve().then(function () { return O3; });

print('done');
