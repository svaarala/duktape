/*---
{
    "skip": true
}
---*/

/*@include util-base.js@*/

/*===
function

false
1
propdesc length: value=1, writable=false, enumerable=false, configurable=true
done
===*/

var P = new Promise(function (resolve, reject_unused) {
    print(typeof resolve);

    // ES2015 says the function is anonymous.  In Node.js the name is 's',
    // in Firefox the name is '' and it is an own property.
    //
    // In Duktape the .name property is missing as own property and is
    // inherited as ''.
    print(resolve.name);
    print(Object.getOwnPropertyNames(resolve).indexOf('name') >= 0);
    //print(Test.getPropDescString(resolve 'name'));

    print(resolve.length);
    print(Test.getPropDescString(resolve, 'length'));
});

print('done');
