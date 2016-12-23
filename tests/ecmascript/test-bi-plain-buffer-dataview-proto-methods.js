/*@include util-buffer.js@*/

/*===
DataView.prototype methods
- getUint16
|6162636465666768696a6b6c6d6e6f70|
26215
26470
- setInt32
|6162636465666768696a6b6c6d6e6f70|
undefined
|616263deadbeef68696a6b6c6d6e6f70|
undefined
|616263deadbeef68efbeadde6d6e6f70|
===*/

function dataViewPrototypeMethodTest() {
    var pb, t;

    pb = createPlainBuffer('abcdefghijklmnop');

    // Spot check one get method
    print('- getUint16');
    print(Duktape.enc('jx', pb));
    print(DataView.prototype.getUint16.call(pb, 5, false));  // 66 67 -> 0x6667 big endian
    print(DataView.prototype.getUint16.call(pb, 5, true));   // 66 67 -> 0x6766 little endian

    // Spot check one set method
    print('- setInt32');
    print(Duktape.enc('jx', pb));
    print(DataView.prototype.setInt32.call(pb, 3, 0xdeadbeef, false));
    print(Duktape.enc('jx', pb));
    print(DataView.prototype.setInt32.call(pb, 8, 0xdeadbeef, true));
    print(Duktape.enc('jx', pb));
}

try {
    print('DataView.prototype methods');
    dataViewPrototypeMethodTest();
} catch (e) {
    print(e.stack || e);
}
