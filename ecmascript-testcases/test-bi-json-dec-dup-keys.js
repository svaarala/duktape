/*---
{
    "nonstandard": true
}
---*/

/*===
{"foo":3,"bar":2}
===*/

/* If a key appears multiple times, the last value wins.
 *
 * What makes this test a non-standard one is that we also test that the
 * enumeration order is as expected, i.e. the second occurrence does
 * not change the enum order (first occurrence defines order).
 */

try {
    print(JSON.stringify(JSON.parse('{"foo":1,"bar":2,"foo":3}')));
} catch (e) {
    print(e.name);
}
