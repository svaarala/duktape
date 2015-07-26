/*===
MAX_VALUE -> number 1.7976931348623157e+308
writable=false, enumerable=false, configurable=false
MIN_VALUE -> number 5e-324
writable=false, enumerable=false, configurable=false
NaN -> number NaN
writable=false, enumerable=false, configurable=false
POSITIVE_INFINITY -> number Infinity
writable=false, enumerable=false, configurable=false
NEGATIVE_INFINITY -> number -Infinity
writable=false, enumerable=false, configurable=false
===*/

function valueTest() {
   var names = [ 'MAX_VALUE', 'MIN_VALUE', 'NaN', 'POSITIVE_INFINITY', 'NEGATIVE_INFINITY' ];
   var i;
   var pd, v;

   for (i = 0; i < names.length; i++) {
       pd = Object.getOwnPropertyDescriptor(Number, names[i]);
       if (!pd) { print('does not exist'); continue; }
       v = pd.value;
       print(names[i], '->', typeof v, v);
       print('writable=' + pd.writable +
             ', enumerable=' + pd.enumerable +
             ', configurable=' + pd.configurable);
   }
}

try {
    valueTest();
} catch (e) {
    print(e);
}
