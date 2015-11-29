/*
 *  Brute force hex decode test, try to cover all code paths and all
 *  lookup table indices.
 */

/*@include util-checksum-string.js@*/

/*===
||
||
2 0
3 691895120
4 1388149584
5 2080971256
6 2777225720
7 3471302228
8 4167556692
9 4861769996
10 5558024460
11 6252149588
12 6948404052
13 7643301688
14 8339556152
15 9035834464
16 9732088928
17 10430337428
18 11126591892
19 11824955276
20 12521209740
21 13220335336
22 13916589800
23 14616839420
24 15313093884
25 16015042652
26 16711297116
27 17413324600
28 18109579064
29 18811437060
30 19507691524
31 20210314284
32 20906568748
final 21610322440
success 7744 failure 2023872
0123456789abcdefABCDEF 0123456789abcdefabcdef
123456789abcdefABCDEF0 123456789abcdefabcdef0
23456789abcdefABCDEF01 23456789abcdefabcdef01
3456789abcdefABCDEF012 3456789abcdefabcdef012
456789abcdefABCDEF0123 456789abcdefabcdef0123
56789abcdefABCDEF01234 56789abcdefabcdef01234
6789abcdefABCDEF012345 6789abcdefabcdef012345
789abcdefABCDEF0123456 789abcdefabcdef0123456
89abcdefABCDEF01234567 89abcdefabcdef01234567
9abcdefABCDEF012345678 9abcdefabcdef012345678
abcdefABCDEF0123456789 abcdefabcdef0123456789
bcdefABCDEF0123456789a bcdefabcdef0123456789a
cdefABCDEF0123456789ab cdefabcdef0123456789ab
defABCDEF0123456789abc defabcdef0123456789abc
efABCDEF0123456789abcd efabcdef0123456789abcd
fABCDEF0123456789abcde fabcdef0123456789abcde
ABCDEF0123456789abcdef abcdef0123456789abcdef
BCDEF0123456789abcdefA bcdef0123456789abcdefa
CDEF0123456789abcdefAB cdef0123456789abcdefab
DEF0123456789abcdefABC def0123456789abcdefabc
EF0123456789abcdefABCD ef0123456789abcdefabcd
F0123456789abcdefABCDE f0123456789abcdefabcde
===*/

function testLengths() {
    var i, j, len;
    var res1, res2;
    var csum = 0;
    var valid = '0123456789abcdefABCDEF';
    var nybbles = '0123456789abcdef';
    var success = 0, failure = 0;
    var tmp, res;

    print(Duktape.enc('jx', Duktape.dec('hex', Duktape.Buffer(''))));
    print(Duktape.enc('jx', Duktape.dec('hex', '')));

    for (len = 2; len <= 32; len++) {
        print(len, csum);
        buf = Duktape.Buffer(len);
        for(i = 0; i < len; i++) {
            buf[i] = valid.charCodeAt(i % valid.length);
        }

        for (i = 0; i < 65536; i++) {
            buf[len - 2] = i >>> 8;
            buf[len - 1] = i & 0xff;

            try {
                res1 = Duktape.enc('jx', Duktape.dec('hex', buf));
                success++;
            } catch (e) {
                res1 = e.name;
                failure++;
            }
            try {
                res2 = Duktape.enc('jx', Duktape.dec('hex', String(buf)));
            } catch (e) {
                res2 = e.name;
            }

            if (res1 !== res2) { throw new Error('encode results differ'); }

            csum += checksumString(res1);
        }
    }

    print('final', csum);
    print('success', success, 'failure', failure);

    // Scrolling 22-char decode window, two 8-char fast loops + 6 leftover
    // chars.  March all valid characters through all positions.

    for (i = 0; i < valid.length; i++) {
        tmp = '';
        for (j = 0; j < 22; j++) {
            tmp += valid[(i + j) % valid.length];
        }
        buf = Duktape.dec('hex', tmp);
        res = [];
        for (j = 0; j < buf.length; j++) {
            res.push(nybbles[buf[j] >>> 4]);
            res.push(nybbles[buf[j] & 0x0f]);
        }
        res = res.join('');
        print(tmp, res);
    }
}

try {
    testLengths();
} catch (e) {
    print(e.stack || e);
}
