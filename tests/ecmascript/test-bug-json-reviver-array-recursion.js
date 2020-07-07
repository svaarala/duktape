/*
 *  https://github.com/svaarala/duktape/issues/2338
 */

/*===
RangeError
done
===*/

function b() {
    a = Array(3);
    this[2] = a;
}
try {
    JSON.parse("[1, 2, []]", b);
} catch (e) {
    print(e.name);
}
print('done');
