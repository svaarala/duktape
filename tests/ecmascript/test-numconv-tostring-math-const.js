/*===
2.718281828459045
2.302585092994046
0.6931471805599453
1.4426950408889634
0.4342944819032518
3.141592653589793
0.7071067811865476
1.4142135623730951
===*/

function mathConstTest() {
    var names = [ 'E', 'LN10', 'LN2', 'LOG2E', 'LOG10E', 'PI', 'SQRT1_2', 'SQRT2' ];
    var i;

    for (i = 0; i < names.length; i++) {
        print(Math[names[i]]);
    }
}

try {
    mathConstTest()
} catch (e) {
    print(e);
}
