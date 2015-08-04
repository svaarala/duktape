/*===
object number
true
true
===*/

function dateNowTest() {
    var d1, d2;

    // these two are equivalent
    d1 = new Date();  // Date object
    d2 = Date.now();  // number
    print(typeof d1, typeof d2);
    print(Math.abs(d1.getTime() - d2) < 5000);  // within 5 sec

    // probably right century; give two arguments to avoid implementation specific
    // behavior
    print(d2 >= Date.UTC(2000, 0) &&
          d2 <= Date.UTC(2100, 0));
}

try {
    dateNowTest();
} catch (e) {
    print(e.name);
}
