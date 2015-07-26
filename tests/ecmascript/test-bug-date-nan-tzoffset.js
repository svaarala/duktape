/*===
NaN
===*/

try {
    var d = new Date(NaN);
    print(d.getTimezoneOffset());
} catch (e) {
    print(e);
}
