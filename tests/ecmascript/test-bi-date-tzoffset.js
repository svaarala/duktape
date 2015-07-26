/*===
no output expected
===*/

/* The underlying implementation of Date.prototype.getTimezoneOffset()
 * currently uses locale-specific datetime handling functions on UNIX.
 * Although we can't check that the implementation works correctly, we
 * can check that it doesn't fail with year values outside 32-bit UNIX
 * timestamp range (1970-2038).
 *
 * (Of course, some UNIX systems provide a 64-bit timestamp and have no
 * issues covering the Ecmascript date range.)
 */

function test(tv) {
    var d = new Date(tv);
    var t;

    t = d.getTimezoneOffset();

    function err(msg) {
        print('timevalue ' + tv + ': ' + msg +
              ' (typeof t=' + typeof t + ', t=' + t + ')');
    }

    if (typeof t !== 'number') {
        return err('invalid return type');
    }
    if (t !== Math.floor(t)) {
        return err('offset not a whole number');
    }
    if (!(t >= -12 * 60 && t <= 12 * 60)) {
        return err('offset not within expected +/- 12h range');
    }
}

var years = [
    1969, 1970, 1980, 1990, 2000, 2010, 2020, 2030, 2037, 2038, 2039,
    -250000, -100000, 100000, 250000
];

var i;

print('no output expected');

try {
    test(Date.now());

    for (i = 0; i < years.length; i++) {
        // Test January and July to get any DST effects
        test(Date.UTC(years[i], 0, 1, 0, 0, 0, 0));
        test(Date.UTC(years[i], 6, 1, 0, 0, 0, 0));
    }
} catch (e) {
    print(e.name);
}
