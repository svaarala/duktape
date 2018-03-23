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

var P = new Promise(function (resolve_unused, reject) {
    print(typeof reject);

    // ES2015 says the function is anonymous.  In Node.js the name is 't',
    // in Firefox the name is '' and it is an own property.
    //
    // In Duktape the .name property is missing as own property and is
    // inherited as ''.
    print(reject.name);
    print(Object.getOwnPropertyNames(reject).indexOf('name') >= 0);
    //print(Test.getPropDescString(reject, 'name'));

    print(reject.length);
    print(Test.getPropDescString(reject, 'length'));
});

print('done');
