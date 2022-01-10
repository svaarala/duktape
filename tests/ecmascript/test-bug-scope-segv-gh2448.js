// https://github.com/svaarala/duktape/issues/2448

/*===
RangeError
===*/

function JSEtest() {
    var src = [];
    var i;

    src.push('(function test() {');
    for (i = 0; i < 1e4; i++) {
        src.push('var x' + i + ' = ' + i + ';');
    }
    src.push('var arguments = test(); return "dummy"; })');
    src = src.join('');
    //print(src);

    var f = eval(src)(src);

    try {
        f();
    } catch (e) {
        print(e.name + ': ' + e.message);
    }

    print('still here');
}

try {
    JSEtest();
} catch (e) {
    //print(e.stack || e);
    print(e.name);
}
