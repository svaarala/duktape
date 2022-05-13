// Non-object argument is returned as is.

/*===
123
===*/

try {
    var res = Object.freeze(123);
    print(res);
} catch (e) {
    print(e.stack || e);
}
