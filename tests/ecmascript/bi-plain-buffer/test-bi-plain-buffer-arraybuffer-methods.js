/*@include util-buffer.js@*/

/*===
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

arrayBufferMethodTest();
