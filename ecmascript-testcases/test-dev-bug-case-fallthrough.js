/*
 *  Bug noticed while doing underscore shimming, case fallthrough does not
 *  work correctly (it used to work but has broken at some point).
 */

/*===
matches 3
matches [object Date]
===*/

function test1() {
    var v = 2;
    switch (v) {
        case 1:
            print('matches 1');
            break;
        case 2:
        case 3:
            print('matches 3');
            break;
        default:
            print('matches default');
    }
}

function test2() {
    var date = new Date(2014, 1, 2);
    switch (Object.prototype.toString.call(date)) {
        case '[object String]':
            print('matches [object String]');
            break;
        case '[object Date]':
        case '[object Boolean]':
            print('matches [object Date]');
            break;
        default:
            print('matches default');
    }
}


try {
    test1();
} catch (e) {
    print(e);
}

try {
    test2();
} catch (e) {
    print(e);
}

