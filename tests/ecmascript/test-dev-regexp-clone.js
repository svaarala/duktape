/*===
false 10 0
true 10 10
===*/

/* "new RegExp(re)" where 're' is a RegExp does NOT return the object
 * as is, but instead creates a "clone" with the same pattern and
 * flags.  lastIndex is initialized to zero.
 *
 * "RegExp(re)" does return the same object.
 */

try {
    var re1 = /foo/g;
    var re2;

    re1.lastIndex = 10;
    re2 = new RegExp(re1);
    print(re1 === re2, re1.lastIndex, re2.lastIndex);

    re1.lastIndex = 10;
    re2 = RegExp(re1);
    print(re1 === re2, re1.lastIndex, re2.lastIndex);
} catch (e) {
    print(e);
}
