/*
 *  Loop detection in slow path uses a hybrid approach since Duktape 1.4.0,
 *  exercise its corner cases using loops of different depth.
 */

/*===
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30
31
32
33
34
35
36
37
38
39
40
41
42
43
44
45
46
47
48
49
50
51
52
53
54
55
56
57
58
59
60
61
62
63
64
65
66
67
68
69
70
71
78
85
92
99
106
113
120
127
134
141
148
155
162
169
176
183
190
197
204
211
218
225
232
239
246
253
260
267
274
281
288
295
302
309
316
323
330
337
344
351
358
365
372
379
386
393
400
407
414
421
428
435
442
449
456
463
470
477
484
491
498
505
512
519
526
533
540
547
554
561
568
575
582
589
596
603
610
617
624
631
638
645
652
659
666
673
680
687
694
701
708
715
722
729
736
743
750
757
764
771
778
785
792
799
806
813
820
827
834
841
848
855
862
869
876
883
890
897
904
911
918
925
932
939
946
953
960
967
974
981
988
995
expected errors: 3117
===*/

// identity replacer to forcibly avoid fast path
function id(k, v) {
    return v;
}

function mkloop(depth, loopDepth) {
    var objects = [];
    var i;

    for (i = 0; i < depth; i++) {
        objects.push({ myDepth: i });
    }

    // Link objects linearly.
    for (i = 0; i < depth - 1; i++) {
        objects[i].ref = objects[i + 1];
    }

    // Add loop link to specified object.
    objects[depth - 1].loopRef = objects[loopDepth];

    return objects[0];
}

function test() {
    var i, j, j_step;
    var obj;
    var match = 0;

    for (i = 1; i < 1000;) {
        print(i);

        // visiting[] is now 64 entries, so go over that a bit densely,
        // then skip.
        j_step = (i <= 70 ? 1 : i >>> 2);

        for (j = 0; j < i; j += j_step) {
            obj = mkloop(i, j);

            try {
                print(JSON.stringify(obj, id, 4));
            } catch (e) {
                if (e.name === 'TypeError' &&
                    (e.message.toLowerCase().indexOf('cyclic') >= 0 ||
                     e.message.toLowerCase().indexOf('circular') >= 0)) {
                    match++;
                } else {
                    print(e.stack);
                    throw new Error('no cyclic error, i=' + i + ', j=' + j + ': ' + e);
                }
            }
        }

        // Sample i densely over the visiting[] size, then skip.
        if (i <= 70) {
            i++;
        } else {
            i += 7;
        }
    }

    print('expected errors: ' + match);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
