/*
 *  Test for handling for large constant count with a large literal.
 *  Literal values are initialized using a fixed number of temps,
 *  so this does not require temp shuffling.
 */

/*===
array 0 0 0
array 10 10 40
array 100 100 490
array 253 253 1408
array 254 254 1414
array 255 255 1420
array 256 256 1426
array 257 257 1432
array 300 300 1690
array 3000 3000 19890
array 10000 10000 68890
array 100000 100000 788890
object 0 0 0
object 10 10 80
object 100 100 980
object 125 125 1280
object 126 126 1292
object 127 127 1304
object 128 128 1316
object 129 129 1328
object 130 130 1340
object 253 253 2816
object 254 254 2828
object 255 255 2840
object 256 256 2852
object 257 257 2864
object 300 300 3380
object 3000 3000 39780
object 10000 10000 137780
object 100000 100000 1577780
===*/

function createArrayConst(num) {
    var res = [];
    for (var i = 0; i < num; i++) {
        res.push('"foo' + i + '"');  // avoid sharing consts
    }
    return '([' + res.join(',') + '])';
}

function createObjectConst(num) {
    var res = [];
    for (var i = 0; i < num; i++) {
        res.push('foo' + i + ':"bar' + i + '"');  // avoid sharing consts
    }
    return '({' + res.join(',') + '})';
}

function testArray(num) {
    var t = eval(createArrayConst(num));
    var len = 0;  // combined string length (kind of a checksum)
    for (i = 0; i < t.length; i++) {
        len += t[i].length;
    }
    print('array', num, t.length, len);
}

function testObject(num) {
    var t = eval(createObjectConst(num));
    var len = 0;  // combined string length (kind of a checksum)
    var keys = Object.getOwnPropertyNames(t);
    for (i = 0; i < keys.length; i++) {
        len += keys[i].length + t[keys[i]].length;
    }
    print('object', num, keys.length, len);
}

try {
    [0, 10, 100, 253, 254, 255, 256, 257, 300, 3000, 10000, 100000]
        .forEach(function(x) { testArray(x); });
} catch (e) {
    print(e);
}

try {
    [0, 10, 100, 125, 126, 127, 128, 129, 130, 253, 254, 255, 256, 257, 300, 3000, 10000, 100000]
        .forEach(function(x) { testObject(x); });
} catch (e) {
    print(e);
}
