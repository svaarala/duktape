/*
 *  Build string with plain concat.  Causes O(n^2) behavior in Duktape
 *  string implementation because every intermediate step is string interned.
 */

function test() {
    var i, j;
    var t;

    for (i = 0; i < 1; i++) {
        t = '';
        for (j = 0; j < 1e5; j++) {
            t += 'x';
            //print(t.length);
        }
    }
}

test();
