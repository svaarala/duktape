function test() {
    var arr = [];
    var fn;
    while (arr.length < 1e4) {
        fn = function () {};
        fn.prototype = null;
        arr.push(fn);
    }
    print(arr.length + ' anonymous functions created');
}
test();
