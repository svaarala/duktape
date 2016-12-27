function test() {
    var arr = [];
    while (arr.length < 1e5) {
        arr.push(Uint8Array.allocPlain(256));
    }
    print(arr.length + ' plain buffers created');
}
test();
