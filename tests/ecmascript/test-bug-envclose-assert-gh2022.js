/*
 *  https://github.com/svaarala/duktape/issues/2022
 */

/*===
done
===*/

function test ( ) {
    try {
        test();
    } catch ( x ) {
        TypeError.stack
    }
    var i;
    var f = eval( f );
}

test();
print('done');
