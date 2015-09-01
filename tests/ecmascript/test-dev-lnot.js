/* XXX: missing a lot of coercion tests */

/*===
true
false
true
false
true
false
===*/

print(true)
print(!true);
print(!!true);
print(false);
print(!false);
print(!!false);

/*===
false
true
true
false
false
true
===*/

print(!{});   // ToBoolean({}) -> true
print(!!{});
print(!'');   // ToBoolean('') -> false
print(!!'');
print(!'a');  // ToBoolean('a') -> true
print(!!'a');
