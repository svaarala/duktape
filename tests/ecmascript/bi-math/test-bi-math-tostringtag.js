/*===
Math
false false true
===*/

var pd;

print(Math[Symbol.toStringTag]);
pd = Object.getOwnPropertyDescriptor(Math, Symbol.toStringTag);
print(pd.writable, pd.enumerable, pd.configurable);
