/*===
JSON
false false true
===*/

var pd;

print(JSON[Symbol.toStringTag]);
pd = Object.getOwnPropertyDescriptor(JSON, Symbol.toStringTag);
print(pd.writable, pd.enumerable, pd.configurable);
