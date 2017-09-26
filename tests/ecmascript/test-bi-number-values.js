/*===
propdesc MAX_VALUE: value=1.7976931348623157e+308, writable=false, enumerable=false, configurable=false
propdesc MIN_VALUE: value=5e-324, writable=false, enumerable=false, configurable=false
propdesc NaN: value=NaN, writable=false, enumerable=false, configurable=false
propdesc POSITIVE_INFINITY: value=Infinity, writable=false, enumerable=false, configurable=false
propdesc NEGATIVE_INFINITY: value=-Infinity, writable=false, enumerable=false, configurable=false
propdesc EPSILON: value=2.220446049250313e-16, writable=false, enumerable=false, configurable=false
propdesc MAX_SAFE_INTEGER: value=9007199254740991, writable=false, enumerable=false, configurable=false
propdesc MIN_SAFE_INTEGER: value=-9007199254740991, writable=false, enumerable=false, configurable=false
===*/

/*@include util-base.js@*/

function valueTest() {
   var names = [
       'MAX_VALUE', 'MIN_VALUE', 'NaN', 'POSITIVE_INFINITY', 'NEGATIVE_INFINITY',
       'EPSILON', 'MAX_SAFE_INTEGER', 'MIN_SAFE_INTEGER'
   ];
   var i;

   for (i = 0; i < names.length; i++) {
       print(Test.getPropDescString(Number, names[i]));
   }
}

try {
    valueTest();
} catch (e) {
    print(e);
}
