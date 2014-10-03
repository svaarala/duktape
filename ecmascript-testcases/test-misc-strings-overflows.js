/*
 *  String overflow tests.
 *
 *  Strings overflow at 4G bytes (at the latest) even on 64-bit platforms,
 *  at least for the moment.  Most string concat operations now use duk_concat()
 *  internally, which is exercised fully by Array.prototype.join().
 */

/*===
1048576
===*/

var oneMeg = 'x';
var i;
for (i = 0; i < 20; i++) {
    oneMeg += oneMeg;
}
print(oneMeg.length);

/*===
RangeError
RangeError
===*/

function arrayJoinSepOverflow() {
    /* The internal helper first computes total size of separators, so see
     * that the overflow check for that works.
     */

    var test = [];
    var i;
    for (i = 0; i < 4097; i++) {
        test[i] = '';
    }
    test.join(oneMeg);  /* 4096 seps * 1M = 2**32 -> overflow */
    print('still here');
}

function arrayJoinResultOverflow() {
    var test = [];
    var i;
    for (i = 0; i < 4097; i++) {
        test[i] = oneMeg;
    }
    test.join('');
    print('still here');
}

try {
    arrayJoinSepOverflow();
} catch (e) {
    print(e.name);
}

try {
    arrayJoinResultOverflow();
} catch (e) {
    print(e.name);
}
