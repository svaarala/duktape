/*===
foo 10000
sum: 49995000
===*/

function test() {
    var source = [];
    var i;

    function foo() {
        var sum = 0;
        print('foo', arguments.length);
        for (var i = 0; i < arguments.length; i++) {
            sum += arguments[i];
        }
        print('sum:', sum);
    }

    source.push('foo(');
    for (i = 0; i < 10000; i++) {
        if (i > 0) { source.push(','); }
        source.push(i);
    }
    source.push(')');
    source = source.join('');

    eval(source);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
