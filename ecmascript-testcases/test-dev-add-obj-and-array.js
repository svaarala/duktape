/*
 *  A random test based on a Slashdot comment :-)
 */

var res;

/*===
0 number
===*/

/* {} + [] parses as an empty block statement (!) followed by unary plus
 * and an array, and is equivalent to:
 *
 *   {};
 *   +[];
 *
 * where +[] evaluates to ToNumber([]):
 *
 *   [] -> ToPrimitive([], hint Number)
 *      -> [].[[DefaultValue]] (hint = Number)
 *      -> Object.prototype.valueOf() -> returns [] itself, not primitive
 *      -> Array.prototype.toString()
 *      -> ""
 *      -> 0
 *
 * The coercion of an empty string or a string consisting only of
 * whitespace is 0 and not a NaN (!); see E5 Section 9.3.1.
 */

res = eval('{}+[]');
print(res, typeof res);

/*===
[object Object] string
[object Object] string
===*/

/* [] + {} parses as a binary addition.
 *
 * Each side is first coerced using ToPrimitive():
 *
 *   ToPrimitive({})
 *     -> {}.[[DefaultValue]] (no hint -> Number)
 *     -> Object.prototype.valueOf() -> returns [] itself, not pritimive
 *     -> Object.prototype.toString()
 *     -> "[object Object]"
 *
 *   ToPrimitive([])
 *     -> [].[[DefaultValue]] (no hint -> Number)
 *     -> Object.prototype.valueOf() -> returns {} itself, not primitive
 *     -> Array.prototype.toString()
 *     -> ""
 *
 * So, [] + {} should yield "" + "[object Object]" == "[object Object].
 *
 * Same goes for ({} + []) which is a binary addition expression because
 * of the parens.
 */

res = eval('[]+{}');
print(res, typeof res);

res = eval('({}+[])');
print(res, typeof res);
