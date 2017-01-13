/*===
TypeError
still here
===*/

var x = new Float64Array(2)
try {
    x.set(undefined)
} catch (e) {
    print(e.name);
}
print('still here');
