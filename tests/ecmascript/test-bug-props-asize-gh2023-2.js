/*
 *  https://github.com/svaarala/duktape/issues/2023
 *
 *  Modified variant.
 */

/*===
done
===*/

var input = [];
input[65536] = 0;
var output = input.map(Math.cos);

print('done');
