/*===
symbol
object
symbol
true
===*/

var symbol = Symbol('foo');
print(typeof symbol);
var object = Object(symbol);
print(typeof object);
var value = object.valueOf();
print(typeof value);
print(symbol === value);
