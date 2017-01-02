function test() {
    var arr = [];
    while (arr.length < 1e5) {
        arr.push(new Uint8Array(256));
    }
    print(arr.length + ' Uint8Arrays created');
}
test();
