/*
 *  Node.js Buffer copy()
 */

/*@include util-nodejs-buffer.js@*/
/*@include util-checksum-string.js@*/

/*===
Node.js Buffer copy() test
fox foxDEFGH
fox foxDEFGH
fox AfoxEFGH
fox ABfoxFGH
fox ABCfoxGH
fox ABCDfoxH
fox ABCDEfox
fox ABCDEFfo
fox ABCDEFGf
fox ABCDEFGH
fox ABCDEFGH
fox ABCDEFGH
fox ABCoEFGH
fox ABCoxFGH
fox ABCoxFGH
fox ABCoxFGH
0 0 2715161
0 1 2943275
0 2 2759183
0 3 2760699
0 4 2800001
0 5 2655183
0 6 2629025
0 7 2629401
0 8 2629777
0 9 2631657
0 10 2656423
0 11 2683009
0 12 2657429
0 13 2657777
0 14 2658125
1 0 2946611
1 1 3026096
1 2 2867472
1 3 2877764
1 4 2870890
1 5 1776336
1 6 2735686
1 7 2739402
1 8 2743234
1 9 2755959
1 10 2772403
1 11 2778807
1 12 2737072
1 13 2733328
1 14 2733622
2 0 2762099
2 1 2863056
2 2 2693634
2 3 2701090
2 4 2719444
2 5 1621948
2 6 2552082
2 7 2553958
2 8 2558742
2 9 2567261
2 10 2584999
2 11 2612619
2 12 2550216
2 13 2547094
2 14 2547424
3 0 2758998
3 1 2858543
3 2 2688343
3 3 2694407
3 4 2713687
3 5 1624198
3 6 2547637
3 7 2552317
3 8 2553829
3 9 2564220
3 10 2582010
3 11 2609296
3 12 2551034
3 13 2549136
3 14 2549466
4 0 2797129
4 1 2873482
4 2 2722400
4 3 2729916
4 4 2759022
4 5 1637214
4 6 2591460
4 7 2596360
4 8 2600772
4 9 2610689
4 10 2626583
4 11 2642357
4 12 2590816
4 13 2588562
4 14 2588890
5 0 1567052
5 1 1776846
5 2 1608166
5 3 1609528
5 4 1647412
5 5 1501234
5 6 1484928
5 7 1485274
5 8 1485620
5 9 1487350
5 10 1502562
5 11 1537218
5 12 1503478
5 13 1503770
5 14 1504062
6 0 2634525
6 1 2734478
6 2 2551638
6 3 2554366
6 4 2586590
6 5 1485686
6 6 2455854
6 7 2458544
6 8 2460320
6 9 2470407
6 10 2461451
6 11 2475867
6 12 2431764
6 13 2428358
6 14 2428712
7 0 2624378
7 1 2731101
7 2 2545761
7 3 2551293
7 4 2581963
7 5 1485986
7 6 2448037
7 7 2455513
7 8 2457049
7 9 2464506
7 10 2456534
7 11 2472216
7 12 2430516
7 13 2428644
7 14 2428998
8 0 2608407
8 1 2725736
8 2 2540652
8 3 2548726
8 4 2575550
8 5 1486286
8 6 2442028
8 7 2450766
8 8 2456700
8 9 2464905
8 10 2454435
8 11 2471217
8 12 2430468
8 13 2428930
8 14 2429284
9 0 2547708
9 1 2695146
9 2 2510094
9 3 2519057
9 4 2547032
9 5 1487786
9 6 2415154
9 7 2422237
9 8 2427404
9 9 2455946
9 10 2451922
9 11 2458052
9 12 2434840
9 13 2430360
9 14 2430714
10 0 2554672
10 1 2715606
10 2 2525936
10 3 2532220
10 4 2574248
10 5 1502666
10 6 2411274
10 7 2419440
10 8 2425456
10 9 2445548
10 10 2450112
10 11 2472298
10 12 2437612
10 13 2437752
10 14 2438106
11 0 2579720
11 1 2727916
11 2 2564690
11 3 2574332
11 4 2597746
11 5 1538364
11 6 2430380
11 7 2438636
11 8 2444888
11 9 2465794
11 10 2481956
11 11 2508598
11 12 2467436
11 13 2467200
11 14 2467530
12 0 2552600
12 1 2719909
12 2 2529165
12 3 2536212
12 4 2575535
12 5 1504026
12 6 2416309
12 7 2422656
12 8 2427552
12 9 2449663
12 10 2454026
12 11 2474642
12 12 2441080
12 13 2438944
12 14 2439298
13 0 2552582
13 1 2720327
13 2 2531485
13 3 2535045
13 4 2575991
13 5 1504366
13 6 2414389
13 7 2416729
13 8 2421494
13 9 2443626
13 10 2448500
13 11 2471216
13 12 2439378
13 13 2439242
13 14 2439596
14 0 2554617
14 1 2720378
14 2 2532162
14 3 2535456
14 4 2576242
14 5 1504706
14 6 2414792
14 7 2416794
14 8 2418818
14 9 2440764
14 10 2447880
14 11 2471414
14 12 2439798
14 13 2439540
14 14 2439894
15 0 2568931
15 1 2728179
15 2 2539653
15 3 2541459
15 4 2584793
15 5 1503704
15 6 2423367
15 7 2424621
15 8 2425452
15 9 2428091
15 10 2437270
15 11 2469472
15 12 2438288
15 13 2438642
15 14 2438996
16 0 2574342
16 1 2732760
16 2 2543794
16 3 2544838
16 4 2588582
16 5 1504044
16 6 2427326
16 7 2427680
16 8 2428034
16 9 2429804
16 10 2437484
16 11 2467792
16 12 2438586
16 13 2438940
16 14 2439294
17 0 2574674
17 1 2733068
17 2 2544112
17 3 2545156
17 4 2588908
17 5 1504384
17 6 2427610
17 7 2427964
17 8 2428318
17 9 2430088
17 10 2437782
17 11 2468110
17 12 2438884
17 13 2439238
17 14 2439592
===*/

function nodejsBufferCopyTest() {
    var b1, b2;

    // A few simple manual tests
    b1 = new Buffer('fox');
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 0); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 1); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 2); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 3); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 4); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 5); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 6); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 7); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 8); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 3, 1, 0); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 3, 1, 1); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 3, 1, 2); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 3, 1, 3); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 3, 1, 4); print(b1, b2);
    b2 = new Buffer('ABCDEFGH'); b1.copy(b2, 3, 1, 8); print(b1, b2);

    // A bunch of tests, copying from b1 to b2 or b1 to b1 (in-place).

    // Since target can be b1 or b2, ensure values are useful for both tests.
    var targetOffset = [
        'NONE',
        undefined,
        null,
        true,
        false,
        -1,
        0,
        1,
        2,
        7,
        11,
        13.8,
        15,
        16,
        17,
        23,
        24,
        25
    ];

    // These only need to work with b1
    var sourceOffset = [
        'NONE',
        undefined,
        null,
        true,
        false,
        -1,
        0,
        1,
        2,
        7,
        11,
        13.8,
        23,
        24,
        25
    ];

    targetOffset.forEach(function (targetStart, idx1) {
        sourceOffset.forEach(function (sourceStart, idx2) {
            var tmp = [];

            sourceOffset.forEach(function (sourceEnd, idx3) {
                var b1, b2;
                var i;

                //print(targetStart, sourceStart, sourceEnd);

                try {
                    b1 = new Buffer(24);
                    for (i = 0; i < b1.length; i++) {
                        b1[i] = 0x61 + i;
                    }
                    b2 = new Buffer(16);
                    for (i = 0; i < b2.length; i++) {
                        b2[i] = 0x2e;
                    }
                    if (targetStart == 'NONE') {
                        b1.copy(b2);
                    } else if (sourceStart == 'NONE') {
                        b1.copy(b2, targetStart);
                    } else if (sourceEnd == 'NONE') {
                        b1.copy(b2, targetStart, sourceStart);
                    } else {
                        b1.copy(b2, targetStart, sourceStart, sourceEnd);
                    }
                    tmp.push(printableNodejsBuffer(b1));
                    tmp.push(printableNodejsBuffer(b2));
                    tmp.push(targetStart + ' ' + sourceStart + ' ' + sourceEnd + ' ' +
                             String(b1) + ' ' + String(b2));
                } catch (e) {
                    tmp.push(targetStart + ' ' + sourceStart + ' ' + sourceEnd + ' ' +
                             String(b1) + ' ' + String(b2), e.name);
                }

                 // In-place case
                try {
                    b1 = new Buffer(24);
                    for (i = 0; i < b1.length; i++) {
                        b1[i] = 0x61 + i;
                    }
                    if (targetStart == 'NONE') {
                        b1.copy(b1);
                    } else if (sourceStart == 'NONE') {
                        b1.copy(b1, targetStart);
                    } else if (sourceEnd == 'NONE') {
                        b1.copy(b1, targetStart, sourceStart);
                    } else {
                        b1.copy(b1, targetStart, sourceStart, sourceEnd);
                    }
                    tmp.push(printableNodejsBuffer(b1));
                    tmp.push(targetStart + ' ' + sourceStart + ' ' + sourceEnd + ' ' +
                             String(b1));
                } catch (e) {
                    tmp.push(targetStart + ' ' + sourceStart + ' ' + sourceEnd + ' ' +
                             String(b1) + ' ' + e.name);
                }
            });

            print(idx1, idx2, checksumString(tmp.join('\n')));
        });
    });
}

try {
    print('Node.js Buffer copy() test');
    nodejsBufferCopyTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Node.js Buffer copy() retval test
8
ABCDEFGH ABCDEFGH 4142434445464748 4142434445464748
3
ABC abABCfgh 414243 6162414243666768
6
ABCDEFGHIJK abABCDEF 4142434445464748494a4b 6162414243444546
RangeError
ABCDEFGHIJK abcdefgh 4142434445464748494a4b 6162636465666768
7
ABCDEFGH aABCDEFG 4142434445464748 6141424344454647
2
ABCDEFGH aFGdefgh 4142434445464748 6146476465666768
3
ABCDEFGH abcdeDEF 4142434445464748 6162636465444546
RangeError
ABCDEFGH abcdefgh 4142434445464748 6162636465666768
RangeError
ABCDEFGH abcdefgh 4142434445464748 6162636465666768
RangeError
ABCDEFGH abcdefgh 4142434445464748 6162636465666768
===*/

/* Return value test: return value must be #bytes written after clamping
 * etc has been taken into account.
 */

function nodejsBufferCopyRetvalTest() {
    var b1, b2;

    // Equal size, zero offset.  Return value is 8.

    b1 = new Buffer('ABCDEFGH');
    b2 = new Buffer('abcdefgh');
    print(b1.copy(b2));
    print(String(b1), String(b2), printableNodejsBuffer(b1), printableNodejsBuffer(b2));

    // Source is smaller than target, offset.

    b1 = new Buffer('ABC');
    b2 = new Buffer('abcdefgh');
    print(b1.copy(b2, 2));
    print(String(b1), String(b2), printableNodejsBuffer(b1), printableNodejsBuffer(b2));

    // Source is larger than target, offset.

    b1 = new Buffer('ABCDEFGHIJK');
    b2 = new Buffer('abcdefgh');
    print(b1.copy(b2, 2));
    print(String(b1), String(b2), printableNodejsBuffer(b1), printableNodejsBuffer(b2));

    // Source is larger than offset; use negative offset -> RangeError
    // instead of clipping from the left.

    try {
        b1 = new Buffer('ABCDEFGHIJK');
        b2 = new Buffer('abcdefgh');
        print(b1.copy(b2, -1));
    } catch (e) {
        print(e.name);
    }
    print(String(b1), String(b2), printableNodejsBuffer(b1), printableNodejsBuffer(b2));

    // Equal size, offset 1: one byte is clipped.  Return value is 7,
    // actual #bytes copied.

    b1 = new Buffer('ABCDEFGH');
    b2 = new Buffer('abcdefgh');
    print(b1.copy(b2, 1));
    print(String(b1), String(b2), printableNodejsBuffer(b1), printableNodejsBuffer(b2));

    // Offset/length in source; copy 2 bytes (5,6) to offsets (1,2).

    b1 = new Buffer('ABCDEFGH');
    b2 = new Buffer('abcdefgh');
    print(b1.copy(b2, 1, 5, 7));
    print(String(b1), String(b2), printableNodejsBuffer(b1), printableNodejsBuffer(b2));

    // Offset/length in source; copy 4 bytes (3,4,5,6) to offsets (5,6,7),
    // clipping 1 byte.

    b1 = new Buffer('ABCDEFGH');
    b2 = new Buffer('abcdefgh');
    print(b1.copy(b2, 5, 3, 7));
    print(String(b1), String(b2), printableNodejsBuffer(b1), printableNodejsBuffer(b2));

    // Negative values cause RangeErrors.

    try {
        b1 = new Buffer('ABCDEFGH');
        b2 = new Buffer('abcdefgh');
        print(b1.copy(b2, -3));
    } catch (e) {
        print(e.name);
    }
    print(String(b1), String(b2), printableNodejsBuffer(b1), printableNodejsBuffer(b2));

    try {
        b1 = new Buffer('ABCDEFGH');
        b2 = new Buffer('abcdefgh');
        print(b1.copy(b2, 1, -1));
    } catch (e) {
        print(e.name);
    }
    print(String(b1), String(b2), printableNodejsBuffer(b1), printableNodejsBuffer(b2));

    try {
        b1 = new Buffer('ABCDEFGH');
        b2 = new Buffer('abcdefgh');
        print(b1.copy(b2, 1, 3, -1));
    } catch (e) {
        print(e.name);
    }
    print(String(b1), String(b2), printableNodejsBuffer(b1), printableNodejsBuffer(b2));
}

try {
    print('Node.js Buffer copy() retval test');
    nodejsBufferCopyRetvalTest();
} catch (e) {
    print(e.stack || e);
}
