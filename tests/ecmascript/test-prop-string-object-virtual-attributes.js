/*
 *  A String object has a special [[GetOwnProperty]] implementation which special
 *  cases array indexes in the range [0, strlen-1], and gives them the attributes:
 *
 *   - [[Enumerable]]: true
 *   - [[Writable]]: false
 *   - [[Configurable]]: false
 *
 */

/*===
0 string
1 string
2 string
===*/

var a = "foo";   /* no need to create explicit object, ToObject coercion suffices */
var i;

try {
    for (i in a) {
        /* since keys are enumerable, all indices should appear here */
        print(i, typeof i);
    }
} catch (e) {
    print(e.name);
}

/*===
o
z
3
===*/
a = new String("foo");
a[2] = 'z';  /* virtual property is not Writable, ignored */
print(a[2]);
a[3] = 'z';  /* virtual property does not exist, so does NOT prevent a write */
print(a[3]);
print(a.length);  /* length is not affected because strings don't have a magic 'length' */

/* XXX: check property descriptors */
/* XXX: add more tests */
