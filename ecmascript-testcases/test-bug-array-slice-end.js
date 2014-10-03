/*===
3,4,5
3,4,5
===*/

/* Broken at some point: end would not default to 'length' */

try {
    print([1,2,3,4,5].slice(2));
    print([1,2,3,4,5].slice(2, 5));
} catch (e) {
    print(e);
}
