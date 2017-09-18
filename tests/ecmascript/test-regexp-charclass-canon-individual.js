/*
 *  Stress test for case insensitive regexp normalization, all BMP
 *  codepoints.
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
    var re;
    var i;

    for (i = 0x0; i <= 0xffff; i++) {
        re = RegExpUtil.makeCaseInsensitiveCharClassRegExp(i, i);
        print(i, re.source, RegExpUtil.getRegExpSingleCharMatches(re));
    }

}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
