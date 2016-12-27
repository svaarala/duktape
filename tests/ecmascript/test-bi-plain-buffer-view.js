/*@include util-buffer.js@*/

/*===
view test
[object Uint32Array]
4 16 0 4
[object ArrayBuffer] false
1616928864
|60606060616161616262626263636363|
===*/

function viewTest() {
    var pb = createPlainBuffer('abcdefghijklmnop');
    var view;
    var i;

    for (i = 0; i < pb.length; i++) {
        pb[i] = 0x60 + (i >> 2);  // endian neutral value
    }

    // Create typedarray on top of plain buffer.  The inherited .buffer
    // getter creates an ArrayBuffer on-the-fly.  Because there's no place
    // to store the result, each .buffer read generates a new ArrayBuffer.

    view = new Uint32Array(pb.buffer);
    print(Object.prototype.toString.call(view));
    print(view.length, view.byteLength, view.byteOffset, view.BYTES_PER_ELEMENT);
    print(view.buffer, view.buffer === pb);
    print(view[0]);
    print(Duktape.enc('jx', view));
}

try {
    print('view test');
    viewTest();
} catch (e) {
    print(e.stack || e);
}
