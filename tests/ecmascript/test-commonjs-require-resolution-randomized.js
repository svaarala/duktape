/*
 *  Randomized identifier testing to stress test identifier resolution.
 */

/*===
0
10000
20000
30000
40000
50000
60000
70000
80000
90000
done
===*/

function rnd() {
    return Math.random();
}

function buildId() {
    var res = '';
    var x;

    for (;;) {
        x = rnd() * 1000;
        if (x >= 100) {
            res += String.fromCharCode( (x - 100) + 0x20 );  // unicode but not control
        } else if (x <= 30) {
            res += '/';
        } else if (x <= 60) {
            res += '/./';
        } else if (x <= 90) {
            res += '/../';
        } else {
            break;
        }
    }

    return res;
}

function randomizedTest() {
    var i, n;
    var succ = 0;

    Duktape.modSearch = function (id) {
        succ++;
        throw new Error('module not found');
    };

    for (i = 0, n = 1e5; i < n; i++) {
        //print(buildId());
        if ((i % 10000) == 0) { print(i); }
        try {
            mod = require(buildId());
        } catch (e) {
            //print(e.name);
        }
    }

    //print('successful: ' + succ + '/' + n);
}

try {
    randomizedTest();
} catch (e) {
    print(e);
}

print('done');
