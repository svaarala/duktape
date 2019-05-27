/*
 *  Cleaned up repro from https://github.com/svaarala/duktape/issues/2061.
 */

/*===
true
done
===*/

try {
    Duktape.errCreate = function(e) {
        return Float64Array;
    };
    100000 .foo();
} catch (e) {
    print(e === Float64Array);
}
print('done');
