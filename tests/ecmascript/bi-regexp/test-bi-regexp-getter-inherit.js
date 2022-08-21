/*===
TypeError
===*/

try {
    var R = /foo/;
    var O = Object.create(R);
    print(O.source);
} catch (e) {
    print(e.name);
}
