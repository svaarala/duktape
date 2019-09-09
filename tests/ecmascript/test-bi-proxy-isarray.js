/*===
[1,2,3]
true
===*/

try {
    var P = new Proxy([1, 2, 3], {});
    print(JSON.stringify(P));
    print(Array.isArray(P));
} catch (e) {
    print(e.stack || e);
}
