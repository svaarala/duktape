/*===
foo bar
foo bar undefined
foo

foo
foo
foo
===*/

/* The apply 'argArray' does NOT have to be an array; its "length"
 * property is coerced with ToUint32() and properties are then
 * looped.
 */

/* Note: Rhino will complain the following:
 *
 * js: uncaught JavaScript runtime exception: TypeError: second argument to Function.prototype.apply must be an array
 *
 * This seems incorrect (for E5), Section 15.3.4.3 step 3 only checks
 * whether the argument is an object.
 */

print.apply(null, { "0": "foo", "1": "bar", "length": 2 });
print.apply(null, { "0": "foo", "1": "bar", "length": 3 });
print.apply(null, { "0": "foo", "1": "bar", "length": 1 });
print.apply(null, { "0": "foo", "1": "bar", "length": 4294967296 });  /* ToUint32(len) -> 0 */
print.apply(null, { "0": "foo", "1": "bar", "length": 4294967297 });  /* ToUint32(len) -> 1 */
print.apply(null, { "0": "foo", "1": "bar", "length": "1" });  /* ToUint32("1") -> 1 */
print.apply(null, { "0": "foo", "1": "bar", "length": true });  /* ToUint32(true) -> 1 */
