/*---
{
    "skip": true
}
---*/

/*===
call Promise.try()
argument called
call returned
done
.then() called
===*/

print('call Promise.try()');
var P = Promise.try(function () {
    print('argument called');
});
P.then(function () {
    print('.then() called');
});
print('call returned');
print('done');
