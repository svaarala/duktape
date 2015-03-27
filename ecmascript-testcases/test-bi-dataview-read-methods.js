/*
 *  Read through a DataView
 */

/*@include util-typedarray.js@*/

/*---
{
    "custom": true,
    "slow": true
}
---*/

/* Custom because there are a few numconv rounding issues. */

/*===
read TypedArray test, arrayLength 16
16 bytes: deadbeefcafebabe1122334455667788
0 0 0  RangeError
0 0 1  RangeError
0 0 2  RangeError
0 0 3  RangeError
0 0 4  RangeError
0 0 5  RangeError
0 0 6  RangeError
0 0 7  RangeError
0 1 0  RangeError
0 1 1  RangeError
0 1 2  RangeError
0 1 3  RangeError
0 1 4  RangeError
0 1 5  RangeError
0 1 6  RangeError
0 1 7  RangeError
0 2 0  RangeError
0 2 1  RangeError
0 2 2  RangeError
0 2 3  RangeError
0 2 4  RangeError
0 2 5  RangeError
0 2 6  RangeError
0 2 7  RangeError
0 3 0  RangeError
0 3 1  RangeError
0 3 2  RangeError
0 3 3  RangeError
0 3 4  RangeError
0 3 5  RangeError
0 3 6  RangeError
0 3 7  RangeError
0 4 0  RangeError
0 4 1  RangeError
0 4 2  RangeError
0 4 3  RangeError
0 4 4  RangeError
0 4 5  RangeError
0 4 6  RangeError
0 4 7  RangeError
0 5 0  RangeError
0 5 1  RangeError
0 5 2  RangeError
0 5 3  RangeError
0 5 4  RangeError
0 5 5  RangeError
0 5 6  RangeError
0 5 7  RangeError
0 6 0  RangeError
0 6 1  RangeError
0 6 2  RangeError
0 6 3  RangeError
0 6 4  RangeError
0 6 5  RangeError
0 6 6  RangeError
0 6 7  RangeError
0 7 0  RangeError
0 7 1  RangeError
0 7 2  RangeError
0 7 3  RangeError
0 7 4  RangeError
0 7 5  RangeError
0 7 6  RangeError
0 7 7  RangeError
0 8 0  RangeError
0 8 1  RangeError
0 8 2  RangeError
0 8 3  RangeError
0 8 4  RangeError
0 8 5  RangeError
0 8 6  RangeError
0 8 7  RangeError
1 0 0  RangeError
1 0 1  RangeError
1 0 2  RangeError
1 0 3  RangeError
1 0 4  RangeError
1 0 5  RangeError
1 0 6  RangeError
1 0 7  RangeError
1 1 0  RangeError
1 1 1  RangeError
1 1 2  RangeError
1 1 3  RangeError
1 1 4  RangeError
1 1 5  RangeError
1 1 6  RangeError
1 1 7  RangeError
1 2 0  RangeError
1 2 1  RangeError
1 2 2  RangeError
1 2 3  RangeError
1 2 4  RangeError
1 2 5  RangeError
1 2 6  RangeError
1 2 7  RangeError
1 3 0  RangeError
1 3 1  RangeError
1 3 2  RangeError
1 3 3  RangeError
1 3 4  RangeError
1 3 5  RangeError
1 3 6  RangeError
1 3 7  RangeError
1 4 0  RangeError
1 4 1  RangeError
1 4 2  RangeError
1 4 3  RangeError
1 4 4  RangeError
1 4 5  RangeError
1 4 6  RangeError
1 4 7  RangeError
1 5 0  RangeError
1 5 1  RangeError
1 5 2  RangeError
1 5 3  RangeError
1 5 4  RangeError
1 5 5  RangeError
1 5 6  RangeError
1 5 7  RangeError
1 6 0  RangeError
1 6 1  RangeError
1 6 2  RangeError
1 6 3  RangeError
1 6 4  RangeError
1 6 5  RangeError
1 6 6  RangeError
1 6 7  RangeError
1 7 0  RangeError
1 7 1  RangeError
1 7 2  RangeError
1 7 3  RangeError
1 7 4  RangeError
1 7 5  RangeError
1 7 6  RangeError
1 7 7  RangeError
1 8 0  RangeError
1 8 1  RangeError
1 8 2  RangeError
1 8 3  RangeError
1 8 4  RangeError
1 8 5  RangeError
1 8 6  RangeError
1 8 7  RangeError
2 0 0  RangeError
2 0 1  RangeError
2 0 2  RangeError
2 0 3  RangeError
2 0 4  RangeError
2 0 5  RangeError
2 0 6  RangeError
2 0 7  RangeError
2 1 0  RangeError
2 1 1  RangeError
2 1 2  RangeError
2 1 3  RangeError
2 1 4  RangeError
2 1 5  RangeError
2 1 6  RangeError
2 1 7  RangeError
2 2 0  RangeError
2 2 1  RangeError
2 2 2  RangeError
2 2 3  RangeError
2 2 4  RangeError
2 2 5  RangeError
2 2 6  RangeError
2 2 7  RangeError
2 3 0  RangeError
2 3 1  RangeError
2 3 2  RangeError
2 3 3  RangeError
2 3 4  RangeError
2 3 5  RangeError
2 3 6  RangeError
2 3 7  RangeError
2 4 0  RangeError
2 4 1  RangeError
2 4 2  RangeError
2 4 3  RangeError
2 4 4  RangeError
2 4 5  RangeError
2 4 6  RangeError
2 4 7  RangeError
2 5 0  RangeError
2 5 1  RangeError
2 5 2  RangeError
2 5 3  RangeError
2 5 4  RangeError
2 5 5  RangeError
2 5 6  RangeError
2 5 7  RangeError
2 6 0  RangeError
2 6 1  RangeError
2 6 2  RangeError
2 6 3  RangeError
2 6 4  RangeError
2 6 5  RangeError
2 6 6  RangeError
2 6 7  RangeError
2 7 0  RangeError
2 7 1  RangeError
2 7 2  RangeError
2 7 3  RangeError
2 7 4  RangeError
2 7 5  RangeError
2 7 6  RangeError
2 7 7  RangeError
2 8 0  RangeError
2 8 1  RangeError
2 8 2  RangeError
2 8 3  RangeError
2 8 4  RangeError
2 8 5  RangeError
2 8 6  RangeError
2 8 7  RangeError
3 0 0 -34 -34 -83 -83
3 0 1 222 222 173 173
3 0 2 -21026 -8531 -16723 -21058
3 0 3 44510 57005 48813 44478
3 0 4 -272716322 -559038737 -890257747 -1379995702
3 0 5 4022250974 3735928559 3404709549 2914971594
3 0 6 -1.1802468879641618e+29 -6259853398707798000 -7855958.5 -2.1706986902403358e-11
3 0 7 -0.0000016090443173227715 -1.1885958788264018e+148 3.3208721076639256e-223 -2.4299377383893497e-88
3 1 0  RangeError
3 1 1  RangeError
3 1 2  RangeError
3 1 3  RangeError
3 1 4  RangeError
3 1 5  RangeError
3 1 6  RangeError
3 1 7  RangeError
3 2 0 -34 -34 RangeError
3 2 1 222 222 RangeError
3 2 2  RangeError
3 2 3  RangeError
3 2 4  RangeError
3 2 5  RangeError
3 2 6  RangeError
3 2 7  RangeError
3 3 0 -34 -34 -83 -83
3 3 1 222 222 173 173
3 3 2 -21026 -8531 RangeError
3 3 3 44510 57005 RangeError
3 3 4  RangeError
3 3 5  RangeError
3 3 6  RangeError
3 3 7  RangeError
3 4 0 -34 -34 -83 -83
3 4 1 222 222 173 173
3 4 2 -21026 -8531 -16723 -21058
3 4 3 44510 57005 48813 44478
3 4 4  RangeError
3 4 5  RangeError
3 4 6  RangeError
3 4 7  RangeError
3 5 0 -34 -34 -83 -83
3 5 1 222 222 173 173
3 5 2 -21026 -8531 -16723 -21058
3 5 3 44510 57005 48813 44478
3 5 4 -272716322 -559038737 RangeError
3 5 5 4022250974 3735928559 RangeError
3 5 6 -1.1802468879641618e+29 -6259853398707798000 RangeError
3 5 7  RangeError
3 6 0 -34 -34 -83 -83
3 6 1 222 222 173 173
3 6 2 -21026 -8531 -16723 -21058
3 6 3 44510 57005 48813 44478
3 6 4 -272716322 -559038737 -890257747 -1379995702
3 6 5 4022250974 3735928559 3404709549 2914971594
3 6 6 -1.1802468879641618e+29 -6259853398707798000 -7855958.5 -2.1706986902403358e-11
3 6 7  RangeError
3 7 0 -34 -34 -83 -83
3 7 1 222 222 173 173
3 7 2 -21026 -8531 -16723 -21058
3 7 3 44510 57005 48813 44478
3 7 4 -272716322 -559038737 -890257747 -1379995702
3 7 5 4022250974 3735928559 3404709549 2914971594
3 7 6 -1.1802468879641618e+29 -6259853398707798000 -7855958.5 -2.1706986902403358e-11
3 7 7 -0.0000016090443173227715 -1.1885958788264018e+148 RangeError
3 8 0  RangeError
3 8 1  RangeError
3 8 2  RangeError
3 8 3  RangeError
3 8 4  RangeError
3 8 5  RangeError
3 8 6  RangeError
3 8 7  RangeError
4 0 0 -83 -83 -66 -66
4 0 1 173 173 190 190
4 0 2 -16723 -21058 -4162 -16657
4 0 3 48813 44478 61374 48879
4 0 4 -890257747 -1379995702 -20254786 -1091581186
4 0 5 3404709549 2914971594 4274712510 3203386110
4 0 6 -7855958.5 -2.1706986902403358e-11 -1.3487443387778376e+38 -0.468345582485199
4 0 7 3.3208721076639256e-223 -2.4299377383893497e-88 1.4210818500899943e-144 -0.000015160059625215153
4 1 0  RangeError
4 1 1  RangeError
4 1 2  RangeError
4 1 3  RangeError
4 1 4  RangeError
4 1 5  RangeError
4 1 6  RangeError
4 1 7  RangeError
4 2 0 -83 -83 RangeError
4 2 1 173 173 RangeError
4 2 2  RangeError
4 2 3  RangeError
4 2 4  RangeError
4 2 5  RangeError
4 2 6  RangeError
4 2 7  RangeError
4 3 0 -83 -83 -66 -66
4 3 1 173 173 190 190
4 3 2 -16723 -21058 RangeError
4 3 3 48813 44478 RangeError
4 3 4  RangeError
4 3 5  RangeError
4 3 6  RangeError
4 3 7  RangeError
4 4 0 -83 -83 -66 -66
4 4 1 173 173 190 190
4 4 2 -16723 -21058 -4162 -16657
4 4 3 48813 44478 61374 48879
4 4 4  RangeError
4 4 5  RangeError
4 4 6  RangeError
4 4 7  RangeError
4 5 0 -83 -83 -66 -66
4 5 1 173 173 190 190
4 5 2 -16723 -21058 -4162 -16657
4 5 3 48813 44478 61374 48879
4 5 4 -890257747 -1379995702 RangeError
4 5 5 3404709549 2914971594 RangeError
4 5 6 -7855958.5 -2.1706986902403358e-11 RangeError
4 5 7  RangeError
4 6 0 -83 -83 -66 -66
4 6 1 173 173 190 190
4 6 2 -16723 -21058 -4162 -16657
4 6 3 48813 44478 61374 48879
4 6 4 -890257747 -1379995702 -20254786 -1091581186
4 6 5 3404709549 2914971594 4274712510 3203386110
4 6 6 -7855958.5 -2.1706986902403358e-11 -1.3487443387778376e+38 -0.468345582485199
4 6 7  RangeError
4 7 0 -83 -83 -66 -66
4 7 1 173 173 190 190
4 7 2 -16723 -21058 -4162 -16657
4 7 3 48813 44478 61374 48879
4 7 4 -890257747 -1379995702 -20254786 -1091581186
4 7 5 3404709549 2914971594 4274712510 3203386110
4 7 6 -7855958.5 -2.1706986902403358e-11 -1.3487443387778376e+38 -0.468345582485199
4 7 7 3.3208721076639256e-223 -2.4299377383893497e-88 RangeError
4 8 0  RangeError
4 8 1  RangeError
4 8 2  RangeError
4 8 3  RangeError
4 8 4  RangeError
4 8 5  RangeError
4 8 6  RangeError
4 8 7  RangeError
5 0 0 -66 -66 -17 -17
5 0 1 190 190 239 239
5 0 2 -4162 -16657 -13585 -4150
5 0 3 61374 48879 51951 61386
5 0 4 -20254786 -1091581186 -1157707025 -271909190
5 0 5 4274712510 3203386110 3137260271 4023058106
5 0 6 -1.3487443387778376e+38 -0.468345582485199 -0.001943914103321731 -1.2564783500451927e+29
5 0 7 1.4210818500899943e-144 -0.000015160059625215153 2.196203770486694e-62 -3.2742408819041713e+230
5 1 0  RangeError
5 1 1  RangeError
5 1 2  RangeError
5 1 3  RangeError
5 1 4  RangeError
5 1 5  RangeError
5 1 6  RangeError
5 1 7  RangeError
5 2 0 -66 -66 RangeError
5 2 1 190 190 RangeError
5 2 2  RangeError
5 2 3  RangeError
5 2 4  RangeError
5 2 5  RangeError
5 2 6  RangeError
5 2 7  RangeError
5 3 0 -66 -66 -17 -17
5 3 1 190 190 239 239
5 3 2 -4162 -16657 RangeError
5 3 3 61374 48879 RangeError
5 3 4  RangeError
5 3 5  RangeError
5 3 6  RangeError
5 3 7  RangeError
5 4 0 -66 -66 -17 -17
5 4 1 190 190 239 239
5 4 2 -4162 -16657 -13585 -4150
5 4 3 61374 48879 51951 61386
5 4 4  RangeError
5 4 5  RangeError
5 4 6  RangeError
5 4 7  RangeError
5 5 0 -66 -66 -17 -17
5 5 1 190 190 239 239
5 5 2 -4162 -16657 -13585 -4150
5 5 3 61374 48879 51951 61386
5 5 4 -20254786 -1091581186 RangeError
5 5 5 4274712510 3203386110 RangeError
5 5 6 -1.3487443387778376e+38 -0.468345582485199 RangeError
5 5 7  RangeError
5 6 0 -66 -66 -17 -17
5 6 1 190 190 239 239
5 6 2 -4162 -16657 -13585 -4150
5 6 3 61374 48879 51951 61386
5 6 4 -20254786 -1091581186 -1157707025 -271909190
5 6 5 4274712510 3203386110 3137260271 4023058106
5 6 6 -1.3487443387778376e+38 -0.468345582485199 -0.001943914103321731 -1.2564783500451927e+29
5 6 7  RangeError
5 7 0 -66 -66 -17 -17
5 7 1 190 190 239 239
5 7 2 -4162 -16657 -13585 -4150
5 7 3 61374 48879 51951 61386
5 7 4 -20254786 -1091581186 -1157707025 -271909190
5 7 5 4274712510 3203386110 3137260271 4023058106
5 7 6 -1.3487443387778376e+38 -0.468345582485199 -0.001943914103321731 -1.2564783500451927e+29
5 7 7 1.4210818500899943e-144 -0.000015160059625215153 RangeError
5 8 0  RangeError
5 8 1  RangeError
5 8 2  RangeError
5 8 3  RangeError
5 8 4  RangeError
5 8 5  RangeError
5 8 6  RangeError
5 8 7  RangeError
6 0 0 17 17 34 34
6 0 1 17 17 34 34
6 0 2 8721 4386 13090 8755
6 0 3 8721 4386 13090 8755
6 0 4 1144201745 287454020 1430532898 573785173
6 0 5 1144201745 287454020 1430532898 573785173
6 0 6 716.5322875976563 1.2795344104949228e-28 13482743300096 2.4295198285501637e-18
6 0 7 -7.086876636573014e-268 3.841412024471731e-226 RangeError
6 1 0  RangeError
6 1 1  RangeError
6 1 2  RangeError
6 1 3  RangeError
6 1 4  RangeError
6 1 5  RangeError
6 1 6  RangeError
6 1 7  RangeError
6 2 0 17 17 RangeError
6 2 1 17 17 RangeError
6 2 2  RangeError
6 2 3  RangeError
6 2 4  RangeError
6 2 5  RangeError
6 2 6  RangeError
6 2 7  RangeError
6 3 0 17 17 34 34
6 3 1 17 17 34 34
6 3 2 8721 4386 RangeError
6 3 3 8721 4386 RangeError
6 3 4  RangeError
6 3 5  RangeError
6 3 6  RangeError
6 3 7  RangeError
6 4 0 17 17 34 34
6 4 1 17 17 34 34
6 4 2 8721 4386 13090 8755
6 4 3 8721 4386 13090 8755
6 4 4  RangeError
6 4 5  RangeError
6 4 6  RangeError
6 4 7  RangeError
6 5 0 17 17 34 34
6 5 1 17 17 34 34
6 5 2 8721 4386 13090 8755
6 5 3 8721 4386 13090 8755
6 5 4 1144201745 287454020 RangeError
6 5 5 1144201745 287454020 RangeError
6 5 6 716.5322875976563 1.2795344104949228e-28 RangeError
6 5 7  RangeError
6 6 0 17 17 34 34
6 6 1 17 17 34 34
6 6 2 8721 4386 13090 8755
6 6 3 8721 4386 13090 8755
6 6 4 1144201745 287454020 1430532898 573785173
6 6 5 1144201745 287454020 1430532898 573785173
6 6 6 716.5322875976563 1.2795344104949228e-28 13482743300096 2.4295198285501637e-18
6 6 7  RangeError
6 7 0 17 17 34 34
6 7 1 17 17 34 34
6 7 2 8721 4386 13090 8755
6 7 3 8721 4386 13090 8755
6 7 4 1144201745 287454020 1430532898 573785173
6 7 5 1144201745 287454020 1430532898 573785173
6 7 6 716.5322875976563 1.2795344104949228e-28 13482743300096 2.4295198285501637e-18
6 7 7 -7.086876636573014e-268 3.841412024471731e-226 RangeError
6 8 0  RangeError
6 8 1  RangeError
6 8 2  RangeError
6 8 3  RangeError
6 8 4  RangeError
6 8 5  RangeError
6 8 6  RangeError
6 8 7  RangeError
7 0 0 102 102 119 119
7 0 1 102 102 119 119
7 0 2 30566 26231 -30601 30600
7 0 3 30566 26231 34935 30600
7 0 4  RangeError
7 0 5  RangeError
7 0 6  RangeError
7 0 7  RangeError
7 1 0  RangeError
7 1 1  RangeError
7 1 2  RangeError
7 1 3  RangeError
7 1 4  RangeError
7 1 5  RangeError
7 1 6  RangeError
7 1 7  RangeError
7 2 0 102 102 RangeError
7 2 1 102 102 RangeError
7 2 2  RangeError
7 2 3  RangeError
7 2 4  RangeError
7 2 5  RangeError
7 2 6  RangeError
7 2 7  RangeError
7 3 0 102 102 119 119
7 3 1 102 102 119 119
7 3 2 30566 26231 RangeError
7 3 3 30566 26231 RangeError
7 3 4  RangeError
7 3 5  RangeError
7 3 6  RangeError
7 3 7  RangeError
7 4 0 102 102 119 119
7 4 1 102 102 119 119
7 4 2 30566 26231 -30601 30600
7 4 3 30566 26231 34935 30600
7 4 4  RangeError
7 4 5  RangeError
7 4 6  RangeError
7 4 7  RangeError
7 5 0  RangeError
7 5 1  RangeError
7 5 2  RangeError
7 5 3  RangeError
7 5 4  RangeError
7 5 5  RangeError
7 5 6  RangeError
7 5 7  RangeError
7 6 0  RangeError
7 6 1  RangeError
7 6 2  RangeError
7 6 3  RangeError
7 6 4  RangeError
7 6 5  RangeError
7 6 6  RangeError
7 6 7  RangeError
7 7 0  RangeError
7 7 1  RangeError
7 7 2  RangeError
7 7 3  RangeError
7 7 4  RangeError
7 7 5  RangeError
7 7 6  RangeError
7 7 7  RangeError
7 8 0  RangeError
7 8 1  RangeError
7 8 2  RangeError
7 8 3  RangeError
7 8 4  RangeError
7 8 5  RangeError
7 8 6  RangeError
7 8 7  RangeError
8 0 0 -120 -120 RangeError
8 0 1 136 136 RangeError
8 0 2  RangeError
8 0 3  RangeError
8 0 4  RangeError
8 0 5  RangeError
8 0 6  RangeError
8 0 7  RangeError
8 1 0  RangeError
8 1 1  RangeError
8 1 2  RangeError
8 1 3  RangeError
8 1 4  RangeError
8 1 5  RangeError
8 1 6  RangeError
8 1 7  RangeError
8 2 0 -120 -120 RangeError
8 2 1 136 136 RangeError
8 2 2  RangeError
8 2 3  RangeError
8 2 4  RangeError
8 2 5  RangeError
8 2 6  RangeError
8 2 7  RangeError
8 3 0  RangeError
8 3 1  RangeError
8 3 2  RangeError
8 3 3  RangeError
8 3 4  RangeError
8 3 5  RangeError
8 3 6  RangeError
8 3 7  RangeError
8 4 0  RangeError
8 4 1  RangeError
8 4 2  RangeError
8 4 3  RangeError
8 4 4  RangeError
8 4 5  RangeError
8 4 6  RangeError
8 4 7  RangeError
8 5 0  RangeError
8 5 1  RangeError
8 5 2  RangeError
8 5 3  RangeError
8 5 4  RangeError
8 5 5  RangeError
8 5 6  RangeError
8 5 7  RangeError
8 6 0  RangeError
8 6 1  RangeError
8 6 2  RangeError
8 6 3  RangeError
8 6 4  RangeError
8 6 5  RangeError
8 6 6  RangeError
8 6 7  RangeError
8 7 0  RangeError
8 7 1  RangeError
8 7 2  RangeError
8 7 3  RangeError
8 7 4  RangeError
8 7 5  RangeError
8 7 6  RangeError
8 7 7  RangeError
8 8 0  RangeError
8 8 1  RangeError
8 8 2  RangeError
8 8 3  RangeError
8 8 4  RangeError
8 8 5  RangeError
8 8 6  RangeError
8 8 7  RangeError
9 0 0  RangeError
9 0 1  RangeError
9 0 2  RangeError
9 0 3  RangeError
9 0 4  RangeError
9 0 5  RangeError
9 0 6  RangeError
9 0 7  RangeError
9 1 0  RangeError
9 1 1  RangeError
9 1 2  RangeError
9 1 3  RangeError
9 1 4  RangeError
9 1 5  RangeError
9 1 6  RangeError
9 1 7  RangeError
9 2 0  RangeError
9 2 1  RangeError
9 2 2  RangeError
9 2 3  RangeError
9 2 4  RangeError
9 2 5  RangeError
9 2 6  RangeError
9 2 7  RangeError
9 3 0  RangeError
9 3 1  RangeError
9 3 2  RangeError
9 3 3  RangeError
9 3 4  RangeError
9 3 5  RangeError
9 3 6  RangeError
9 3 7  RangeError
9 4 0  RangeError
9 4 1  RangeError
9 4 2  RangeError
9 4 3  RangeError
9 4 4  RangeError
9 4 5  RangeError
9 4 6  RangeError
9 4 7  RangeError
9 5 0  RangeError
9 5 1  RangeError
9 5 2  RangeError
9 5 3  RangeError
9 5 4  RangeError
9 5 5  RangeError
9 5 6  RangeError
9 5 7  RangeError
9 6 0  RangeError
9 6 1  RangeError
9 6 2  RangeError
9 6 3  RangeError
9 6 4  RangeError
9 6 5  RangeError
9 6 6  RangeError
9 6 7  RangeError
9 7 0  RangeError
9 7 1  RangeError
9 7 2  RangeError
9 7 3  RangeError
9 7 4  RangeError
9 7 5  RangeError
9 7 6  RangeError
9 7 7  RangeError
9 8 0  RangeError
9 8 1  RangeError
9 8 2  RangeError
9 8 3  RangeError
9 8 4  RangeError
9 8 5  RangeError
9 8 6  RangeError
9 8 7  RangeError
10 0 0  RangeError
10 0 1  RangeError
10 0 2  RangeError
10 0 3  RangeError
10 0 4  RangeError
10 0 5  RangeError
10 0 6  RangeError
10 0 7  RangeError
10 1 0  RangeError
10 1 1  RangeError
10 1 2  RangeError
10 1 3  RangeError
10 1 4  RangeError
10 1 5  RangeError
10 1 6  RangeError
10 1 7  RangeError
10 2 0  RangeError
10 2 1  RangeError
10 2 2  RangeError
10 2 3  RangeError
10 2 4  RangeError
10 2 5  RangeError
10 2 6  RangeError
10 2 7  RangeError
10 3 0  RangeError
10 3 1  RangeError
10 3 2  RangeError
10 3 3  RangeError
10 3 4  RangeError
10 3 5  RangeError
10 3 6  RangeError
10 3 7  RangeError
10 4 0  RangeError
10 4 1  RangeError
10 4 2  RangeError
10 4 3  RangeError
10 4 4  RangeError
10 4 5  RangeError
10 4 6  RangeError
10 4 7  RangeError
10 5 0  RangeError
10 5 1  RangeError
10 5 2  RangeError
10 5 3  RangeError
10 5 4  RangeError
10 5 5  RangeError
10 5 6  RangeError
10 5 7  RangeError
10 6 0  RangeError
10 6 1  RangeError
10 6 2  RangeError
10 6 3  RangeError
10 6 4  RangeError
10 6 5  RangeError
10 6 6  RangeError
10 6 7  RangeError
10 7 0  RangeError
10 7 1  RangeError
10 7 2  RangeError
10 7 3  RangeError
10 7 4  RangeError
10 7 5  RangeError
10 7 6  RangeError
10 7 7  RangeError
10 8 0  RangeError
10 8 1  RangeError
10 8 2  RangeError
10 8 3  RangeError
10 8 4  RangeError
10 8 5  RangeError
10 8 6  RangeError
10 8 7  RangeError
11 0 0  RangeError
11 0 1  RangeError
11 0 2  RangeError
11 0 3  RangeError
11 0 4  RangeError
11 0 5  RangeError
11 0 6  RangeError
11 0 7  RangeError
11 1 0  RangeError
11 1 1  RangeError
11 1 2  RangeError
11 1 3  RangeError
11 1 4  RangeError
11 1 5  RangeError
11 1 6  RangeError
11 1 7  RangeError
11 2 0  RangeError
11 2 1  RangeError
11 2 2  RangeError
11 2 3  RangeError
11 2 4  RangeError
11 2 5  RangeError
11 2 6  RangeError
11 2 7  RangeError
11 3 0  RangeError
11 3 1  RangeError
11 3 2  RangeError
11 3 3  RangeError
11 3 4  RangeError
11 3 5  RangeError
11 3 6  RangeError
11 3 7  RangeError
11 4 0  RangeError
11 4 1  RangeError
11 4 2  RangeError
11 4 3  RangeError
11 4 4  RangeError
11 4 5  RangeError
11 4 6  RangeError
11 4 7  RangeError
11 5 0  RangeError
11 5 1  RangeError
11 5 2  RangeError
11 5 3  RangeError
11 5 4  RangeError
11 5 5  RangeError
11 5 6  RangeError
11 5 7  RangeError
11 6 0  RangeError
11 6 1  RangeError
11 6 2  RangeError
11 6 3  RangeError
11 6 4  RangeError
11 6 5  RangeError
11 6 6  RangeError
11 6 7  RangeError
11 7 0  RangeError
11 7 1  RangeError
11 7 2  RangeError
11 7 3  RangeError
11 7 4  RangeError
11 7 5  RangeError
11 7 6  RangeError
11 7 7  RangeError
11 8 0  RangeError
11 8 1  RangeError
11 8 2  RangeError
11 8 3  RangeError
11 8 4  RangeError
11 8 5  RangeError
11 8 6  RangeError
11 8 7  RangeError
read TypedArray test, arrayLength 17
17 bytes: deadbeefcafebabe112233445566778899
0 0 0  RangeError
0 0 1  RangeError
0 0 2  RangeError
0 0 3  RangeError
0 0 4  RangeError
0 0 5  RangeError
0 0 6  RangeError
0 0 7  RangeError
0 1 0  RangeError
0 1 1  RangeError
0 1 2  RangeError
0 1 3  RangeError
0 1 4  RangeError
0 1 5  RangeError
0 1 6  RangeError
0 1 7  RangeError
0 2 0  RangeError
0 2 1  RangeError
0 2 2  RangeError
0 2 3  RangeError
0 2 4  RangeError
0 2 5  RangeError
0 2 6  RangeError
0 2 7  RangeError
0 3 0  RangeError
0 3 1  RangeError
0 3 2  RangeError
0 3 3  RangeError
0 3 4  RangeError
0 3 5  RangeError
0 3 6  RangeError
0 3 7  RangeError
0 4 0  RangeError
0 4 1  RangeError
0 4 2  RangeError
0 4 3  RangeError
0 4 4  RangeError
0 4 5  RangeError
0 4 6  RangeError
0 4 7  RangeError
0 5 0  RangeError
0 5 1  RangeError
0 5 2  RangeError
0 5 3  RangeError
0 5 4  RangeError
0 5 5  RangeError
0 5 6  RangeError
0 5 7  RangeError
0 6 0  RangeError
0 6 1  RangeError
0 6 2  RangeError
0 6 3  RangeError
0 6 4  RangeError
0 6 5  RangeError
0 6 6  RangeError
0 6 7  RangeError
0 7 0  RangeError
0 7 1  RangeError
0 7 2  RangeError
0 7 3  RangeError
0 7 4  RangeError
0 7 5  RangeError
0 7 6  RangeError
0 7 7  RangeError
0 8 0  RangeError
0 8 1  RangeError
0 8 2  RangeError
0 8 3  RangeError
0 8 4  RangeError
0 8 5  RangeError
0 8 6  RangeError
0 8 7  RangeError
1 0 0  RangeError
1 0 1  RangeError
1 0 2  RangeError
1 0 3  RangeError
1 0 4  RangeError
1 0 5  RangeError
1 0 6  RangeError
1 0 7  RangeError
1 1 0  RangeError
1 1 1  RangeError
1 1 2  RangeError
1 1 3  RangeError
1 1 4  RangeError
1 1 5  RangeError
1 1 6  RangeError
1 1 7  RangeError
1 2 0  RangeError
1 2 1  RangeError
1 2 2  RangeError
1 2 3  RangeError
1 2 4  RangeError
1 2 5  RangeError
1 2 6  RangeError
1 2 7  RangeError
1 3 0  RangeError
1 3 1  RangeError
1 3 2  RangeError
1 3 3  RangeError
1 3 4  RangeError
1 3 5  RangeError
1 3 6  RangeError
1 3 7  RangeError
1 4 0  RangeError
1 4 1  RangeError
1 4 2  RangeError
1 4 3  RangeError
1 4 4  RangeError
1 4 5  RangeError
1 4 6  RangeError
1 4 7  RangeError
1 5 0  RangeError
1 5 1  RangeError
1 5 2  RangeError
1 5 3  RangeError
1 5 4  RangeError
1 5 5  RangeError
1 5 6  RangeError
1 5 7  RangeError
1 6 0  RangeError
1 6 1  RangeError
1 6 2  RangeError
1 6 3  RangeError
1 6 4  RangeError
1 6 5  RangeError
1 6 6  RangeError
1 6 7  RangeError
1 7 0  RangeError
1 7 1  RangeError
1 7 2  RangeError
1 7 3  RangeError
1 7 4  RangeError
1 7 5  RangeError
1 7 6  RangeError
1 7 7  RangeError
1 8 0  RangeError
1 8 1  RangeError
1 8 2  RangeError
1 8 3  RangeError
1 8 4  RangeError
1 8 5  RangeError
1 8 6  RangeError
1 8 7  RangeError
2 0 0  RangeError
2 0 1  RangeError
2 0 2  RangeError
2 0 3  RangeError
2 0 4  RangeError
2 0 5  RangeError
2 0 6  RangeError
2 0 7  RangeError
2 1 0  RangeError
2 1 1  RangeError
2 1 2  RangeError
2 1 3  RangeError
2 1 4  RangeError
2 1 5  RangeError
2 1 6  RangeError
2 1 7  RangeError
2 2 0  RangeError
2 2 1  RangeError
2 2 2  RangeError
2 2 3  RangeError
2 2 4  RangeError
2 2 5  RangeError
2 2 6  RangeError
2 2 7  RangeError
2 3 0  RangeError
2 3 1  RangeError
2 3 2  RangeError
2 3 3  RangeError
2 3 4  RangeError
2 3 5  RangeError
2 3 6  RangeError
2 3 7  RangeError
2 4 0  RangeError
2 4 1  RangeError
2 4 2  RangeError
2 4 3  RangeError
2 4 4  RangeError
2 4 5  RangeError
2 4 6  RangeError
2 4 7  RangeError
2 5 0  RangeError
2 5 1  RangeError
2 5 2  RangeError
2 5 3  RangeError
2 5 4  RangeError
2 5 5  RangeError
2 5 6  RangeError
2 5 7  RangeError
2 6 0  RangeError
2 6 1  RangeError
2 6 2  RangeError
2 6 3  RangeError
2 6 4  RangeError
2 6 5  RangeError
2 6 6  RangeError
2 6 7  RangeError
2 7 0  RangeError
2 7 1  RangeError
2 7 2  RangeError
2 7 3  RangeError
2 7 4  RangeError
2 7 5  RangeError
2 7 6  RangeError
2 7 7  RangeError
2 8 0  RangeError
2 8 1  RangeError
2 8 2  RangeError
2 8 3  RangeError
2 8 4  RangeError
2 8 5  RangeError
2 8 6  RangeError
2 8 7  RangeError
3 0 0 -34 -34 -83 -83
3 0 1 222 222 173 173
3 0 2 -21026 -8531 -16723 -21058
3 0 3 44510 57005 48813 44478
3 0 4 -272716322 -559038737 -890257747 -1379995702
3 0 5 4022250974 3735928559 3404709549 2914971594
3 0 6 -1.1802468879641618e+29 -6259853398707798000 -7855958.5 -2.1706986902403358e-11
3 0 7 -0.0000016090443173227715 -1.1885958788264018e+148 3.3208721076639256e-223 -2.4299377383893497e-88
3 1 0  RangeError
3 1 1  RangeError
3 1 2  RangeError
3 1 3  RangeError
3 1 4  RangeError
3 1 5  RangeError
3 1 6  RangeError
3 1 7  RangeError
3 2 0 -34 -34 RangeError
3 2 1 222 222 RangeError
3 2 2  RangeError
3 2 3  RangeError
3 2 4  RangeError
3 2 5  RangeError
3 2 6  RangeError
3 2 7  RangeError
3 3 0 -34 -34 -83 -83
3 3 1 222 222 173 173
3 3 2 -21026 -8531 RangeError
3 3 3 44510 57005 RangeError
3 3 4  RangeError
3 3 5  RangeError
3 3 6  RangeError
3 3 7  RangeError
3 4 0 -34 -34 -83 -83
3 4 1 222 222 173 173
3 4 2 -21026 -8531 -16723 -21058
3 4 3 44510 57005 48813 44478
3 4 4  RangeError
3 4 5  RangeError
3 4 6  RangeError
3 4 7  RangeError
3 5 0 -34 -34 -83 -83
3 5 1 222 222 173 173
3 5 2 -21026 -8531 -16723 -21058
3 5 3 44510 57005 48813 44478
3 5 4 -272716322 -559038737 RangeError
3 5 5 4022250974 3735928559 RangeError
3 5 6 -1.1802468879641618e+29 -6259853398707798000 RangeError
3 5 7  RangeError
3 6 0 -34 -34 -83 -83
3 6 1 222 222 173 173
3 6 2 -21026 -8531 -16723 -21058
3 6 3 44510 57005 48813 44478
3 6 4 -272716322 -559038737 -890257747 -1379995702
3 6 5 4022250974 3735928559 3404709549 2914971594
3 6 6 -1.1802468879641618e+29 -6259853398707798000 -7855958.5 -2.1706986902403358e-11
3 6 7  RangeError
3 7 0 -34 -34 -83 -83
3 7 1 222 222 173 173
3 7 2 -21026 -8531 -16723 -21058
3 7 3 44510 57005 48813 44478
3 7 4 -272716322 -559038737 -890257747 -1379995702
3 7 5 4022250974 3735928559 3404709549 2914971594
3 7 6 -1.1802468879641618e+29 -6259853398707798000 -7855958.5 -2.1706986902403358e-11
3 7 7 -0.0000016090443173227715 -1.1885958788264018e+148 RangeError
3 8 0  RangeError
3 8 1  RangeError
3 8 2  RangeError
3 8 3  RangeError
3 8 4  RangeError
3 8 5  RangeError
3 8 6  RangeError
3 8 7  RangeError
4 0 0 -83 -83 -66 -66
4 0 1 173 173 190 190
4 0 2 -16723 -21058 -4162 -16657
4 0 3 48813 44478 61374 48879
4 0 4 -890257747 -1379995702 -20254786 -1091581186
4 0 5 3404709549 2914971594 4274712510 3203386110
4 0 6 -7855958.5 -2.1706986902403358e-11 -1.3487443387778376e+38 -0.468345582485199
4 0 7 3.3208721076639256e-223 -2.4299377383893497e-88 1.4210818500899943e-144 -0.000015160059625215153
4 1 0  RangeError
4 1 1  RangeError
4 1 2  RangeError
4 1 3  RangeError
4 1 4  RangeError
4 1 5  RangeError
4 1 6  RangeError
4 1 7  RangeError
4 2 0 -83 -83 RangeError
4 2 1 173 173 RangeError
4 2 2  RangeError
4 2 3  RangeError
4 2 4  RangeError
4 2 5  RangeError
4 2 6  RangeError
4 2 7  RangeError
4 3 0 -83 -83 -66 -66
4 3 1 173 173 190 190
4 3 2 -16723 -21058 RangeError
4 3 3 48813 44478 RangeError
4 3 4  RangeError
4 3 5  RangeError
4 3 6  RangeError
4 3 7  RangeError
4 4 0 -83 -83 -66 -66
4 4 1 173 173 190 190
4 4 2 -16723 -21058 -4162 -16657
4 4 3 48813 44478 61374 48879
4 4 4  RangeError
4 4 5  RangeError
4 4 6  RangeError
4 4 7  RangeError
4 5 0 -83 -83 -66 -66
4 5 1 173 173 190 190
4 5 2 -16723 -21058 -4162 -16657
4 5 3 48813 44478 61374 48879
4 5 4 -890257747 -1379995702 RangeError
4 5 5 3404709549 2914971594 RangeError
4 5 6 -7855958.5 -2.1706986902403358e-11 RangeError
4 5 7  RangeError
4 6 0 -83 -83 -66 -66
4 6 1 173 173 190 190
4 6 2 -16723 -21058 -4162 -16657
4 6 3 48813 44478 61374 48879
4 6 4 -890257747 -1379995702 -20254786 -1091581186
4 6 5 3404709549 2914971594 4274712510 3203386110
4 6 6 -7855958.5 -2.1706986902403358e-11 -1.3487443387778376e+38 -0.468345582485199
4 6 7  RangeError
4 7 0 -83 -83 -66 -66
4 7 1 173 173 190 190
4 7 2 -16723 -21058 -4162 -16657
4 7 3 48813 44478 61374 48879
4 7 4 -890257747 -1379995702 -20254786 -1091581186
4 7 5 3404709549 2914971594 4274712510 3203386110
4 7 6 -7855958.5 -2.1706986902403358e-11 -1.3487443387778376e+38 -0.468345582485199
4 7 7 3.3208721076639256e-223 -2.4299377383893497e-88 RangeError
4 8 0  RangeError
4 8 1  RangeError
4 8 2  RangeError
4 8 3  RangeError
4 8 4  RangeError
4 8 5  RangeError
4 8 6  RangeError
4 8 7  RangeError
5 0 0 -66 -66 -17 -17
5 0 1 190 190 239 239
5 0 2 -4162 -16657 -13585 -4150
5 0 3 61374 48879 51951 61386
5 0 4 -20254786 -1091581186 -1157707025 -271909190
5 0 5 4274712510 3203386110 3137260271 4023058106
5 0 6 -1.3487443387778376e+38 -0.468345582485199 -0.001943914103321731 -1.2564783500451927e+29
5 0 7 1.4210818500899943e-144 -0.000015160059625215153 2.196203770486694e-62 -3.2742408819041713e+230
5 1 0  RangeError
5 1 1  RangeError
5 1 2  RangeError
5 1 3  RangeError
5 1 4  RangeError
5 1 5  RangeError
5 1 6  RangeError
5 1 7  RangeError
5 2 0 -66 -66 RangeError
5 2 1 190 190 RangeError
5 2 2  RangeError
5 2 3  RangeError
5 2 4  RangeError
5 2 5  RangeError
5 2 6  RangeError
5 2 7  RangeError
5 3 0 -66 -66 -17 -17
5 3 1 190 190 239 239
5 3 2 -4162 -16657 RangeError
5 3 3 61374 48879 RangeError
5 3 4  RangeError
5 3 5  RangeError
5 3 6  RangeError
5 3 7  RangeError
5 4 0 -66 -66 -17 -17
5 4 1 190 190 239 239
5 4 2 -4162 -16657 -13585 -4150
5 4 3 61374 48879 51951 61386
5 4 4  RangeError
5 4 5  RangeError
5 4 6  RangeError
5 4 7  RangeError
5 5 0 -66 -66 -17 -17
5 5 1 190 190 239 239
5 5 2 -4162 -16657 -13585 -4150
5 5 3 61374 48879 51951 61386
5 5 4 -20254786 -1091581186 RangeError
5 5 5 4274712510 3203386110 RangeError
5 5 6 -1.3487443387778376e+38 -0.468345582485199 RangeError
5 5 7  RangeError
5 6 0 -66 -66 -17 -17
5 6 1 190 190 239 239
5 6 2 -4162 -16657 -13585 -4150
5 6 3 61374 48879 51951 61386
5 6 4 -20254786 -1091581186 -1157707025 -271909190
5 6 5 4274712510 3203386110 3137260271 4023058106
5 6 6 -1.3487443387778376e+38 -0.468345582485199 -0.001943914103321731 -1.2564783500451927e+29
5 6 7  RangeError
5 7 0 -66 -66 -17 -17
5 7 1 190 190 239 239
5 7 2 -4162 -16657 -13585 -4150
5 7 3 61374 48879 51951 61386
5 7 4 -20254786 -1091581186 -1157707025 -271909190
5 7 5 4274712510 3203386110 3137260271 4023058106
5 7 6 -1.3487443387778376e+38 -0.468345582485199 -0.001943914103321731 -1.2564783500451927e+29
5 7 7 1.4210818500899943e-144 -0.000015160059625215153 RangeError
5 8 0  RangeError
5 8 1  RangeError
5 8 2  RangeError
5 8 3  RangeError
5 8 4  RangeError
5 8 5  RangeError
5 8 6  RangeError
5 8 7  RangeError
6 0 0 17 17 34 34
6 0 1 17 17 34 34
6 0 2 8721 4386 13090 8755
6 0 3 8721 4386 13090 8755
6 0 4 1144201745 287454020 1430532898 573785173
6 0 5 1144201745 287454020 1430532898 573785173
6 0 6 716.5322875976563 1.2795344104949228e-28 13482743300096 2.4295198285501637e-18
6 0 7 -7.086876636573014e-268 3.841412024471731e-226 -1.1246123502434041e-185 6.171838568684498e-144
6 1 0  RangeError
6 1 1  RangeError
6 1 2  RangeError
6 1 3  RangeError
6 1 4  RangeError
6 1 5  RangeError
6 1 6  RangeError
6 1 7  RangeError
6 2 0 17 17 RangeError
6 2 1 17 17 RangeError
6 2 2  RangeError
6 2 3  RangeError
6 2 4  RangeError
6 2 5  RangeError
6 2 6  RangeError
6 2 7  RangeError
6 3 0 17 17 34 34
6 3 1 17 17 34 34
6 3 2 8721 4386 RangeError
6 3 3 8721 4386 RangeError
6 3 4  RangeError
6 3 5  RangeError
6 3 6  RangeError
6 3 7  RangeError
6 4 0 17 17 34 34
6 4 1 17 17 34 34
6 4 2 8721 4386 13090 8755
6 4 3 8721 4386 13090 8755
6 4 4  RangeError
6 4 5  RangeError
6 4 6  RangeError
6 4 7  RangeError
6 5 0 17 17 34 34
6 5 1 17 17 34 34
6 5 2 8721 4386 13090 8755
6 5 3 8721 4386 13090 8755
6 5 4 1144201745 287454020 RangeError
6 5 5 1144201745 287454020 RangeError
6 5 6 716.5322875976563 1.2795344104949228e-28 RangeError
6 5 7  RangeError
6 6 0 17 17 34 34
6 6 1 17 17 34 34
6 6 2 8721 4386 13090 8755
6 6 3 8721 4386 13090 8755
6 6 4 1144201745 287454020 1430532898 573785173
6 6 5 1144201745 287454020 1430532898 573785173
6 6 6 716.5322875976563 1.2795344104949228e-28 13482743300096 2.4295198285501637e-18
6 6 7  RangeError
6 7 0 17 17 34 34
6 7 1 17 17 34 34
6 7 2 8721 4386 13090 8755
6 7 3 8721 4386 13090 8755
6 7 4 1144201745 287454020 1430532898 573785173
6 7 5 1144201745 287454020 1430532898 573785173
6 7 6 716.5322875976563 1.2795344104949228e-28 13482743300096 2.4295198285501637e-18
6 7 7 -7.086876636573014e-268 3.841412024471731e-226 RangeError
6 8 0  RangeError
6 8 1  RangeError
6 8 2  RangeError
6 8 3  RangeError
6 8 4  RangeError
6 8 5  RangeError
6 8 6  RangeError
6 8 7  RangeError
7 0 0 102 102 119 119
7 0 1 102 102 119 119
7 0 2 30566 26231 -30601 30600
7 0 3 30566 26231 34935 30600
7 0 4 -1719109786 1719109785 RangeError
7 0 5 2575857510 1719109785 RangeError
7 0 6 -1.411029505825944e-23 2.9223607581867867e+23 RangeError
7 0 7  RangeError
7 1 0  RangeError
7 1 1  RangeError
7 1 2  RangeError
7 1 3  RangeError
7 1 4  RangeError
7 1 5  RangeError
7 1 6  RangeError
7 1 7  RangeError
7 2 0 102 102 RangeError
7 2 1 102 102 RangeError
7 2 2  RangeError
7 2 3  RangeError
7 2 4  RangeError
7 2 5  RangeError
7 2 6  RangeError
7 2 7  RangeError
7 3 0 102 102 119 119
7 3 1 102 102 119 119
7 3 2 30566 26231 RangeError
7 3 3 30566 26231 RangeError
7 3 4  RangeError
7 3 5  RangeError
7 3 6  RangeError
7 3 7  RangeError
7 4 0 102 102 119 119
7 4 1 102 102 119 119
7 4 2 30566 26231 -30601 30600
7 4 3 30566 26231 34935 30600
7 4 4  RangeError
7 4 5  RangeError
7 4 6  RangeError
7 4 7  RangeError
7 5 0 102 102 119 119
7 5 1 102 102 119 119
7 5 2 30566 26231 -30601 30600
7 5 3 30566 26231 34935 30600
7 5 4 -1719109786 1719109785 RangeError
7 5 5 2575857510 1719109785 RangeError
7 5 6 -1.411029505825944e-23 2.9223607581867867e+23 RangeError
7 5 7  RangeError
7 6 0  RangeError
7 6 1  RangeError
7 6 2  RangeError
7 6 3  RangeError
7 6 4  RangeError
7 6 5  RangeError
7 6 6  RangeError
7 6 7  RangeError
7 7 0  RangeError
7 7 1  RangeError
7 7 2  RangeError
7 7 3  RangeError
7 7 4  RangeError
7 7 5  RangeError
7 7 6  RangeError
7 7 7  RangeError
7 8 0  RangeError
7 8 1  RangeError
7 8 2  RangeError
7 8 3  RangeError
7 8 4  RangeError
7 8 5  RangeError
7 8 6  RangeError
7 8 7  RangeError
8 0 0 -120 -120 -103 -103
8 0 1 136 136 153 153
8 0 2 -26232 -30567 RangeError
8 0 3 39304 34969 RangeError
8 0 4  RangeError
8 0 5  RangeError
8 0 6  RangeError
8 0 7  RangeError
8 1 0  RangeError
8 1 1  RangeError
8 1 2  RangeError
8 1 3  RangeError
8 1 4  RangeError
8 1 5  RangeError
8 1 6  RangeError
8 1 7  RangeError
8 2 0 -120 -120 RangeError
8 2 1 136 136 RangeError
8 2 2  RangeError
8 2 3  RangeError
8 2 4  RangeError
8 2 5  RangeError
8 2 6  RangeError
8 2 7  RangeError
8 3 0 -120 -120 -103 -103
8 3 1 136 136 153 153
8 3 2 -26232 -30567 RangeError
8 3 3 39304 34969 RangeError
8 3 4  RangeError
8 3 5  RangeError
8 3 6  RangeError
8 3 7  RangeError
8 4 0  RangeError
8 4 1  RangeError
8 4 2  RangeError
8 4 3  RangeError
8 4 4  RangeError
8 4 5  RangeError
8 4 6  RangeError
8 4 7  RangeError
8 5 0  RangeError
8 5 1  RangeError
8 5 2  RangeError
8 5 3  RangeError
8 5 4  RangeError
8 5 5  RangeError
8 5 6  RangeError
8 5 7  RangeError
8 6 0  RangeError
8 6 1  RangeError
8 6 2  RangeError
8 6 3  RangeError
8 6 4  RangeError
8 6 5  RangeError
8 6 6  RangeError
8 6 7  RangeError
8 7 0  RangeError
8 7 1  RangeError
8 7 2  RangeError
8 7 3  RangeError
8 7 4  RangeError
8 7 5  RangeError
8 7 6  RangeError
8 7 7  RangeError
8 8 0  RangeError
8 8 1  RangeError
8 8 2  RangeError
8 8 3  RangeError
8 8 4  RangeError
8 8 5  RangeError
8 8 6  RangeError
8 8 7  RangeError
9 0 0 -103 -103 RangeError
9 0 1 153 153 RangeError
9 0 2  RangeError
9 0 3  RangeError
9 0 4  RangeError
9 0 5  RangeError
9 0 6  RangeError
9 0 7  RangeError
9 1 0  RangeError
9 1 1  RangeError
9 1 2  RangeError
9 1 3  RangeError
9 1 4  RangeError
9 1 5  RangeError
9 1 6  RangeError
9 1 7  RangeError
9 2 0 -103 -103 RangeError
9 2 1 153 153 RangeError
9 2 2  RangeError
9 2 3  RangeError
9 2 4  RangeError
9 2 5  RangeError
9 2 6  RangeError
9 2 7  RangeError
9 3 0  RangeError
9 3 1  RangeError
9 3 2  RangeError
9 3 3  RangeError
9 3 4  RangeError
9 3 5  RangeError
9 3 6  RangeError
9 3 7  RangeError
9 4 0  RangeError
9 4 1  RangeError
9 4 2  RangeError
9 4 3  RangeError
9 4 4  RangeError
9 4 5  RangeError
9 4 6  RangeError
9 4 7  RangeError
9 5 0  RangeError
9 5 1  RangeError
9 5 2  RangeError
9 5 3  RangeError
9 5 4  RangeError
9 5 5  RangeError
9 5 6  RangeError
9 5 7  RangeError
9 6 0  RangeError
9 6 1  RangeError
9 6 2  RangeError
9 6 3  RangeError
9 6 4  RangeError
9 6 5  RangeError
9 6 6  RangeError
9 6 7  RangeError
9 7 0  RangeError
9 7 1  RangeError
9 7 2  RangeError
9 7 3  RangeError
9 7 4  RangeError
9 7 5  RangeError
9 7 6  RangeError
9 7 7  RangeError
9 8 0  RangeError
9 8 1  RangeError
9 8 2  RangeError
9 8 3  RangeError
9 8 4  RangeError
9 8 5  RangeError
9 8 6  RangeError
9 8 7  RangeError
10 0 0  RangeError
10 0 1  RangeError
10 0 2  RangeError
10 0 3  RangeError
10 0 4  RangeError
10 0 5  RangeError
10 0 6  RangeError
10 0 7  RangeError
10 1 0  RangeError
10 1 1  RangeError
10 1 2  RangeError
10 1 3  RangeError
10 1 4  RangeError
10 1 5  RangeError
10 1 6  RangeError
10 1 7  RangeError
10 2 0  RangeError
10 2 1  RangeError
10 2 2  RangeError
10 2 3  RangeError
10 2 4  RangeError
10 2 5  RangeError
10 2 6  RangeError
10 2 7  RangeError
10 3 0  RangeError
10 3 1  RangeError
10 3 2  RangeError
10 3 3  RangeError
10 3 4  RangeError
10 3 5  RangeError
10 3 6  RangeError
10 3 7  RangeError
10 4 0  RangeError
10 4 1  RangeError
10 4 2  RangeError
10 4 3  RangeError
10 4 4  RangeError
10 4 5  RangeError
10 4 6  RangeError
10 4 7  RangeError
10 5 0  RangeError
10 5 1  RangeError
10 5 2  RangeError
10 5 3  RangeError
10 5 4  RangeError
10 5 5  RangeError
10 5 6  RangeError
10 5 7  RangeError
10 6 0  RangeError
10 6 1  RangeError
10 6 2  RangeError
10 6 3  RangeError
10 6 4  RangeError
10 6 5  RangeError
10 6 6  RangeError
10 6 7  RangeError
10 7 0  RangeError
10 7 1  RangeError
10 7 2  RangeError
10 7 3  RangeError
10 7 4  RangeError
10 7 5  RangeError
10 7 6  RangeError
10 7 7  RangeError
10 8 0  RangeError
10 8 1  RangeError
10 8 2  RangeError
10 8 3  RangeError
10 8 4  RangeError
10 8 5  RangeError
10 8 6  RangeError
10 8 7  RangeError
11 0 0  RangeError
11 0 1  RangeError
11 0 2  RangeError
11 0 3  RangeError
11 0 4  RangeError
11 0 5  RangeError
11 0 6  RangeError
11 0 7  RangeError
11 1 0  RangeError
11 1 1  RangeError
11 1 2  RangeError
11 1 3  RangeError
11 1 4  RangeError
11 1 5  RangeError
11 1 6  RangeError
11 1 7  RangeError
11 2 0  RangeError
11 2 1  RangeError
11 2 2  RangeError
11 2 3  RangeError
11 2 4  RangeError
11 2 5  RangeError
11 2 6  RangeError
11 2 7  RangeError
11 3 0  RangeError
11 3 1  RangeError
11 3 2  RangeError
11 3 3  RangeError
11 3 4  RangeError
11 3 5  RangeError
11 3 6  RangeError
11 3 7  RangeError
11 4 0  RangeError
11 4 1  RangeError
11 4 2  RangeError
11 4 3  RangeError
11 4 4  RangeError
11 4 5  RangeError
11 4 6  RangeError
11 4 7  RangeError
11 5 0  RangeError
11 5 1  RangeError
11 5 2  RangeError
11 5 3  RangeError
11 5 4  RangeError
11 5 5  RangeError
11 5 6  RangeError
11 5 7  RangeError
11 6 0  RangeError
11 6 1  RangeError
11 6 2  RangeError
11 6 3  RangeError
11 6 4  RangeError
11 6 5  RangeError
11 6 6  RangeError
11 6 7  RangeError
11 7 0  RangeError
11 7 1  RangeError
11 7 2  RangeError
11 7 3  RangeError
11 7 4  RangeError
11 7 5  RangeError
11 7 6  RangeError
11 7 7  RangeError
11 8 0  RangeError
11 8 1  RangeError
11 8 2  RangeError
11 8 3  RangeError
11 8 4  RangeError
11 8 5  RangeError
11 8 6  RangeError
11 8 7  RangeError
===*/

function readDataViewTest(arrayLength) {
    // DEADBEEF CAFEBABE 11223344 55667788 99
    var b = new ArrayBuffer(arrayLength);
    var b_u8 = new Uint8Array(b);
    [
        0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
        0x99
    ].forEach(function (v, i) {
        if (i < arrayLength) {
            b_u8[i] = v;
        }
    });
    printTypedArray(b);

    [ -100, -5, -1, 0, 1, 2, 8, 13, 15, 16, 17, 100 ].forEach(function (offset, idx1) {
        [ undefined, 0, 1, 2, 3, 4, 5, 8, 100 ].forEach(function (length, idx2) {
            [ 'getInt8', 'getUint8', 'getInt16', 'getUint16',
              'getInt32', 'getUint32', 'getFloat32', 'getFloat64' ].forEach(function (funcname, idx3) {
                var evalstr;

                evalstr = 'new DataView(b, ' + offset;
                if (length !== undefined) {
                    evalstr += ', ' + length;
                }
                evalstr += ')';

                var vals = [];
                try {
                    // workaround because there's no programmatic 'construct' call

                    var v = eval(evalstr);
                    vals.push(v[funcname](0, true));
                    vals.push(v[funcname](0, false));
                    vals.push(v[funcname](1, true));
                    vals.push(v[funcname](1, false));
                    print(idx1, idx2, idx3, vals.join(' '));
                } catch (e) {
                    print(idx1, idx2, idx3, vals.join(' '), e.name);
                }
            });
        });
    });
}

try {
    /* There are no alignment requirements for DataView, but test two
     * lengths anyway.
     */

    print('read TypedArray test, arrayLength 16');
    readDataViewTest(16);

    print('read TypedArray test, arrayLength 17');
    readDataViewTest(17);
} catch (e) {
    print(e.stack || e);
}
