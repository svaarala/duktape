/*===
[object JSON]
true
TypeError
TypeError
===*/

/* [[Class]] is "JSON" */
print(Object.prototype.toString.call(JSON));

/* extensible */
print(Object.isExtensible(JSON));

try {
    // not constructable -> TypeError
    new JSON();
} catch (e) {
    print(e.name);
}

try {
    // not callable -> TypeError
    new JSON();
} catch (e) {
    print(e.name);
}
