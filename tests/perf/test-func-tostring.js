function test() {
    var fn = function foo() {};
    var i;

    for (i = 0; i < 1e6; i++) {
        fn.toString();
        fn.toString();
        fn.toString();
        fn.toString();
        fn.toString();
        fn.toString();
        fn.toString();
        fn.toString();
        fn.toString();
        fn.toString();
    }
}

test();
