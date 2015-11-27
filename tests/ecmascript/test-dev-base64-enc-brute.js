/*
 *  Brute force base64 encode test, try to cover all code paths and all
 *  lookup table indices.
 */

/*@include util-checksum-string.js@*/

/*===
AA==
AQ==
Ag==
Aw==
BA==
BQ==
Bg==
Bw==
CA==
CQ==
Cg==
Cw==
DA==
DQ==
Dg==
Dw==
EA==
EQ==
Eg==
Ew==
FA==
FQ==
Fg==
Fw==
GA==
GQ==
Gg==
Gw==
HA==
HQ==
Hg==
Hw==
IA==
IQ==
Ig==
Iw==
JA==
JQ==
Jg==
Jw==
KA==
KQ==
Kg==
Kw==
LA==
LQ==
Lg==
Lw==
MA==
MQ==
Mg==
Mw==
NA==
NQ==
Ng==
Nw==
OA==
OQ==
Og==
Ow==
PA==
PQ==
Pg==
Pw==
QA==
QQ==
Qg==
Qw==
RA==
RQ==
Rg==
Rw==
SA==
SQ==
Sg==
Sw==
TA==
TQ==
Tg==
Tw==
UA==
UQ==
Ug==
Uw==
VA==
VQ==
Vg==
Vw==
WA==
WQ==
Wg==
Ww==
XA==
XQ==
Xg==
Xw==
YA==
YQ==
Yg==
Yw==
ZA==
ZQ==
Zg==
Zw==
aA==
aQ==
ag==
aw==
bA==
bQ==
bg==
bw==
cA==
cQ==
cg==
cw==
dA==
dQ==
dg==
dw==
eA==
eQ==
eg==
ew==
fA==
fQ==
fg==
fw==
gA==
gQ==
gg==
gw==
hA==
hQ==
hg==
hw==
iA==
iQ==
ig==
iw==
jA==
jQ==
jg==
jw==
kA==
kQ==
kg==
kw==
lA==
lQ==
lg==
lw==
mA==
mQ==
mg==
mw==
nA==
nQ==
ng==
nw==
oA==
oQ==
og==
ow==
pA==
pQ==
pg==
pw==
qA==
qQ==
qg==
qw==
rA==
rQ==
rg==
rw==
sA==
sQ==
sg==
sw==
tA==
tQ==
tg==
tw==
uA==
uQ==
ug==
uw==
vA==
vQ==
vg==
vw==
wA==
wQ==
wg==
ww==
xA==
xQ==
xg==
xw==
yA==
yQ==
yg==
yw==
zA==
zQ==
zg==
zw==
0A==
0Q==
0g==
0w==
1A==
1Q==
1g==
1w==
2A==
2Q==
2g==
2w==
3A==
3Q==
3g==
3w==
4A==
4Q==
4g==
4w==
5A==
5Q==
5g==
5w==
6A==
6Q==
6g==
6w==
7A==
7Q==
7g==
7w==
8A==
8Q==
8g==
8w==
9A==
9Q==
9g==
9w==
+A==
+Q==
+g==
+w==
/A==
/Q==
/g==
/w==
0
0
1
279956
2
4875712
3
10604800
4
90295040
5
184050176
6
366344192
7
1017657344
8
1905774592
1
2
4
8
15
27
49
88
158
283
507
908
1626
2911
5211
9328
16698
29890
53504
95773
171434
306867
549292
983233
1759988
3150379
5639179
11742480113
===*/

function test() {
    var buf;
    var i, len;
    var csum;

    for (i = 0; i < 256; i++) {
        buf = Duktape.Buffer(1); buf[0] = i;
        print(Duktape.enc('base64', buf));
    }

    for (len = 0; len <= 8; len++) {
        print(len);
        buf = Duktape.Buffer(len);

        csum = 0;
        for (;;) {
            csum += checksumString(Duktape.enc('base64', buf));

            // Full coverage for last byte, sparse coverage for others.
            if (len == 0) { break; }
            buf[len - 1]++;
            if (buf[len - 1] == 0x00) {
                for (i = len - 2; i >= 0; i--) {
                    if (buf[i] == 0xff) {
                        buf[i] = 0x00;
                    } else if (i == len - 2) {
                        buf[i] += 0x11;
                        break;
                    } else {
                        buf[i] += 0xff;  // just cycle 0x00, 0xff
                        break;
                    }
                }
                if (i < 0) { break; }
            }
        }
        print(csum);
    }

    csum = 0;
    for (len = 1; len < 8 * 1024 * 1024; len = Math.floor(len * 1.79 + 1)) {
        print(len);
        buf = Duktape.Buffer(len);
        for (i = 0; i < len; i++) { buf[i] = i; }
        csum += checksumString(Duktape.enc('base64', buf));
    }
    print(csum);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
