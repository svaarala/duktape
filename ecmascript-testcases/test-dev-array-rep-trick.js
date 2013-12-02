/*===
10 x
10 y
100000 x
100000 y
===*/

/* String repeat can be implemented with an array join. */

function dump(x) {
    var ch = '?'
    if (x.length > 0) {
        ch = x.charAt(0);
        for (var i = 0; i < x.length; i++) {
            if (x.charAt(i) !== ch) {
                throw new Error('error in string');
            }
        }
    }
    print(x.length, ch);
}

try {
    dump(Array.prototype.join.call({ length: 10 + 1 }, 'x'));
    var arr = []; arr.length = 10 + 1;
    dump(arr.join('y'));
} catch (e) {
    print(e);
}

try {
    dump(Array.prototype.join.call({ length: 100000 + 1 }, 'x'));
    var arr = []; arr.length = 100000 + 1;
    dump(arr.join('y'));
} catch (e) {
    print(e);
}
