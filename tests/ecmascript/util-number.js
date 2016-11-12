/*
 *  Number test utils.
 *
 *  Difficulties arise from:
 *
 *    - Trigonometric functions are not required to be bit exact
 *    - Random number testing
 *    - Some calls distinguish between +0 and -0 (e.g. asin)
 *
 *  To distinguish zero signs, the idiom '1 / x' is used.  If x is +0,
 *  this will result in Infinity, and if x is -0, it will result in
 *  -Infinity.
 */

function printRounded6(x) {
    if (x === 0 && 1 / x < 0) {
        print('-' + Math.round(x * 1000000));
    } else {
        print(Math.round(x * 1000000));
    }
}

function toStringExact(x) {
    if (x === 0 && 1 / x < 0) {
        return '-0';
    } else {
        return String(x);
    }
}
function printExact(x) {
    print(toStringExact(x));
}
