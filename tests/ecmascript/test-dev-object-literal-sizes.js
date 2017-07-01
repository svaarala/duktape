/*
 *  Object literals of different sizes
 */

/*===
0
100
200
300
400
500
600
700
800
900
done
===*/

function makeTestFunction(count) {
    var src = [];
    var i;

    src.push('(function () {');
    src.push('    var obj = {');
    for (i = 0; i < count; i++) {
        src.push('        key' + i + ': "val' + i + '",');
    }
    src.push('    };');
    src.push('    return obj;');
    src.push('})');
    return eval(src.join('\n') + '\n');
}

function test() {
    var i, j, obj, fn;

    for (i = 0; i < 1000; i++) {
        if ((i % 100) == 0) {
            print(i);
        }
        fn = makeTestFunction(i);
        for (j = 0; j < 100; j++) {
            obj = fn();
        }
        if (Object.keys(obj).length !== i) {
            throw new Error('failed for i: ' + i);
        }
    }
    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
