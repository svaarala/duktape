function test() {
    var i;
    var arr;

    for (i = 0; i < 1e4; i++) {
        arr = [];
        while (arr.length < 1e3) {
            arr[arr.length] = 'foo'; arr[arr.length] = 'bar';
            arr[arr.length] = 'foo'; arr[arr.length] = 'bar';
            arr[arr.length] = 'foo'; arr[arr.length] = 'bar';
            arr[arr.length] = 'foo'; arr[arr.length] = 'bar';
            arr[arr.length] = 'foo'; arr[arr.length] = 'bar';
        }
    }
}

test();
