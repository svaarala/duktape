/*===
1 2
===*/

/* This broke at some point. */

try {
    for (var a=1, b=2; ; ) { print(a,b); break; }
} catch (e) {
    print(e);
}
