/*===
undefined
function
===*/

function test() {
    print(typeof new.target);
}

try {
    test();
    new test();
} catch (e) {
    print(e.stack || e);
}
