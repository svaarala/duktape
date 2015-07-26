/*
 *  Bug noticed while doing underscore shimming, case fallthrough does not
 *  work correctly (it used to work but has broken at some point).
 *
 *  The critical thing is that there are two case statements and the first
 *  case matches and falls through to the second (non-matching).
 */

/*===
matches 3
matches 3
matches 3
matches [object Date]
===*/

function test1a() {
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

function test1b() {
    var v = 2;
    switch (v) {
        case 1:
            print('matches 1');
            break;
        case 3:
        case 2:
            print('matches 3');
            break;
        default:
            print('matches default');
    }
}

function test1c() {
    var v = 2;
    switch (v) {
        case 1:
            print('matches 1');
            break;
        case 2:
            ;  // this is enough to stop triggering the bug
        case 3:
            print('matches 3');
            break;
        default:
            print('matches default');
    }
}

// Check also with strings (originally found with this)
function test2a() {
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
    test1a();
} catch (e) {
    print(e);
}

try {
    test1b();
} catch (e) {
    print(e);
}

try {
    test1c();
} catch (e) {
    print(e);
}

try {
    test2a();
} catch (e) {
    print(e);
}
