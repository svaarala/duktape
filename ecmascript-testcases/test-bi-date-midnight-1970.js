/*===
1970-01-01T00:00:00.000Z
0 Infinity
===*/

/* Midnight Jan 1, 1970 time value is +0 (check sign).  E5.1 Section 15.9.1.1. */

function midnight1970Test() {
    var d;

    d = new Date(Date.UTC(1970, 0, 1, 0, 0, 0, 0));
    print(d.toISOString());
    print(d.getTime(), 1 / d.getTime());  // 1/x to test sign
}

try {
    midnight1970Test();
} catch (e) {
    print(e.name);
}
