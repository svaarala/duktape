/*===
TypeError
===*/

var x = new Float64Array(2)
try {
    x.set(undefined)
} catch (e) {
    print(e.name);
}
