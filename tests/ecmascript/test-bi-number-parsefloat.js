/*===
function
true
object
true
false
true
done
===*/

print(typeof Number.parseFloat);
print(Number.parseFloat === parseFloat);
var pd = Object.getOwnPropertyDescriptor(Number, 'parseFloat');
print(typeof pd);
print(pd.writable);
print(pd.enumerable);
print(pd.configurable);
print('done');
