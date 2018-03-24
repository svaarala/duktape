// https://github.com/tc39/proposal-promise-try

/*---
{
    "skip": true
}
---*/

/*@include util-base.js@*/

/*===
function
try
1
propdesc try: value=function:try, writable=true, enumerable=false, configurable=true
done
===*/

print(typeof Promise.try);
print(Promise.try.name);
print(Promise.try.length);
print(Test.getPropDescString(Promise, 'try'));

print('done');
