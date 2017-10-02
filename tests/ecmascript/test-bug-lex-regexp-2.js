/*
 *  Reported by Karl Dahlke.
 */

/*===
true
true
true
done
===*/

var src1 = '/blah/.test("blah")';
var src2 = 'if (1==1) { /blah/.test("blah") }';
var src3 = 'if (1==1) /blah/.test("blah")';

try {
    print(eval(src1));
} catch (e) {
    print(e.stack || e);
}

try {
    print(eval(src2));
} catch (e) {
    print(e.stack || e);
}

// This form failed to parse at least up to Duktape 2.2.
try {
    print(eval(src3));
} catch (e) {
    print(e.stack || e);
}

print('done');
