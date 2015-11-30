/*
 *  JX hex encode fast path coverage
 *
 *  The fast path inner loop assumes an align-by-2 output pointer and makes a
 *  temporary adjustment followed by memmove() if the output is not aligned.
 *  Test both alignments.
 */

/*===
0 {foo:||} {foox:||}
1 {foo:|00|} {foox:|00|}
2 {foo:|0055|} {foox:|0055|}
3 {foo:|0055aa|} {foox:|0055aa|}
4 {foo:|0055aaff|} {foox:|0055aaff|}
5 {foo:|0055aaff54|} {foox:|0055aaff54|}
6 {foo:|0055aaff54a9|} {foox:|0055aaff54a9|}
7 {foo:|0055aaff54a9fe|} {foox:|0055aaff54a9fe|}
8 {foo:|0055aaff54a9fe53|} {foox:|0055aaff54a9fe53|}
9 {foo:|0055aaff54a9fe53a8|} {foox:|0055aaff54a9fe53a8|}
10 {foo:|0055aaff54a9fe53a8fd|} {foox:|0055aaff54a9fe53a8fd|}
11 {foo:|0055aaff54a9fe53a8fd52|} {foox:|0055aaff54a9fe53a8fd52|}
12 {foo:|0055aaff54a9fe53a8fd52a7|} {foox:|0055aaff54a9fe53a8fd52a7|}
13 {foo:|0055aaff54a9fe53a8fd52a7fc|} {foox:|0055aaff54a9fe53a8fd52a7fc|}
14 {foo:|0055aaff54a9fe53a8fd52a7fc51|} {foox:|0055aaff54a9fe53a8fd52a7fc51|}
15 {foo:|0055aaff54a9fe53a8fd52a7fc51a6|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6|}
16 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb|}
17 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50|}
18 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5|}
19 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa|}
20 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4f|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4f|}
21 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4|}
22 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f9|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f9|}
23 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94e|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94e|}
24 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3|}
25 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f8|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f8|}
26 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84d|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84d|}
27 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2|}
28 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f7|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f7|}
29 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74c|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74c|}
30 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1|}
31 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f6|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f6|}
32 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64b|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64b|}
33 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0|}
34 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f5|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f5|}
35 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a|}
36 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9f|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9f|}
37 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4|}
38 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff449|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff449|}
39 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499e|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499e|}
40 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3|}
41 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef348|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef348|}
42 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489d|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489d|}
43 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2|}
44 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df247|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df247|}
45 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479c|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479c|}
46 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1|}
47 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf146|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf146|}
48 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469b|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469b|}
49 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0|}
50 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf045|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf045|}
51 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459a|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459a|}
52 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef|}
53 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef44|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef44|}
54 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499|}
55 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee|}
56 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee43|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee43|}
57 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee4398|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee4398|}
58 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee4398ed|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee4398ed|}
59 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee4398ed42|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee4398ed42|}
60 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee4398ed4297|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee4398ed4297|}
61 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee4398ed4297ec|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee4398ed4297ec|}
62 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee4398ed4297ec41|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee4398ed4297ec41|}
63 {foo:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee4398ed4297ec4196|} {foox:|0055aaff54a9fe53a8fd52a7fc51a6fb50a5fa4fa4f94ea3f84da2f74ca1f64ba0f54a9ff4499ef3489df2479cf1469bf0459aef4499ee4398ed4297ec4196|}
0 {foo:|000102030405060708090a|} {foox:|000102030405060708090a|}
1 {foo:|0102030405060708090a0b|} {foox:|0102030405060708090a0b|}
2 {foo:|02030405060708090a0b0c|} {foox:|02030405060708090a0b0c|}
3 {foo:|030405060708090a0b0c0d|} {foox:|030405060708090a0b0c0d|}
4 {foo:|0405060708090a0b0c0d0e|} {foox:|0405060708090a0b0c0d0e|}
5 {foo:|05060708090a0b0c0d0e0f|} {foox:|05060708090a0b0c0d0e0f|}
6 {foo:|060708090a0b0c0d0e0f10|} {foox:|060708090a0b0c0d0e0f10|}
7 {foo:|0708090a0b0c0d0e0f1011|} {foox:|0708090a0b0c0d0e0f1011|}
8 {foo:|08090a0b0c0d0e0f101112|} {foox:|08090a0b0c0d0e0f101112|}
9 {foo:|090a0b0c0d0e0f10111213|} {foox:|090a0b0c0d0e0f10111213|}
10 {foo:|0a0b0c0d0e0f1011121314|} {foox:|0a0b0c0d0e0f1011121314|}
11 {foo:|0b0c0d0e0f101112131415|} {foox:|0b0c0d0e0f101112131415|}
12 {foo:|0c0d0e0f10111213141516|} {foox:|0c0d0e0f10111213141516|}
13 {foo:|0d0e0f1011121314151617|} {foox:|0d0e0f1011121314151617|}
14 {foo:|0e0f101112131415161718|} {foox:|0e0f101112131415161718|}
15 {foo:|0f10111213141516171819|} {foox:|0f10111213141516171819|}
16 {foo:|101112131415161718191a|} {foox:|101112131415161718191a|}
17 {foo:|1112131415161718191a1b|} {foox:|1112131415161718191a1b|}
18 {foo:|12131415161718191a1b1c|} {foox:|12131415161718191a1b1c|}
19 {foo:|131415161718191a1b1c1d|} {foox:|131415161718191a1b1c1d|}
20 {foo:|1415161718191a1b1c1d1e|} {foox:|1415161718191a1b1c1d1e|}
21 {foo:|15161718191a1b1c1d1e1f|} {foox:|15161718191a1b1c1d1e1f|}
22 {foo:|161718191a1b1c1d1e1f20|} {foox:|161718191a1b1c1d1e1f20|}
23 {foo:|1718191a1b1c1d1e1f2021|} {foox:|1718191a1b1c1d1e1f2021|}
24 {foo:|18191a1b1c1d1e1f202122|} {foox:|18191a1b1c1d1e1f202122|}
25 {foo:|191a1b1c1d1e1f20212223|} {foox:|191a1b1c1d1e1f20212223|}
26 {foo:|1a1b1c1d1e1f2021222324|} {foox:|1a1b1c1d1e1f2021222324|}
27 {foo:|1b1c1d1e1f202122232425|} {foox:|1b1c1d1e1f202122232425|}
28 {foo:|1c1d1e1f20212223242526|} {foox:|1c1d1e1f20212223242526|}
29 {foo:|1d1e1f2021222324252627|} {foox:|1d1e1f2021222324252627|}
30 {foo:|1e1f202122232425262728|} {foox:|1e1f202122232425262728|}
31 {foo:|1f20212223242526272829|} {foox:|1f20212223242526272829|}
32 {foo:|202122232425262728292a|} {foox:|202122232425262728292a|}
33 {foo:|2122232425262728292a2b|} {foox:|2122232425262728292a2b|}
34 {foo:|22232425262728292a2b2c|} {foox:|22232425262728292a2b2c|}
35 {foo:|232425262728292a2b2c2d|} {foox:|232425262728292a2b2c2d|}
36 {foo:|2425262728292a2b2c2d2e|} {foox:|2425262728292a2b2c2d2e|}
37 {foo:|25262728292a2b2c2d2e2f|} {foox:|25262728292a2b2c2d2e2f|}
38 {foo:|262728292a2b2c2d2e2f30|} {foox:|262728292a2b2c2d2e2f30|}
39 {foo:|2728292a2b2c2d2e2f3031|} {foox:|2728292a2b2c2d2e2f3031|}
40 {foo:|28292a2b2c2d2e2f303132|} {foox:|28292a2b2c2d2e2f303132|}
41 {foo:|292a2b2c2d2e2f30313233|} {foox:|292a2b2c2d2e2f30313233|}
42 {foo:|2a2b2c2d2e2f3031323334|} {foox:|2a2b2c2d2e2f3031323334|}
43 {foo:|2b2c2d2e2f303132333435|} {foox:|2b2c2d2e2f303132333435|}
44 {foo:|2c2d2e2f30313233343536|} {foox:|2c2d2e2f30313233343536|}
45 {foo:|2d2e2f3031323334353637|} {foox:|2d2e2f3031323334353637|}
46 {foo:|2e2f303132333435363738|} {foox:|2e2f303132333435363738|}
47 {foo:|2f30313233343536373839|} {foox:|2f30313233343536373839|}
48 {foo:|303132333435363738393a|} {foox:|303132333435363738393a|}
49 {foo:|3132333435363738393a3b|} {foox:|3132333435363738393a3b|}
50 {foo:|32333435363738393a3b3c|} {foox:|32333435363738393a3b3c|}
51 {foo:|333435363738393a3b3c3d|} {foox:|333435363738393a3b3c3d|}
52 {foo:|3435363738393a3b3c3d3e|} {foox:|3435363738393a3b3c3d3e|}
53 {foo:|35363738393a3b3c3d3e3f|} {foox:|35363738393a3b3c3d3e3f|}
54 {foo:|363738393a3b3c3d3e3f40|} {foox:|363738393a3b3c3d3e3f40|}
55 {foo:|3738393a3b3c3d3e3f4041|} {foox:|3738393a3b3c3d3e3f4041|}
56 {foo:|38393a3b3c3d3e3f404142|} {foox:|38393a3b3c3d3e3f404142|}
57 {foo:|393a3b3c3d3e3f40414243|} {foox:|393a3b3c3d3e3f40414243|}
58 {foo:|3a3b3c3d3e3f4041424344|} {foox:|3a3b3c3d3e3f4041424344|}
59 {foo:|3b3c3d3e3f404142434445|} {foox:|3b3c3d3e3f404142434445|}
60 {foo:|3c3d3e3f40414243444546|} {foox:|3c3d3e3f40414243444546|}
61 {foo:|3d3e3f4041424344454647|} {foox:|3d3e3f4041424344454647|}
62 {foo:|3e3f404142434445464748|} {foox:|3e3f404142434445464748|}
63 {foo:|3f40414243444546474849|} {foox:|3f40414243444546474849|}
64 {foo:|404142434445464748494a|} {foox:|404142434445464748494a|}
65 {foo:|4142434445464748494a4b|} {foox:|4142434445464748494a4b|}
66 {foo:|42434445464748494a4b4c|} {foox:|42434445464748494a4b4c|}
67 {foo:|434445464748494a4b4c4d|} {foox:|434445464748494a4b4c4d|}
68 {foo:|4445464748494a4b4c4d4e|} {foox:|4445464748494a4b4c4d4e|}
69 {foo:|45464748494a4b4c4d4e4f|} {foox:|45464748494a4b4c4d4e4f|}
70 {foo:|464748494a4b4c4d4e4f50|} {foox:|464748494a4b4c4d4e4f50|}
71 {foo:|4748494a4b4c4d4e4f5051|} {foox:|4748494a4b4c4d4e4f5051|}
72 {foo:|48494a4b4c4d4e4f505152|} {foox:|48494a4b4c4d4e4f505152|}
73 {foo:|494a4b4c4d4e4f50515253|} {foox:|494a4b4c4d4e4f50515253|}
74 {foo:|4a4b4c4d4e4f5051525354|} {foox:|4a4b4c4d4e4f5051525354|}
75 {foo:|4b4c4d4e4f505152535455|} {foox:|4b4c4d4e4f505152535455|}
76 {foo:|4c4d4e4f50515253545556|} {foox:|4c4d4e4f50515253545556|}
77 {foo:|4d4e4f5051525354555657|} {foox:|4d4e4f5051525354555657|}
78 {foo:|4e4f505152535455565758|} {foox:|4e4f505152535455565758|}
79 {foo:|4f50515253545556575859|} {foox:|4f50515253545556575859|}
80 {foo:|505152535455565758595a|} {foox:|505152535455565758595a|}
81 {foo:|5152535455565758595a5b|} {foox:|5152535455565758595a5b|}
82 {foo:|52535455565758595a5b5c|} {foox:|52535455565758595a5b5c|}
83 {foo:|535455565758595a5b5c5d|} {foox:|535455565758595a5b5c5d|}
84 {foo:|5455565758595a5b5c5d5e|} {foox:|5455565758595a5b5c5d5e|}
85 {foo:|55565758595a5b5c5d5e5f|} {foox:|55565758595a5b5c5d5e5f|}
86 {foo:|565758595a5b5c5d5e5f60|} {foox:|565758595a5b5c5d5e5f60|}
87 {foo:|5758595a5b5c5d5e5f6061|} {foox:|5758595a5b5c5d5e5f6061|}
88 {foo:|58595a5b5c5d5e5f606162|} {foox:|58595a5b5c5d5e5f606162|}
89 {foo:|595a5b5c5d5e5f60616263|} {foox:|595a5b5c5d5e5f60616263|}
90 {foo:|5a5b5c5d5e5f6061626364|} {foox:|5a5b5c5d5e5f6061626364|}
91 {foo:|5b5c5d5e5f606162636465|} {foox:|5b5c5d5e5f606162636465|}
92 {foo:|5c5d5e5f60616263646566|} {foox:|5c5d5e5f60616263646566|}
93 {foo:|5d5e5f6061626364656667|} {foox:|5d5e5f6061626364656667|}
94 {foo:|5e5f606162636465666768|} {foox:|5e5f606162636465666768|}
95 {foo:|5f60616263646566676869|} {foox:|5f60616263646566676869|}
96 {foo:|606162636465666768696a|} {foox:|606162636465666768696a|}
97 {foo:|6162636465666768696a6b|} {foox:|6162636465666768696a6b|}
98 {foo:|62636465666768696a6b6c|} {foox:|62636465666768696a6b6c|}
99 {foo:|636465666768696a6b6c6d|} {foox:|636465666768696a6b6c6d|}
100 {foo:|6465666768696a6b6c6d6e|} {foox:|6465666768696a6b6c6d6e|}
101 {foo:|65666768696a6b6c6d6e6f|} {foox:|65666768696a6b6c6d6e6f|}
102 {foo:|666768696a6b6c6d6e6f70|} {foox:|666768696a6b6c6d6e6f70|}
103 {foo:|6768696a6b6c6d6e6f7071|} {foox:|6768696a6b6c6d6e6f7071|}
104 {foo:|68696a6b6c6d6e6f707172|} {foox:|68696a6b6c6d6e6f707172|}
105 {foo:|696a6b6c6d6e6f70717273|} {foox:|696a6b6c6d6e6f70717273|}
106 {foo:|6a6b6c6d6e6f7071727374|} {foox:|6a6b6c6d6e6f7071727374|}
107 {foo:|6b6c6d6e6f707172737475|} {foox:|6b6c6d6e6f707172737475|}
108 {foo:|6c6d6e6f70717273747576|} {foox:|6c6d6e6f70717273747576|}
109 {foo:|6d6e6f7071727374757677|} {foox:|6d6e6f7071727374757677|}
110 {foo:|6e6f707172737475767778|} {foox:|6e6f707172737475767778|}
111 {foo:|6f70717273747576777879|} {foox:|6f70717273747576777879|}
112 {foo:|707172737475767778797a|} {foox:|707172737475767778797a|}
113 {foo:|7172737475767778797a7b|} {foox:|7172737475767778797a7b|}
114 {foo:|72737475767778797a7b7c|} {foox:|72737475767778797a7b7c|}
115 {foo:|737475767778797a7b7c7d|} {foox:|737475767778797a7b7c7d|}
116 {foo:|7475767778797a7b7c7d7e|} {foox:|7475767778797a7b7c7d7e|}
117 {foo:|75767778797a7b7c7d7e7f|} {foox:|75767778797a7b7c7d7e7f|}
118 {foo:|767778797a7b7c7d7e7f80|} {foox:|767778797a7b7c7d7e7f80|}
119 {foo:|7778797a7b7c7d7e7f8081|} {foox:|7778797a7b7c7d7e7f8081|}
120 {foo:|78797a7b7c7d7e7f808182|} {foox:|78797a7b7c7d7e7f808182|}
121 {foo:|797a7b7c7d7e7f80818283|} {foox:|797a7b7c7d7e7f80818283|}
122 {foo:|7a7b7c7d7e7f8081828384|} {foox:|7a7b7c7d7e7f8081828384|}
123 {foo:|7b7c7d7e7f808182838485|} {foox:|7b7c7d7e7f808182838485|}
124 {foo:|7c7d7e7f80818283848586|} {foox:|7c7d7e7f80818283848586|}
125 {foo:|7d7e7f8081828384858687|} {foox:|7d7e7f8081828384858687|}
126 {foo:|7e7f808182838485868788|} {foox:|7e7f808182838485868788|}
127 {foo:|7f80818283848586878889|} {foox:|7f80818283848586878889|}
128 {foo:|808182838485868788898a|} {foox:|808182838485868788898a|}
129 {foo:|8182838485868788898a8b|} {foox:|8182838485868788898a8b|}
130 {foo:|82838485868788898a8b8c|} {foox:|82838485868788898a8b8c|}
131 {foo:|838485868788898a8b8c8d|} {foox:|838485868788898a8b8c8d|}
132 {foo:|8485868788898a8b8c8d8e|} {foox:|8485868788898a8b8c8d8e|}
133 {foo:|85868788898a8b8c8d8e8f|} {foox:|85868788898a8b8c8d8e8f|}
134 {foo:|868788898a8b8c8d8e8f90|} {foox:|868788898a8b8c8d8e8f90|}
135 {foo:|8788898a8b8c8d8e8f9091|} {foox:|8788898a8b8c8d8e8f9091|}
136 {foo:|88898a8b8c8d8e8f909192|} {foox:|88898a8b8c8d8e8f909192|}
137 {foo:|898a8b8c8d8e8f90919293|} {foox:|898a8b8c8d8e8f90919293|}
138 {foo:|8a8b8c8d8e8f9091929394|} {foox:|8a8b8c8d8e8f9091929394|}
139 {foo:|8b8c8d8e8f909192939495|} {foox:|8b8c8d8e8f909192939495|}
140 {foo:|8c8d8e8f90919293949596|} {foox:|8c8d8e8f90919293949596|}
141 {foo:|8d8e8f9091929394959697|} {foox:|8d8e8f9091929394959697|}
142 {foo:|8e8f909192939495969798|} {foox:|8e8f909192939495969798|}
143 {foo:|8f90919293949596979899|} {foox:|8f90919293949596979899|}
144 {foo:|909192939495969798999a|} {foox:|909192939495969798999a|}
145 {foo:|9192939495969798999a9b|} {foox:|9192939495969798999a9b|}
146 {foo:|92939495969798999a9b9c|} {foox:|92939495969798999a9b9c|}
147 {foo:|939495969798999a9b9c9d|} {foox:|939495969798999a9b9c9d|}
148 {foo:|9495969798999a9b9c9d9e|} {foox:|9495969798999a9b9c9d9e|}
149 {foo:|95969798999a9b9c9d9e9f|} {foox:|95969798999a9b9c9d9e9f|}
150 {foo:|969798999a9b9c9d9e9fa0|} {foox:|969798999a9b9c9d9e9fa0|}
151 {foo:|9798999a9b9c9d9e9fa0a1|} {foox:|9798999a9b9c9d9e9fa0a1|}
152 {foo:|98999a9b9c9d9e9fa0a1a2|} {foox:|98999a9b9c9d9e9fa0a1a2|}
153 {foo:|999a9b9c9d9e9fa0a1a2a3|} {foox:|999a9b9c9d9e9fa0a1a2a3|}
154 {foo:|9a9b9c9d9e9fa0a1a2a3a4|} {foox:|9a9b9c9d9e9fa0a1a2a3a4|}
155 {foo:|9b9c9d9e9fa0a1a2a3a4a5|} {foox:|9b9c9d9e9fa0a1a2a3a4a5|}
156 {foo:|9c9d9e9fa0a1a2a3a4a5a6|} {foox:|9c9d9e9fa0a1a2a3a4a5a6|}
157 {foo:|9d9e9fa0a1a2a3a4a5a6a7|} {foox:|9d9e9fa0a1a2a3a4a5a6a7|}
158 {foo:|9e9fa0a1a2a3a4a5a6a7a8|} {foox:|9e9fa0a1a2a3a4a5a6a7a8|}
159 {foo:|9fa0a1a2a3a4a5a6a7a8a9|} {foox:|9fa0a1a2a3a4a5a6a7a8a9|}
160 {foo:|a0a1a2a3a4a5a6a7a8a9aa|} {foox:|a0a1a2a3a4a5a6a7a8a9aa|}
161 {foo:|a1a2a3a4a5a6a7a8a9aaab|} {foox:|a1a2a3a4a5a6a7a8a9aaab|}
162 {foo:|a2a3a4a5a6a7a8a9aaabac|} {foox:|a2a3a4a5a6a7a8a9aaabac|}
163 {foo:|a3a4a5a6a7a8a9aaabacad|} {foox:|a3a4a5a6a7a8a9aaabacad|}
164 {foo:|a4a5a6a7a8a9aaabacadae|} {foox:|a4a5a6a7a8a9aaabacadae|}
165 {foo:|a5a6a7a8a9aaabacadaeaf|} {foox:|a5a6a7a8a9aaabacadaeaf|}
166 {foo:|a6a7a8a9aaabacadaeafb0|} {foox:|a6a7a8a9aaabacadaeafb0|}
167 {foo:|a7a8a9aaabacadaeafb0b1|} {foox:|a7a8a9aaabacadaeafb0b1|}
168 {foo:|a8a9aaabacadaeafb0b1b2|} {foox:|a8a9aaabacadaeafb0b1b2|}
169 {foo:|a9aaabacadaeafb0b1b2b3|} {foox:|a9aaabacadaeafb0b1b2b3|}
170 {foo:|aaabacadaeafb0b1b2b3b4|} {foox:|aaabacadaeafb0b1b2b3b4|}
171 {foo:|abacadaeafb0b1b2b3b4b5|} {foox:|abacadaeafb0b1b2b3b4b5|}
172 {foo:|acadaeafb0b1b2b3b4b5b6|} {foox:|acadaeafb0b1b2b3b4b5b6|}
173 {foo:|adaeafb0b1b2b3b4b5b6b7|} {foox:|adaeafb0b1b2b3b4b5b6b7|}
174 {foo:|aeafb0b1b2b3b4b5b6b7b8|} {foox:|aeafb0b1b2b3b4b5b6b7b8|}
175 {foo:|afb0b1b2b3b4b5b6b7b8b9|} {foox:|afb0b1b2b3b4b5b6b7b8b9|}
176 {foo:|b0b1b2b3b4b5b6b7b8b9ba|} {foox:|b0b1b2b3b4b5b6b7b8b9ba|}
177 {foo:|b1b2b3b4b5b6b7b8b9babb|} {foox:|b1b2b3b4b5b6b7b8b9babb|}
178 {foo:|b2b3b4b5b6b7b8b9babbbc|} {foox:|b2b3b4b5b6b7b8b9babbbc|}
179 {foo:|b3b4b5b6b7b8b9babbbcbd|} {foox:|b3b4b5b6b7b8b9babbbcbd|}
180 {foo:|b4b5b6b7b8b9babbbcbdbe|} {foox:|b4b5b6b7b8b9babbbcbdbe|}
181 {foo:|b5b6b7b8b9babbbcbdbebf|} {foox:|b5b6b7b8b9babbbcbdbebf|}
182 {foo:|b6b7b8b9babbbcbdbebfc0|} {foox:|b6b7b8b9babbbcbdbebfc0|}
183 {foo:|b7b8b9babbbcbdbebfc0c1|} {foox:|b7b8b9babbbcbdbebfc0c1|}
184 {foo:|b8b9babbbcbdbebfc0c1c2|} {foox:|b8b9babbbcbdbebfc0c1c2|}
185 {foo:|b9babbbcbdbebfc0c1c2c3|} {foox:|b9babbbcbdbebfc0c1c2c3|}
186 {foo:|babbbcbdbebfc0c1c2c3c4|} {foox:|babbbcbdbebfc0c1c2c3c4|}
187 {foo:|bbbcbdbebfc0c1c2c3c4c5|} {foox:|bbbcbdbebfc0c1c2c3c4c5|}
188 {foo:|bcbdbebfc0c1c2c3c4c5c6|} {foox:|bcbdbebfc0c1c2c3c4c5c6|}
189 {foo:|bdbebfc0c1c2c3c4c5c6c7|} {foox:|bdbebfc0c1c2c3c4c5c6c7|}
190 {foo:|bebfc0c1c2c3c4c5c6c7c8|} {foox:|bebfc0c1c2c3c4c5c6c7c8|}
191 {foo:|bfc0c1c2c3c4c5c6c7c8c9|} {foox:|bfc0c1c2c3c4c5c6c7c8c9|}
192 {foo:|c0c1c2c3c4c5c6c7c8c9ca|} {foox:|c0c1c2c3c4c5c6c7c8c9ca|}
193 {foo:|c1c2c3c4c5c6c7c8c9cacb|} {foox:|c1c2c3c4c5c6c7c8c9cacb|}
194 {foo:|c2c3c4c5c6c7c8c9cacbcc|} {foox:|c2c3c4c5c6c7c8c9cacbcc|}
195 {foo:|c3c4c5c6c7c8c9cacbcccd|} {foox:|c3c4c5c6c7c8c9cacbcccd|}
196 {foo:|c4c5c6c7c8c9cacbcccdce|} {foox:|c4c5c6c7c8c9cacbcccdce|}
197 {foo:|c5c6c7c8c9cacbcccdcecf|} {foox:|c5c6c7c8c9cacbcccdcecf|}
198 {foo:|c6c7c8c9cacbcccdcecfd0|} {foox:|c6c7c8c9cacbcccdcecfd0|}
199 {foo:|c7c8c9cacbcccdcecfd0d1|} {foox:|c7c8c9cacbcccdcecfd0d1|}
200 {foo:|c8c9cacbcccdcecfd0d1d2|} {foox:|c8c9cacbcccdcecfd0d1d2|}
201 {foo:|c9cacbcccdcecfd0d1d2d3|} {foox:|c9cacbcccdcecfd0d1d2d3|}
202 {foo:|cacbcccdcecfd0d1d2d3d4|} {foox:|cacbcccdcecfd0d1d2d3d4|}
203 {foo:|cbcccdcecfd0d1d2d3d4d5|} {foox:|cbcccdcecfd0d1d2d3d4d5|}
204 {foo:|cccdcecfd0d1d2d3d4d5d6|} {foox:|cccdcecfd0d1d2d3d4d5d6|}
205 {foo:|cdcecfd0d1d2d3d4d5d6d7|} {foox:|cdcecfd0d1d2d3d4d5d6d7|}
206 {foo:|cecfd0d1d2d3d4d5d6d7d8|} {foox:|cecfd0d1d2d3d4d5d6d7d8|}
207 {foo:|cfd0d1d2d3d4d5d6d7d8d9|} {foox:|cfd0d1d2d3d4d5d6d7d8d9|}
208 {foo:|d0d1d2d3d4d5d6d7d8d9da|} {foox:|d0d1d2d3d4d5d6d7d8d9da|}
209 {foo:|d1d2d3d4d5d6d7d8d9dadb|} {foox:|d1d2d3d4d5d6d7d8d9dadb|}
210 {foo:|d2d3d4d5d6d7d8d9dadbdc|} {foox:|d2d3d4d5d6d7d8d9dadbdc|}
211 {foo:|d3d4d5d6d7d8d9dadbdcdd|} {foox:|d3d4d5d6d7d8d9dadbdcdd|}
212 {foo:|d4d5d6d7d8d9dadbdcddde|} {foox:|d4d5d6d7d8d9dadbdcddde|}
213 {foo:|d5d6d7d8d9dadbdcdddedf|} {foox:|d5d6d7d8d9dadbdcdddedf|}
214 {foo:|d6d7d8d9dadbdcdddedfe0|} {foox:|d6d7d8d9dadbdcdddedfe0|}
215 {foo:|d7d8d9dadbdcdddedfe0e1|} {foox:|d7d8d9dadbdcdddedfe0e1|}
216 {foo:|d8d9dadbdcdddedfe0e1e2|} {foox:|d8d9dadbdcdddedfe0e1e2|}
217 {foo:|d9dadbdcdddedfe0e1e2e3|} {foox:|d9dadbdcdddedfe0e1e2e3|}
218 {foo:|dadbdcdddedfe0e1e2e3e4|} {foox:|dadbdcdddedfe0e1e2e3e4|}
219 {foo:|dbdcdddedfe0e1e2e3e4e5|} {foox:|dbdcdddedfe0e1e2e3e4e5|}
220 {foo:|dcdddedfe0e1e2e3e4e5e6|} {foox:|dcdddedfe0e1e2e3e4e5e6|}
221 {foo:|dddedfe0e1e2e3e4e5e6e7|} {foox:|dddedfe0e1e2e3e4e5e6e7|}
222 {foo:|dedfe0e1e2e3e4e5e6e7e8|} {foox:|dedfe0e1e2e3e4e5e6e7e8|}
223 {foo:|dfe0e1e2e3e4e5e6e7e8e9|} {foox:|dfe0e1e2e3e4e5e6e7e8e9|}
224 {foo:|e0e1e2e3e4e5e6e7e8e9ea|} {foox:|e0e1e2e3e4e5e6e7e8e9ea|}
225 {foo:|e1e2e3e4e5e6e7e8e9eaeb|} {foox:|e1e2e3e4e5e6e7e8e9eaeb|}
226 {foo:|e2e3e4e5e6e7e8e9eaebec|} {foox:|e2e3e4e5e6e7e8e9eaebec|}
227 {foo:|e3e4e5e6e7e8e9eaebeced|} {foox:|e3e4e5e6e7e8e9eaebeced|}
228 {foo:|e4e5e6e7e8e9eaebecedee|} {foox:|e4e5e6e7e8e9eaebecedee|}
229 {foo:|e5e6e7e8e9eaebecedeeef|} {foox:|e5e6e7e8e9eaebecedeeef|}
230 {foo:|e6e7e8e9eaebecedeeeff0|} {foox:|e6e7e8e9eaebecedeeeff0|}
231 {foo:|e7e8e9eaebecedeeeff0f1|} {foox:|e7e8e9eaebecedeeeff0f1|}
232 {foo:|e8e9eaebecedeeeff0f1f2|} {foox:|e8e9eaebecedeeeff0f1f2|}
233 {foo:|e9eaebecedeeeff0f1f2f3|} {foox:|e9eaebecedeeeff0f1f2f3|}
234 {foo:|eaebecedeeeff0f1f2f3f4|} {foox:|eaebecedeeeff0f1f2f3f4|}
235 {foo:|ebecedeeeff0f1f2f3f4f5|} {foox:|ebecedeeeff0f1f2f3f4f5|}
236 {foo:|ecedeeeff0f1f2f3f4f5f6|} {foox:|ecedeeeff0f1f2f3f4f5f6|}
237 {foo:|edeeeff0f1f2f3f4f5f6f7|} {foox:|edeeeff0f1f2f3f4f5f6f7|}
238 {foo:|eeeff0f1f2f3f4f5f6f7f8|} {foox:|eeeff0f1f2f3f4f5f6f7f8|}
239 {foo:|eff0f1f2f3f4f5f6f7f8f9|} {foox:|eff0f1f2f3f4f5f6f7f8f9|}
240 {foo:|f0f1f2f3f4f5f6f7f8f9fa|} {foox:|f0f1f2f3f4f5f6f7f8f9fa|}
241 {foo:|f1f2f3f4f5f6f7f8f9fafb|} {foox:|f1f2f3f4f5f6f7f8f9fafb|}
242 {foo:|f2f3f4f5f6f7f8f9fafbfc|} {foox:|f2f3f4f5f6f7f8f9fafbfc|}
243 {foo:|f3f4f5f6f7f8f9fafbfcfd|} {foox:|f3f4f5f6f7f8f9fafbfcfd|}
244 {foo:|f4f5f6f7f8f9fafbfcfdfe|} {foox:|f4f5f6f7f8f9fafbfcfdfe|}
245 {foo:|f5f6f7f8f9fafbfcfdfeff|} {foox:|f5f6f7f8f9fafbfcfdfeff|}
246 {foo:|f6f7f8f9fafbfcfdfeff00|} {foox:|f6f7f8f9fafbfcfdfeff00|}
247 {foo:|f7f8f9fafbfcfdfeff0001|} {foox:|f7f8f9fafbfcfdfeff0001|}
248 {foo:|f8f9fafbfcfdfeff000102|} {foox:|f8f9fafbfcfdfeff000102|}
249 {foo:|f9fafbfcfdfeff00010203|} {foox:|f9fafbfcfdfeff00010203|}
250 {foo:|fafbfcfdfeff0001020304|} {foox:|fafbfcfdfeff0001020304|}
251 {foo:|fbfcfdfeff000102030405|} {foox:|fbfcfdfeff000102030405|}
252 {foo:|fcfdfeff00010203040506|} {foox:|fcfdfeff00010203040506|}
253 {foo:|fdfeff0001020304050607|} {foox:|fdfeff0001020304050607|}
254 {foo:|feff000102030405060708|} {foox:|feff000102030405060708|}
255 {foo:|ff00010203040506070809|} {foox:|ff00010203040506070809|}
===*/

function test() {
    var i, j, len;
    var buf;

    for (len = 0; len < 64; len++) {
        buf = Duktape.Buffer(len);
        for (i = 0; i < buf.length; i++) {
            buf[i] = 0x55 * i;
        }

        // Vary key by 1 char to ensure both aligned and unaligned output for
        // hex data.
        print(len, Duktape.enc('jx', { foo: buf }), Duktape.enc('jx', { foox: buf }));
    }

    // March all bytes through an 11 byte long buffer (2 x 4 bytes fast path, 3 leftover).
    for (i = 0; i < 256; i++) {
        buf = Duktape.Buffer(11);
        for (j = 0; j < 11; j++) {
            buf[j] = i + j;
        }
        print(i, Duktape.enc('jx', { foo: buf }), Duktape.enc('jx', { foox: buf }));
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
