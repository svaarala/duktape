// XXX: util
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

// XXX: util
function getCodePoints(x) {
    var res = [];
    var i, n;

    n = x.length;
    for (i = 0; i < n; i++) {
        res.push(x.charCodeAt(i));
    }

    return res.join(' ');
}

// indirect eval -> this is bound to the global object, E5 Section 10.4.2, step 1.a.
var g = (function () { var e = eval; return e('this'); } )();

var ucnybbles = [ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' ];
var lcnybbles = [ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' ];

/*===
basic
0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127 128 129 130 131 132 133 134 135 136 137 138 139 140 141 142 143 144 145 146 147 148 149 150 151 152 153 154 155 156 157 158 159 160 161 162 163 164 165 166 167 168 169 170 171 172 173 174 175 176 177 178 179 180 181 182 183 184 185 186 187 188 189 190 191 192 193 194 195 196 197 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 223 224 225 226 227 228 229 230 231 232 233 234 235 236 237 238 239 240 241 242 243 244 245 246 247 248 249 250 251 252 253 254 255
37 117 49 50 51
37 117 49 50 51 103
37 117 49 50 51 71
37 85 49 50 51 52
4671
4671
37 49
37 49 103
37 49 71
31
31
%x escapes
%u escapes
===*/

print('basic');

function basicTest() {
    var i;
    var tmp;

    // simple test of the 8-bit range
    tmp = [];
    for (i = 0; i < 256; i++) {
        tmp.push(String.fromCharCode(i));
    }
    tmp = tmp.join('');
    print(getCodePoints(g.unescape(tmp)));

    // % must be followed by 'u' and 4 hex digits to make a unicode escape
    print(getCodePoints(g.unescape('%u123')));
    print(getCodePoints(g.unescape('%u123g')));
    print(getCodePoints(g.unescape('%u123G')));
    print(getCodePoints(g.unescape('%U1234')));  // uppercase 'u' not accepted
    print(getCodePoints(g.unescape('%u123f')));  // valid
    print(getCodePoints(g.unescape('%u123F')));  // valid

    // % must be followed by 'x' and 2 hex digits to make a short escape
    print(getCodePoints(g.unescape('%1')));
    print(getCodePoints(g.unescape('%1g')));
    print(getCodePoints(g.unescape('%1G')));
    print(getCodePoints(g.unescape('%1f')));  // valid
    print(getCodePoints(g.unescape('%1F')));  // valid

    // check that all 2-byte escapes work
    print('%x escapes');
    for (i = 0; i < 256; i++) {
        tmp = '%' + ucnybbles[i >> 4] + ucnybbles[i & 0x0f];
        res = g.unescape(tmp);
        if (res.length !== 1 || res.charCodeAt(0) !== i) {
            print('unescape error for %xx at index i:', i);
        }

        tmp = '%' + lcnybbles[i >> 4] + lcnybbles[i & 0x0f];
        res = g.unescape(tmp);
        if (res.length !== 1 || res.charCodeAt(0) !== i) {
            print('unescape error for %xx at index i:', i);
        }
    }

    // check that all 4-byte escapes work
    print('%u escapes');
    for (i = 0; i < 65536; i++) {
        tmp = '%u' + ucnybbles[(i >> 12) & 0x0f] + ucnybbles[(i >> 8) & 0x0f] +
                     ucnybbles[(i >> 4) & 0x0f] + ucnybbles[i & 0x0f];
        res = g.unescape(tmp);
        if (res.length !== 1 || res.charCodeAt(0) !== i) {
            print('unescape error for %uxxxx at index i:', i);
        }

        tmp = '%u' + lcnybbles[(i >> 12) & 0x0f] + lcnybbles[(i >> 8) & 0x0f] +
                     lcnybbles[(i >> 4) & 0x0f] + lcnybbles[i & 0x0f];
        res = g.unescape(tmp);
        if (res.length !== 1 || res.charCodeAt(0) !== i) {
            print('unescape error for %uxxxx at index i:', i);
        }
    }
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
bruteforce
0 5759968
1024 17273824
2048 28787680
3072 40301536
4096 51815392
5120 63329248
6144 74843104
7168 86356960
8192 97870816
9216 109384672
10240 120898528
11264 132412384
12288 143926240
13312 155440096
14336 166953952
15360 178467808
16384 189981664
17408 201495520
18432 213009376
19456 224523232
20480 236037088
21504 247550944
22528 259064800
23552 270578656
24576 282092512
25600 293606368
26624 305120224
27648 316634080
28672 328147936
29696 339661792
30720 351175648
31744 362689504
32768 374203360
33792 385717216
34816 397231072
35840 408744928
36864 420258784
37888 431772640
38912 443286496
39936 454800352
40960 466314208
41984 477828064
43008 489341920
44032 500855776
45056 512369632
46080 523883488
47104 535397344
48128 546911200
49152 558425056
50176 569938912
51200 581452768
52224 592966624
53248 604480480
54272 615994336
55296 627508192
56320 639022048
57344 650535904
58368 662049760
59392 673563616
60416 685077472
61440 696591328
62464 708105184
63488 719619040
64512 731132896
===*/

print('bruteforce');

function bruteForceTest() {
    var i;
    var tmp;

    // decode every character (including '%', because the characters
    // following it won't make a valid escape, it will decode as itself)
    for (i = 0; i < 65536; i += 1024) {
        tmp = [];
        for (j = 0; j < 1024; j++) {
            tmp.push(String.fromCharCode(i + j));
        }
        tmp = tmp.join('');

        print(i, checkSumString(g.unescape(tmp)));
    }
}

try {
    bruteForceTest();
} catch (e) {
    print(e.name);
}
