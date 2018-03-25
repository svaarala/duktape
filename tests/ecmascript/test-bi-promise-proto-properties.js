/*---
{
    "skip": true
}
---*/

/*@include util-base.js@*/

/*===
object
propdesc then: value=function:then, writable=true, enumerable=false, configurable=true
propdesc catch: value=function:catch, writable=true, enumerable=false, configurable=true
propdesc constructor: value=function:Promise, writable=true, enumerable=false, configurable=true
propdesc Symbol(Symbol.toStringTag): value="Promise", writable=false, enumerable=false, configurable=true
true
done
===*/

var proto = Promise.prototype;
print(typeof proto);
print(Test.getPropDescString(proto, 'then'));
print(Test.getPropDescString(proto, 'catch'));
print(Test.getPropDescString(proto, 'constructor'));
print(Test.getPropDescString(proto, Symbol.toStringTag));
print(proto.constructor === Promise);

print('done');
