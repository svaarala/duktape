/*===
REPLACEMENT
undefined
foo
bar
===*/

/* Object.defineProperties is required to call the original
 * Object.defineProperty() regardless of the current value
 * of Object.defineProperty (which is a configurable value).
 */

var orig_define_property;
var obj;

orig_define_property = Object.defineProperty;
Object.defineProperty = function() { print("REPLACEMENT"); }

obj = {};
Object.defineProperty(obj, 'foo', { value: 'bar' });
print(obj.foo);

Object.defineProperties(obj, { prop1: { value: 'foo' }, prop2: { value: 'bar' } });
print(obj.prop1);
print(obj.prop2);
