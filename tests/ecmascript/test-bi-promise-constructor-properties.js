/*---
{
    "skip": true
}
---*/

/*@include util-base.js@*/

/*===
- general
function
1
Promise
true
true
propdesc Promise: value=function:Promise, writable=true, enumerable=false, configurable=true
true
- properties
propdesc length: value=1, writable=false, enumerable=false, configurable=true
propdesc name: value="Promise", writable=false, enumerable=false, configurable=true
propdesc prototype: value=[object Promise], writable=false, enumerable=false, configurable=false
propdesc all: value=function:all, writable=true, enumerable=false, configurable=true
propdesc race: value=function:race, writable=true, enumerable=false, configurable=true
propdesc reject: value=function:reject, writable=true, enumerable=false, configurable=true
propdesc resolve: value=function:resolve, writable=true, enumerable=false, configurable=true
done
===*/

// General.
var GLOBAL = new Function('return this')();

print('- general');
print(typeof Promise);
print(Promise.length);
print(Promise.name);
print('Promise' in GLOBAL);
print(Object.getPrototypeOf(Promise) === Function.prototype);
print(Test.getPropDescString(GLOBAL, 'Promise'));
print(Object.getPrototypeOf(new Promise(function () {})) === Promise.prototype);

print('- properties');
print(Test.getPropDescString(Promise, 'length'));
print(Test.getPropDescString(Promise, 'name'));
print(Test.getPropDescString(Promise, 'prototype'));
print(Test.getPropDescString(Promise, 'all'));
print(Test.getPropDescString(Promise, 'race'));
print(Test.getPropDescString(Promise, 'reject'));
print(Test.getPropDescString(Promise, 'resolve'));
// @@species separately

print('done');
