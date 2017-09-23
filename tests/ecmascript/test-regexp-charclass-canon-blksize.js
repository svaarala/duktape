/*
 *  Stress test for case insensitive regexp normalization, all ranges
 *  of canon blksize-1, blksize, blksize+1.
 *
 *  This is disabled by default because the test takes hours to run.
 *  Run manually when changing regexp code.
 */

/*@include util-regexp.js@*/

/*---
{
    "skip": true,
    "slow": true
}
---*/

/*===
===*/

function test() {
    var blksize = 32;  // current blksize constant
    var re;
    var i;
    var upper;
    var limit = 0xffff;

    for (i = 0; i <= limit; i++) {
        upper = i + blksize - 1;
        if (upper <= 0xffff) {
            re = RegExpUtil.makeCaseInsensitiveCharClassRegExp(i, upper);
            print(i, re.source, RegExpUtil.getRegExpSingleCharMatches(re));
        }
    }
    for (i = 0; i <= limit; i++) {
        upper = i + blksize;
        if (upper <= 0xffff) {
            re = RegExpUtil.makeCaseInsensitiveCharClassRegExp(i, upper);
            print(i, re.source, RegExpUtil.getRegExpSingleCharMatches(re));
        }
    }
    for (i = 0; i <= limit; i++) {
        upper = i + blksize + 1;
        if (upper <= 0xffff) {
            re = RegExpUtil.makeCaseInsensitiveCharClassRegExp(i, upper);
            print(i, re.source, RegExpUtil.getRegExpSingleCharMatches(re));
        }
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
