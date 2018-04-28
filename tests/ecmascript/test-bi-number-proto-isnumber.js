// In ES5.1 Number.prototype is a Number itself:
//
// - https://ecma-international.org/ecma-262/5.1/#sec-15.7.4
//
// - "The Number prototype object is itself a Number object (its [[Class]]
//   is "Number") whose value is +0."
//
// In ES2015 it was changed to be a non-Number:
//
// - https://www.ecma-international.org/ecma-262/6.0/#sec-properties-of-the-number-prototype-object
//
// - "The Number prototype object is the intrinsic object %NumberPrototype%.
//   The Number prototype object is an ordinary object. It is not a Number
//   instance and does not have a [[NumberData]] internal slot."
//
// In ES2016 this was reverted and it's a Number again:
//
// - https://www.ecma-international.org/ecma-262/7.0/index.html#sec-number.prototype
//
// - "The Number prototype object is the intrinsic object %NumberPrototype%.
//   The Number prototype object is an ordinary object. The Number prototype
//   is itself a Number object; it has a [[NumberData]] internal slot with
//   the value +0."
//
// Test for the ES5.1/ES2016+ requirement.

/*===
123
123
0
done
===*/

print(Number.prototype.valueOf.call(new Number(123)));
print(Number.prototype.valueOf.call(123));
print(Number.prototype.valueOf.call(Number.prototype));
print('done');
