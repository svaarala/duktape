/*===
10000002
undefined
undefined
123
===*/

function test() {
    var src = '[' + ','.repeat(1e7) + ',123]';
    var arr = eval(src);
    //print(Duktape.enc('jx', Duktape.info(arr)));
    print(arr.length);
    print(arr[0]);
    print(arr[1e7]);
    print(arr[1e7 + 1]);
}

test();
