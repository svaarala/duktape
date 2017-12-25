/*
 *  Trailing gaps in map() input are included in the result length, but the
 *  user-supplied callback is not called.
 */

/*===
6 1 2 nonexistent nonexistent 3 nonexistent
map fn: number 1
map fn: number 2
map fn: number 3
6 1 4 nonexistent nonexistent 9 nonexistent
10 nonexistent nonexistent nonexistent 10 nonexistent nonexistent nonexistent 20 nonexistent nonexistent
map fn: number 10
map fn: number 20
10 nonexistent nonexistent nonexistent 100 nonexistent nonexistent nonexistent 400 nonexistent nonexistent
===*/

function test() {
    function mapper(x) {
        print('map fn:', typeof x, x);
        return x*x;
    }
    function dump(v) {
        var tmp = [];
        for (var i = 0; i < v.length; i++) {
            if (i in v) { tmp.push(v[i]) } else { tmp.push('nonexistent'); }
        }
        print(v.length, tmp.join(' '));
    }

    var a = [ 1, 2, , , 3, ,  ];  // -> items: 1 2 na na 3 na, length 6
    dump(a);

    var b = a.map(mapper);
    dump(b);

    var c = { '3': 10, '7': 20, 'length': 10 };
    dump(c);

    var d = Array.prototype.map.call(c, mapper);
    dump(d);
}

try {
    test();
} catch (e) {
    print(e);
}
