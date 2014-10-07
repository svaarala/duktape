var obj = { foo:1, bar:2, quux: [ 1, 2 ], emptyObj: {}, emptyArr: [] };

/*===
{"foo":1,"bar":2,"quux":[1,2],"emptyObj":{},"emptyArr":[]}
===*/

/* Baseline */

print(JSON.stringify(obj));

/*===
{"foo":1,"bar":2,"quux":[1,2],"emptyObj":{},"emptyArr":[]}
{
  "foo": 1,
  "bar": 2,
  "quux": [
    1,
    2
  ],
  "emptyObj": {},
  "emptyArr": []
}
{
          "foo": 1,
          "bar": 2,
          "quux": [
                    1,
                    2
          ],
          "emptyObj": {},
          "emptyArr": []
}
{
          "foo": 1,
          "bar": 2,
          "quux": [
                    1,
                    2
          ],
          "emptyObj": {},
          "emptyArr": []
}
===*/

/* String up to 10 chars is accepted.  If the string is given but is empty,
 * stringify() must behave as if no string was given (= compact encoding).
 *
 * Note that empty arrays and objects are required to appear in a compact
 * form, as "[]" and "{}", respectively.
 */

print(JSON.stringify(obj, null, ''));
print(JSON.stringify(obj, null, '  '));
print(JSON.stringify(obj, null, '          '));
print(JSON.stringify(obj, null, '          clipped'));

/*===
{
foo"foo": 1,
foo"bar": 2,
foo"quux": [
foofoo1,
foofoo2
foo],
foo"emptyObj": {},
foo"emptyArr": []
}
===*/

/* String may also contain arbitrary characters, producing invalid JSON. */

print(JSON.stringify(obj, null, 'foo'));

/*===
{"foo":1,"bar":2,"quux":[1,2],"emptyObj":{},"emptyArr":[]}
{"foo":1,"bar":2,"quux":[1,2],"emptyObj":{},"emptyArr":[]}
{"foo":1,"bar":2,"quux":[1,2],"emptyObj":{},"emptyArr":[]}
{
 "foo": 1,
 "bar": 2,
 "quux": [
  1,
  2
 ],
 "emptyObj": {},
 "emptyArr": []
}
{
  "foo": 1,
  "bar": 2,
  "quux": [
    1,
    2
  ],
  "emptyObj": {},
  "emptyArr": []
}
{
    "foo": 1,
    "bar": 2,
    "quux": [
        1,
        2
    ],
    "emptyObj": {},
    "emptyArr": []
}
{
    "foo": 1,
    "bar": 2,
    "quux": [
        1,
        2
    ],
    "emptyObj": {},
    "emptyArr": []
}
{
          "foo": 1,
          "bar": 2,
          "quux": [
                    1,
                    2
          ],
          "emptyObj": {},
          "emptyArr": []
}
{
          "foo": 1,
          "bar": 2,
          "quux": [
                    1,
                    2
          ],
          "emptyObj": {},
          "emptyArr": []
}
{
          "foo": 1,
          "bar": 2,
          "quux": [
                    1,
                    2
          ],
          "emptyObj": {},
          "emptyArr": []
}
{"foo":1,"bar":2,"quux":[1,2],"emptyObj":{},"emptyArr":[]}
===*/

/* Space argument can also be a number */

print(JSON.stringify(obj, null, Number.NEGATIVE_INFINITY));
print(JSON.stringify(obj, null, -1));
print(JSON.stringify(obj, null, 0));
print(JSON.stringify(obj, null, 1));
print(JSON.stringify(obj, null, 2));
print(JSON.stringify(obj, null, 4.1));
print(JSON.stringify(obj, null, 4.9));
print(JSON.stringify(obj, null, 10));
print(JSON.stringify(obj, null, 11));
print(JSON.stringify(obj, null, Number.POSITIVE_INFINITY));
print(JSON.stringify(obj, null, Number.NaN));

/*===
===*/

/* XXX: coercion of String and Number objects */
