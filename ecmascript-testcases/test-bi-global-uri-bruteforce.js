/*
 *  Brute force test cases for URI encoding and decoding functions.
 *
 *  There are no test cases for decoding extended UTF-8 codepoints
 *  because such strings must explicitly be rejected (this is
 *  specified quite clearly).  If a custom entrypoint with more
 *  control is added, separate testcases for that should be added.
 */

/*---
{
    "slow": true
}
---*/

// indirect eval -> this is bound to the global object, E5 Section 10.4.2, step 1.a.
var g = (function () { var e = eval; return e('this'); } )();

/* Dump a string as decimal codepoints, ensures that tests produce ASCII only
 * outputs.
 */
function dumpCodePoints(x) {
    var i;
    var res = [];

    for (i = 0; i < x.length; i++) {
        res.push(x.charCodeAt(i));
    }

    return res.join(' ');
}

/*
 *  Brute force URI encode/decode conversion for the entire BMP range.
 */

function genString(start, count) {
    var res = [];
    var i, cp;

    for (i = 0; i < count; i++) {
        cp = start + i;
        res[i] = String.fromCharCode(cp);
    }

    return res.join('');
}

/* Trivial string checksum used to summarize brute force output lines
 * (minimizes test case size).
 */
function checkSumString(x) {
    var i, n;
    var res = 0;
    var mult = [ 1, 3, 5, 7, 11, 13, 17, 19, 23 ];

    n = x.length;
    for (i = 0; i < n; i++) {
        res += x.charCodeAt(i) * mult[i % mult.length];
        res = res >>> 0;  // coerce to 32 bits
    }

    return res;
}

/*===
encodeURI BMP
0 true 3321596
1024 true 3710104
2048 true 5466624
3072 true 5477888
4096 true 5370368
5120 true 5381632
6144 true 5471744
7168 true 5483008
8192 true 5375488
9216 true 5386752
10240 true 5476864
11264 true 5488128
12288 true 5380608
13312 true 5391872
14336 true 5481984
15360 true 5493248
16384 true 5385728
17408 true 5396992
18432 true 5487104
19456 true 5498368
20480 true 5390848
21504 true 5402112
22528 true 5492224
23552 true 5503488
24576 true 5395968
25600 true 5407232
26624 true 5497344
27648 true 5508608
28672 true 5401088
29696 true 5412352
30720 true 5502464
31744 true 5513728
32768 true 5406208
33792 true 5417472
34816 true 5507584
35840 true 5518848
36864 true 5411328
37888 true 5422592
38912 true 5512704
39936 true 5523968
40960 true 5452288
41984 true 5463552
43008 true 5553664
44032 true 5564928
45056 true 5457408
46080 true 5468672
47104 true 5558784
48128 true 5570048
49152 true 5462528
50176 true 5473792
51200 true 5563904
52224 true 5575168
53248 true 5467648
54272 true 5478912
55296 URIError (expected)
56320 URIError (expected)
57344 true 5472768
58368 true 5484032
59392 true 5574144
60416 true 5585408
61440 true 5477888
62464 true 5489152
63488 true 5579264
64512 true 5590528
encodeURIComponent BMP
0 true 3177834
1024 true 3710104
2048 true 5466624
3072 true 5477888
4096 true 5370368
5120 true 5381632
6144 true 5471744
7168 true 5483008
8192 true 5375488
9216 true 5386752
10240 true 5476864
11264 true 5488128
12288 true 5380608
13312 true 5391872
14336 true 5481984
15360 true 5493248
16384 true 5385728
17408 true 5396992
18432 true 5487104
19456 true 5498368
20480 true 5390848
21504 true 5402112
22528 true 5492224
23552 true 5503488
24576 true 5395968
25600 true 5407232
26624 true 5497344
27648 true 5508608
28672 true 5401088
29696 true 5412352
30720 true 5502464
31744 true 5513728
32768 true 5406208
33792 true 5417472
34816 true 5507584
35840 true 5518848
36864 true 5411328
37888 true 5422592
38912 true 5512704
39936 true 5523968
40960 true 5452288
41984 true 5463552
43008 true 5553664
44032 true 5564928
45056 true 5457408
46080 true 5468672
47104 true 5558784
48128 true 5570048
49152 true 5462528
50176 true 5473792
51200 true 5563904
52224 true 5575168
53248 true 5467648
54272 true 5478912
55296 URIError (expected)
56320 URIError (expected)
57344 true 5472768
58368 true 5484032
59392 true 5574144
60416 true 5585408
61440 true 5477888
62464 true 5489152
63488 true 5579264
64512 true 5590528
===*/

/* Test encodeURI and encodeURIComponent for BMP.  Results are string hashed
 * to minimize output size.
 */

function encodeURIBMPTest(is_uricomponent) {
    var x1, x2, x3;
    var got_urierror;

    // step 1024 (= 0x400), ensure that 0xd800...0xdfff aligns nicely
    for (var i = 0; i < 65536; i += 1024) {
        got_urierror = false;
        try {
            x1 = genString(i, 1024);
            if (is_uricomponent) {
                x2 = g.encodeURIComponent(x1);
                x3 = g.decodeURIComponent(x2);
            } else {
                x2 = g.encodeURI(x1);
                x3 = g.decodeURI(x2);
            }
            print(i, (x1 === x3), checkSumString(x2));
        } catch (e) {
            if (e.name === 'URIError') {
                got_urierror = true;
            } else {
                throw e;
            }
        }

        if (i >= 0xd800 && i < 0xe000) {
            // For surrogate pairs encoded 'naively' into UTF-8,
            // URIError is required.
            if (got_urierror) {
                print(i, 'URIError (expected)');
            } else {
                print(i, 'URIError (unexpected)');
            }
        }
    }
}

try {
    print('encodeURI BMP');
    encodeURIBMPTest(false);
    print('encodeURIComponent BMP');
    encodeURIBMPTest(true);
} catch (e) {
    print(e.name, e);
}

/*===
encodeURI surrogate pairs
459501750
465268918
465989814
459676504
460397400
466164568
466885464
460572154
461293050
467060218
467781114
461467804
462188700
467955868
468676764
462363454
encodeURIComponent surrogate pairs
459501750
465268918
465989814
459676504
460397400
466164568
466885464
460572154
461293050
467060218
467781114
461467804
462188700
467955868
468676764
462363454
===*/

/* Test surrogate pair encoding.  Results are string hashed in large pieces
 * to minimize test output size.
 */

function encodeURISurrogatePairTest(is_uricomponent) {
    var x1, x2, x3;
    var lo, hi;
    var tmp = [];

    for (var i = 0; i < 0x100000; i++) {
        hi = 0xd800 + ((i >> 10) & 0x03ff);
        lo = 0xdc00 + (i & 0x03ff);

        tmp.push(String.fromCharCode(hi, lo));
        if (tmp.length < 65536) {
            continue;
        }

        x1 = tmp.join('');
        if (is_uricomponent) {
            x2 = g.encodeURIComponent(x1);
            x3 = g.decodeURIComponent(x2);
        } else {
            x2 = g.encodeURI(x1);
            x3 = g.decodeURI(x2);
        }
        print(checkSumString(x2));
        tmp = [];
    }

    if (tmp.length != 0) {
        throw new Error('internal error');
    }
}

try {
    print('encodeURI surrogate pairs');
    encodeURISurrogatePairTest(false);
    print('encodeURIComponent surrogate pairs');
    encodeURISurrogatePairTest(true);
} catch (e) {
    print(e.name, e);
}
/*===
===*/

/*
decodeURI
decodeURIComponent
*/

/* XXX: decoding bruteforce cases?  ASCII range decoding cases already
 * exist, what to test here?
 */
