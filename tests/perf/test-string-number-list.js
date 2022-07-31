function test() {
    var arr = [];
    for (var i = 0; i < 1e6; i++) {
        arr.push(String(i));
    }
    print(arr[1e6-1]);
}

test();
