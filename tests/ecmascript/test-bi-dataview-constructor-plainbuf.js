/*@include util-buffer.js@*/

/*===
TypeError
===*/

function test() {
    // Because only ArrayBuffers are accepted and plain buffers mimic
    // Uint8Arrays, they are rejected by DataView constructor.

    try {
        var dv = new DataView(createPlainBuffer('abcd'));
        print('never here!');
    } catch (e) {
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
