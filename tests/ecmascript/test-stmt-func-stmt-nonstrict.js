/*===
a called
a called
b called
b called
===*/

try {
    function a() {
        print('a called');
    }
    a();
} catch (e) {
    print(e.stack || e);
}
// In Node.js v12.7.0 'a' is still visible here, i.e. declaration
// is hoisted.
a();

function test() {
    try {
        function b() {
            print('b called');
        }
        b();
    } catch (e) {
        print(e.stack || e);
    }
    // Also hoisted here.
    b();
}
test();
