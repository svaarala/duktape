/*
 *  Exercise double-to-float out-of-range behavior which is technically
 *  undefined behavior in C99.
 */

/*===
3.4028234663852886e+38
3.4028234663852886e+38
Infinity
-3.4028234663852886e+38
-3.4028234663852886e+38
-Infinity
done
===*/

var f32 = new Float32Array(1);

/* Maximum float (as double). */
f32[0] = 340282346638528859811704183484516925440.0;
print(f32[0]);

/* Maximum double that rounds to maximum float (rather than infinity).
 * Without explicit handling for the out-of-range (out of 'float' range
 * here) case, -fsanitize=undefined will prompt:
 * duk_util_cast.c:139:11: runtime error: value 3.40282e+38 is outside the range of representable values of type 'float'
 * Same for all casts where the argument is beyond 'float' range.
 */
f32[0] = 340282356779733623858607532500980858880.0;
print(f32[0]);

/* First double that rounds to infinity. */
f32[0] = 340282356779733661637539395458142568448.0;
print(f32[0]);

/* And same as negative. */
f32[0] = -340282346638528859811704183484516925440.0;
print(f32[0]);
f32[0] = -340282356779733623858607532500980858880.0;
print(f32[0]);
f32[0] = -340282356779733661637539395458142568448.0;
print(f32[0]);

print('done');
