/*===
TypeError
done
===*/

try {
    var R = /foo/;
    var O = Object.create(R);
    print(O.source);
} catch (e) {
    print(e.name);
}
print('done');
