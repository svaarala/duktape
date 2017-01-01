/*===
true
===*/

function test() {
    var re = /[--]/;
    print(re.test('-'));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
