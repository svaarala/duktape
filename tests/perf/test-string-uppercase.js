/*
 *  Test string uppercasing for a long ASCII snippet (fast path).
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var txt = 'Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.';
    var i;

    for (i = 0; i < 10; i++) {
        txt = txt + txt;
    }

    print(txt.length);

    for (i = 0; i < 1000; i++) {
        void txt.toUpperCase();
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
