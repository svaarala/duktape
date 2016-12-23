/*@include util-buffer.js@*/

/*===
ArrayBuffer methods
- isView
true
===*/

function arrayBufferMethodTest() {
    var pb;

    pb = createPlainBuffer('abcdefghijklmnop');

    // ArrayBuffer only has .isView()
    print('- isView');
    print(ArrayBuffer.isView(pb));
}

try {
    print('ArrayBuffer methods');
    arrayBufferMethodTest();
} catch (e) {
    print(e.stack || e);
}
