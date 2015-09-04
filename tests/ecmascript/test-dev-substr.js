var str;
var t;

/*===
oob
===*/

/* Basic ASCII case */

str = "foobar";
print(str.substring(1,4));

/*===
oob
oobar
oobar
f
foo
oo
foobar
foobar
foobar
foobar

===*/

/* Offset argument coercions etc */

str = "foobar";

print(str.substring('1', '4'));        // coerced with ToInteger
print(str.substring('1'));             // missing argument -> default to end

print(str.substring('1', undefined));  // undefined argument -> default to end
print(str.substring('1', null));       // null -> coerce with ToInteger(null) -> 0; substr(1,0) -> substr(0,1)
print(str.substring('3', false));      // true -> coerce with ToInteger(true) -> 0
print(str.substring('3', true));       // true -> coerce with ToInteger(true) -> 1

print(str.substring(-10, 10000));      // clamping
print(str.substring(Number.NEGATIVE_INFINITY, Number.POSITIVE_INFINITY));  // still a number
print(str.substring(10000, -10));      // clamping, swapping
print(str.substring(Number.POSITIVE_INFINITY, Number.NEGATIVE_INFINITY));  // still a number

print(str.substring(3, 3));  // zero length

/*===
foo
bar
16384 65535
===*/

/* Basic testing of Unicode strings.  In the implementation, coercing and
 * clamping works on character offsets which are converted to byte offsets
 * very late in the process.  So all the cases tested above don't need to
 * be replicated here -- we just need to test that the character offset to
 * byte offset conversion works.
 */

str = 'foo\u0080\u4000\uFFFFbar';

print(str.substring(0, 3));
print(str.substring(6, 9));
t = str.substring(4, 6);  // U+4000 U+FFFF
print(t.charCodeAt(0), t.charCodeAt(1));

/* XXX: more tests */
