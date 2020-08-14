/*
 *  Basic property write performance, array index writes extending an array
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr = [];
    var i;

    var src = [];
    src.push('(function () {');
    src.push('    var i, j, arr;');
    src.push('    for (i = 0; i < 1e6; i++) {');
    src.push('        arr = [];');
    for (i = 0; i < 100; i++) {
        src.push('        arr[' + i + '] = 123;');
    }
    src.push('    }');
    src.push('})');
    src = src.join('\n');
    //print(src);
    var code = eval(src);

    code();
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
