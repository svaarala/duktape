/*===
10 20
-> 10 21
-> 11 21
===*/

/* Broken at some point. */

var x = 10;
var y = 20;

function f_postinc() {
    print(x, y);

    // interpreted as "x; ++y;"
    x
    ++y

    print('->', x, y);

    // interpreted as "x++; y;"
    x++
    y

    print('->', x, y);
}

try {
    f_postinc();
} catch (e) {
    print(e.name);
}
