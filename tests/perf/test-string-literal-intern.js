/*
 *  Exercise the internal mechanism for intern checking a string literal
 *  pushed using e.g. duk_push_literal().  The ideal test target has a lot
 *  of literal interning and minimal other activity.  No really good target
 *  exists now, but 'String(Buffer.prototype)' just pushes a literal and
 *  returns.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var S = String;
    var BP = Buffer.prototype;
    var i;

    for (i = 0; i < 1e5; i++) {
        S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP);
        S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP);
        S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP);
        S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP);
        S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP);
        S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP);
        S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP);
        S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP);
        S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP);
        S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP); S(BP);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
