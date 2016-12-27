function test() {
    var arr = [];
    while (arr.length < 1e5) {
        arr.push(function () {});
    }
    print(arr.length + ' anonymous functions created');
}
test();
