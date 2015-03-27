/*
 *  Read fixed size field
 */

/*@include util-nodejs-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/* Custom because of Node.js differences, and a few numconv rounding issues. */

/*===
read field test
16
16 bytes: deadbeefcafebabe1122334455667788
false -100 readUInt8 RangeError
false -100 readUInt16LE RangeError
false -100 readUInt16BE RangeError
false -100 readUInt32LE RangeError
false -100 readUInt32BE RangeError
false -100 readInt8 RangeError
false -100 readInt16LE RangeError
false -100 readInt16BE RangeError
false -100 readInt32LE RangeError
false -100 readInt32BE RangeError
false -100 readFloatLE RangeError
false -100 readFloatBE RangeError
false -100 readDoubleLE RangeError
false -100 readDoubleBE RangeError
false -10 readUInt8 RangeError
false -10 readUInt16LE RangeError
false -10 readUInt16BE RangeError
false -10 readUInt32LE RangeError
false -10 readUInt32BE RangeError
false -10 readInt8 RangeError
false -10 readInt16LE RangeError
false -10 readInt16BE RangeError
false -10 readInt32LE RangeError
false -10 readInt32BE RangeError
false -10 readFloatLE RangeError
false -10 readFloatBE RangeError
false -10 readDoubleLE RangeError
false -10 readDoubleBE RangeError
false -9 readUInt8 RangeError
false -9 readUInt16LE RangeError
false -9 readUInt16BE RangeError
false -9 readUInt32LE RangeError
false -9 readUInt32BE RangeError
false -9 readInt8 RangeError
false -9 readInt16LE RangeError
false -9 readInt16BE RangeError
false -9 readInt32LE RangeError
false -9 readInt32BE RangeError
false -9 readFloatLE RangeError
false -9 readFloatBE RangeError
false -9 readDoubleLE RangeError
false -9 readDoubleBE RangeError
false -8 readUInt8 RangeError
false -8 readUInt16LE RangeError
false -8 readUInt16BE RangeError
false -8 readUInt32LE RangeError
false -8 readUInt32BE RangeError
false -8 readInt8 RangeError
false -8 readInt16LE RangeError
false -8 readInt16BE RangeError
false -8 readInt32LE RangeError
false -8 readInt32BE RangeError
false -8 readFloatLE RangeError
false -8 readFloatBE RangeError
false -8 readDoubleLE RangeError
false -8 readDoubleBE RangeError
false -7 readUInt8 RangeError
false -7 readUInt16LE RangeError
false -7 readUInt16BE RangeError
false -7 readUInt32LE RangeError
false -7 readUInt32BE RangeError
false -7 readInt8 RangeError
false -7 readInt16LE RangeError
false -7 readInt16BE RangeError
false -7 readInt32LE RangeError
false -7 readInt32BE RangeError
false -7 readFloatLE RangeError
false -7 readFloatBE RangeError
false -7 readDoubleLE RangeError
false -7 readDoubleBE RangeError
false -6 readUInt8 RangeError
false -6 readUInt16LE RangeError
false -6 readUInt16BE RangeError
false -6 readUInt32LE RangeError
false -6 readUInt32BE RangeError
false -6 readInt8 RangeError
false -6 readInt16LE RangeError
false -6 readInt16BE RangeError
false -6 readInt32LE RangeError
false -6 readInt32BE RangeError
false -6 readFloatLE RangeError
false -6 readFloatBE RangeError
false -6 readDoubleLE RangeError
false -6 readDoubleBE RangeError
false -5 readUInt8 RangeError
false -5 readUInt16LE RangeError
false -5 readUInt16BE RangeError
false -5 readUInt32LE RangeError
false -5 readUInt32BE RangeError
false -5 readInt8 RangeError
false -5 readInt16LE RangeError
false -5 readInt16BE RangeError
false -5 readInt32LE RangeError
false -5 readInt32BE RangeError
false -5 readFloatLE RangeError
false -5 readFloatBE RangeError
false -5 readDoubleLE RangeError
false -5 readDoubleBE RangeError
false -4 readUInt8 RangeError
false -4 readUInt16LE RangeError
false -4 readUInt16BE RangeError
false -4 readUInt32LE RangeError
false -4 readUInt32BE RangeError
false -4 readInt8 RangeError
false -4 readInt16LE RangeError
false -4 readInt16BE RangeError
false -4 readInt32LE RangeError
false -4 readInt32BE RangeError
false -4 readFloatLE RangeError
false -4 readFloatBE RangeError
false -4 readDoubleLE RangeError
false -4 readDoubleBE RangeError
false -3 readUInt8 RangeError
false -3 readUInt16LE RangeError
false -3 readUInt16BE RangeError
false -3 readUInt32LE RangeError
false -3 readUInt32BE RangeError
false -3 readInt8 RangeError
false -3 readInt16LE RangeError
false -3 readInt16BE RangeError
false -3 readInt32LE RangeError
false -3 readInt32BE RangeError
false -3 readFloatLE RangeError
false -3 readFloatBE RangeError
false -3 readDoubleLE RangeError
false -3 readDoubleBE RangeError
false -2 readUInt8 RangeError
false -2 readUInt16LE RangeError
false -2 readUInt16BE RangeError
false -2 readUInt32LE RangeError
false -2 readUInt32BE RangeError
false -2 readInt8 RangeError
false -2 readInt16LE RangeError
false -2 readInt16BE RangeError
false -2 readInt32LE RangeError
false -2 readInt32BE RangeError
false -2 readFloatLE RangeError
false -2 readFloatBE RangeError
false -2 readDoubleLE RangeError
false -2 readDoubleBE RangeError
false -1 readUInt8 RangeError
false -1 readUInt16LE RangeError
false -1 readUInt16BE RangeError
false -1 readUInt32LE RangeError
false -1 readUInt32BE RangeError
false -1 readInt8 RangeError
false -1 readInt16LE RangeError
false -1 readInt16BE RangeError
false -1 readInt32LE RangeError
false -1 readInt32BE RangeError
false -1 readFloatLE RangeError
false -1 readFloatBE RangeError
false -1 readDoubleLE RangeError
false -1 readDoubleBE RangeError
false 0 readUInt8 222
false 0 readUInt16LE 44510
false 0 readUInt16BE 57005
false 0 readUInt32LE 4022250974
false 0 readUInt32BE 3735928559
false 0 readInt8 -34
false 0 readInt16LE -21026
false 0 readInt16BE -8531
false 0 readInt32LE -272716322
false 0 readInt32BE -559038737
false 0 readFloatLE -1.1802468879641618e+29
false 0 readFloatBE -6259853398707798000
false 0 readDoubleLE -0.0000016090443173227715
false 0 readDoubleBE -1.1885958788264018e+148
false 1 readUInt8 173
false 1 readUInt16LE 48813
false 1 readUInt16BE 44478
false 1 readUInt32LE 3404709549
false 1 readUInt32BE 2914971594
false 1 readInt8 -83
false 1 readInt16LE -16723
false 1 readInt16BE -21058
false 1 readInt32LE -890257747
false 1 readInt32BE -1379995702
false 1 readFloatLE -7855958.5
false 1 readFloatBE -2.1706986902403358e-11
false 1 readDoubleLE 3.3208721076639256e-223
false 1 readDoubleBE -2.4299377383893497e-88
false 2 readUInt8 190
false 2 readUInt16LE 61374
false 2 readUInt16BE 48879
false 2 readUInt32LE 4274712510
false 2 readUInt32BE 3203386110
false 2 readInt8 -66
false 2 readInt16LE -4162
false 2 readInt16BE -16657
false 2 readInt32LE -20254786
false 2 readInt32BE -1091581186
false 2 readFloatLE -1.3487443387778376e+38
false 2 readFloatBE -0.468345582485199
false 2 readDoubleLE 1.4210818500899943e-144
false 2 readDoubleBE -0.000015160059625215153
false 3 readUInt8 239
false 3 readUInt16LE 51951
false 3 readUInt16BE 61386
false 3 readUInt32LE 3137260271
false 3 readUInt32BE 4023058106
false 3 readInt8 -17
false 3 readInt16LE -13585
false 3 readInt16BE -4150
false 3 readInt32LE -1157707025
false 3 readInt32BE -271909190
false 3 readFloatLE -0.001943914103321731
false 3 readFloatBE -1.2564783500451927e+29
false 3 readDoubleLE 2.196203770486694e-62
false 3 readDoubleBE -3.2742408819041713e+230
false 8 readUInt8 17
false 8 readUInt16LE 8721
false 8 readUInt16BE 4386
false 8 readUInt32LE 1144201745
false 8 readUInt32BE 287454020
false 8 readInt8 17
false 8 readInt16LE 8721
false 8 readInt16BE 4386
false 8 readInt32LE 1144201745
false 8 readInt32BE 287454020
false 8 readFloatLE 716.5322875976563
false 8 readFloatBE 1.2795344104949228e-28
false 8 readDoubleLE -7.086876636573014e-268
false 8 readDoubleBE 3.841412024471731e-226
false 9 readUInt8 34
false 9 readUInt16LE 13090
false 9 readUInt16BE 8755
false 9 readUInt32LE 1430532898
false 9 readUInt32BE 573785173
false 9 readInt8 34
false 9 readInt16LE 13090
false 9 readInt16BE 8755
false 9 readInt32LE 1430532898
false 9 readInt32BE 573785173
false 9 readFloatLE 13482743300096
false 9 readFloatBE 2.4295198285501637e-18
false 9 readDoubleLE RangeError
false 9 readDoubleBE RangeError
false 10 readUInt8 51
false 10 readUInt16LE 17459
false 10 readUInt16BE 13124
false 10 readUInt32LE 1716864051
false 10 readUInt32BE 860116326
false 10 readInt8 51
false 10 readInt16LE 17459
false 10 readInt16BE 13124
false 10 readInt32LE 1716864051
false 10 readInt32BE 860116326
false 10 readFloatLE 2.5178052859638565e+23
false 10 readFloatBE 4.5712475582604384e-8
false 10 readDoubleLE RangeError
false 10 readDoubleBE RangeError
false 11 readUInt8 68
false 11 readUInt16LE 21828
false 11 readUInt16BE 17493
false 11 readUInt32LE 2003195204
false 11 readUInt32BE 1146447479
false 11 readInt8 68
false 11 readInt16LE 21828
false 11 readInt16BE 17493
false 11 readInt32LE 2003195204
false 11 readInt32BE 1146447479
false 11 readFloatLE 4.6717096476342645e+33
false 11 readFloatBE 853.6010131835938
false 11 readDoubleLE RangeError
false 11 readDoubleBE RangeError
false 12 readUInt8 85
false 12 readUInt16LE 26197
false 12 readUInt16BE 21862
false 12 readUInt32LE 2289526357
false 12 readUInt32BE 1432778632
false 12 readInt8 85
false 12 readInt16LE 26197
false 12 readInt16BE 21862
false 12 readInt32LE -2005440939
false 12 readInt32BE 1432778632
false 12 readFloatLE -7.444914951583743e-34
false 12 readFloatBE 15837566074880
false 12 readDoubleLE RangeError
false 12 readDoubleBE RangeError
false 13 readUInt8 102
false 13 readUInt16LE 30566
false 13 readUInt16BE 26231
false 13 readUInt32LE RangeError
false 13 readUInt32BE RangeError
false 13 readInt8 102
false 13 readInt16LE 30566
false 13 readInt16BE 26231
false 13 readInt32LE RangeError
false 13 readInt32BE RangeError
false 13 readFloatLE RangeError
false 13 readFloatBE RangeError
false 13 readDoubleLE RangeError
false 13 readDoubleBE RangeError
false 14 readUInt8 119
false 14 readUInt16LE 34935
false 14 readUInt16BE 30600
false 14 readUInt32LE RangeError
false 14 readUInt32BE RangeError
false 14 readInt8 119
false 14 readInt16LE -30601
false 14 readInt16BE 30600
false 14 readInt32LE RangeError
false 14 readInt32BE RangeError
false 14 readFloatLE RangeError
false 14 readFloatBE RangeError
false 14 readDoubleLE RangeError
false 14 readDoubleBE RangeError
false 15 readUInt8 136
false 15 readUInt16LE RangeError
false 15 readUInt16BE RangeError
false 15 readUInt32LE RangeError
false 15 readUInt32BE RangeError
false 15 readInt8 -120
false 15 readInt16LE RangeError
false 15 readInt16BE RangeError
false 15 readInt32LE RangeError
false 15 readInt32BE RangeError
false 15 readFloatLE RangeError
false 15 readFloatBE RangeError
false 15 readDoubleLE RangeError
false 15 readDoubleBE RangeError
false 16 readUInt8 RangeError
false 16 readUInt16LE RangeError
false 16 readUInt16BE RangeError
false 16 readUInt32LE RangeError
false 16 readUInt32BE RangeError
false 16 readInt8 RangeError
false 16 readInt16LE RangeError
false 16 readInt16BE RangeError
false 16 readInt32LE RangeError
false 16 readInt32BE RangeError
false 16 readFloatLE RangeError
false 16 readFloatBE RangeError
false 16 readDoubleLE RangeError
false 16 readDoubleBE RangeError
false 100 readUInt8 RangeError
false 100 readUInt16LE RangeError
false 100 readUInt16BE RangeError
false 100 readUInt32LE RangeError
false 100 readUInt32BE RangeError
false 100 readInt8 RangeError
false 100 readInt16LE RangeError
false 100 readInt16BE RangeError
false 100 readInt32LE RangeError
false 100 readInt32BE RangeError
false 100 readFloatLE RangeError
false 100 readFloatBE RangeError
false 100 readDoubleLE RangeError
false 100 readDoubleBE RangeError
true -100 readUInt8 NaN
true -100 readUInt16LE NaN
true -100 readUInt16BE NaN
true -100 readUInt32LE NaN
true -100 readUInt32BE NaN
true -100 readInt8 NaN
true -100 readInt16LE NaN
true -100 readInt16BE NaN
true -100 readInt32LE NaN
true -100 readInt32BE NaN
true -100 readFloatLE NaN
true -100 readFloatBE NaN
true -100 readDoubleLE NaN
true -100 readDoubleBE NaN
true -10 readUInt8 NaN
true -10 readUInt16LE NaN
true -10 readUInt16BE NaN
true -10 readUInt32LE NaN
true -10 readUInt32BE NaN
true -10 readInt8 NaN
true -10 readInt16LE NaN
true -10 readInt16BE NaN
true -10 readInt32LE NaN
true -10 readInt32BE NaN
true -10 readFloatLE NaN
true -10 readFloatBE NaN
true -10 readDoubleLE NaN
true -10 readDoubleBE NaN
true -9 readUInt8 NaN
true -9 readUInt16LE NaN
true -9 readUInt16BE NaN
true -9 readUInt32LE NaN
true -9 readUInt32BE NaN
true -9 readInt8 NaN
true -9 readInt16LE NaN
true -9 readInt16BE NaN
true -9 readInt32LE NaN
true -9 readInt32BE NaN
true -9 readFloatLE NaN
true -9 readFloatBE NaN
true -9 readDoubleLE NaN
true -9 readDoubleBE NaN
true -8 readUInt8 NaN
true -8 readUInt16LE NaN
true -8 readUInt16BE NaN
true -8 readUInt32LE NaN
true -8 readUInt32BE NaN
true -8 readInt8 NaN
true -8 readInt16LE NaN
true -8 readInt16BE NaN
true -8 readInt32LE NaN
true -8 readInt32BE NaN
true -8 readFloatLE NaN
true -8 readFloatBE NaN
true -8 readDoubleLE NaN
true -8 readDoubleBE NaN
true -7 readUInt8 NaN
true -7 readUInt16LE NaN
true -7 readUInt16BE NaN
true -7 readUInt32LE NaN
true -7 readUInt32BE NaN
true -7 readInt8 NaN
true -7 readInt16LE NaN
true -7 readInt16BE NaN
true -7 readInt32LE NaN
true -7 readInt32BE NaN
true -7 readFloatLE NaN
true -7 readFloatBE NaN
true -7 readDoubleLE NaN
true -7 readDoubleBE NaN
true -6 readUInt8 NaN
true -6 readUInt16LE NaN
true -6 readUInt16BE NaN
true -6 readUInt32LE NaN
true -6 readUInt32BE NaN
true -6 readInt8 NaN
true -6 readInt16LE NaN
true -6 readInt16BE NaN
true -6 readInt32LE NaN
true -6 readInt32BE NaN
true -6 readFloatLE NaN
true -6 readFloatBE NaN
true -6 readDoubleLE NaN
true -6 readDoubleBE NaN
true -5 readUInt8 NaN
true -5 readUInt16LE NaN
true -5 readUInt16BE NaN
true -5 readUInt32LE NaN
true -5 readUInt32BE NaN
true -5 readInt8 NaN
true -5 readInt16LE NaN
true -5 readInt16BE NaN
true -5 readInt32LE NaN
true -5 readInt32BE NaN
true -5 readFloatLE NaN
true -5 readFloatBE NaN
true -5 readDoubleLE NaN
true -5 readDoubleBE NaN
true -4 readUInt8 NaN
true -4 readUInt16LE NaN
true -4 readUInt16BE NaN
true -4 readUInt32LE NaN
true -4 readUInt32BE NaN
true -4 readInt8 NaN
true -4 readInt16LE NaN
true -4 readInt16BE NaN
true -4 readInt32LE NaN
true -4 readInt32BE NaN
true -4 readFloatLE NaN
true -4 readFloatBE NaN
true -4 readDoubleLE NaN
true -4 readDoubleBE NaN
true -3 readUInt8 NaN
true -3 readUInt16LE NaN
true -3 readUInt16BE NaN
true -3 readUInt32LE NaN
true -3 readUInt32BE NaN
true -3 readInt8 NaN
true -3 readInt16LE NaN
true -3 readInt16BE NaN
true -3 readInt32LE NaN
true -3 readInt32BE NaN
true -3 readFloatLE NaN
true -3 readFloatBE NaN
true -3 readDoubleLE NaN
true -3 readDoubleBE NaN
true -2 readUInt8 NaN
true -2 readUInt16LE NaN
true -2 readUInt16BE NaN
true -2 readUInt32LE NaN
true -2 readUInt32BE NaN
true -2 readInt8 NaN
true -2 readInt16LE NaN
true -2 readInt16BE NaN
true -2 readInt32LE NaN
true -2 readInt32BE NaN
true -2 readFloatLE NaN
true -2 readFloatBE NaN
true -2 readDoubleLE NaN
true -2 readDoubleBE NaN
true -1 readUInt8 NaN
true -1 readUInt16LE NaN
true -1 readUInt16BE NaN
true -1 readUInt32LE NaN
true -1 readUInt32BE NaN
true -1 readInt8 NaN
true -1 readInt16LE NaN
true -1 readInt16BE NaN
true -1 readInt32LE NaN
true -1 readInt32BE NaN
true -1 readFloatLE NaN
true -1 readFloatBE NaN
true -1 readDoubleLE NaN
true -1 readDoubleBE NaN
true 0 readUInt8 222
true 0 readUInt16LE 44510
true 0 readUInt16BE 57005
true 0 readUInt32LE 4022250974
true 0 readUInt32BE 3735928559
true 0 readInt8 -34
true 0 readInt16LE -21026
true 0 readInt16BE -8531
true 0 readInt32LE -272716322
true 0 readInt32BE -559038737
true 0 readFloatLE -1.1802468879641618e+29
true 0 readFloatBE -6259853398707798000
true 0 readDoubleLE -0.0000016090443173227715
true 0 readDoubleBE -1.1885958788264018e+148
true 1 readUInt8 173
true 1 readUInt16LE 48813
true 1 readUInt16BE 44478
true 1 readUInt32LE 3404709549
true 1 readUInt32BE 2914971594
true 1 readInt8 -83
true 1 readInt16LE -16723
true 1 readInt16BE -21058
true 1 readInt32LE -890257747
true 1 readInt32BE -1379995702
true 1 readFloatLE -7855958.5
true 1 readFloatBE -2.1706986902403358e-11
true 1 readDoubleLE 3.3208721076639256e-223
true 1 readDoubleBE -2.4299377383893497e-88
true 2 readUInt8 190
true 2 readUInt16LE 61374
true 2 readUInt16BE 48879
true 2 readUInt32LE 4274712510
true 2 readUInt32BE 3203386110
true 2 readInt8 -66
true 2 readInt16LE -4162
true 2 readInt16BE -16657
true 2 readInt32LE -20254786
true 2 readInt32BE -1091581186
true 2 readFloatLE -1.3487443387778376e+38
true 2 readFloatBE -0.468345582485199
true 2 readDoubleLE 1.4210818500899943e-144
true 2 readDoubleBE -0.000015160059625215153
true 3 readUInt8 239
true 3 readUInt16LE 51951
true 3 readUInt16BE 61386
true 3 readUInt32LE 3137260271
true 3 readUInt32BE 4023058106
true 3 readInt8 -17
true 3 readInt16LE -13585
true 3 readInt16BE -4150
true 3 readInt32LE -1157707025
true 3 readInt32BE -271909190
true 3 readFloatLE -0.001943914103321731
true 3 readFloatBE -1.2564783500451927e+29
true 3 readDoubleLE 2.196203770486694e-62
true 3 readDoubleBE -3.2742408819041713e+230
true 8 readUInt8 17
true 8 readUInt16LE 8721
true 8 readUInt16BE 4386
true 8 readUInt32LE 1144201745
true 8 readUInt32BE 287454020
true 8 readInt8 17
true 8 readInt16LE 8721
true 8 readInt16BE 4386
true 8 readInt32LE 1144201745
true 8 readInt32BE 287454020
true 8 readFloatLE 716.5322875976563
true 8 readFloatBE 1.2795344104949228e-28
true 8 readDoubleLE -7.086876636573014e-268
true 8 readDoubleBE 3.841412024471731e-226
true 9 readUInt8 34
true 9 readUInt16LE 13090
true 9 readUInt16BE 8755
true 9 readUInt32LE 1430532898
true 9 readUInt32BE 573785173
true 9 readInt8 34
true 9 readInt16LE 13090
true 9 readInt16BE 8755
true 9 readInt32LE 1430532898
true 9 readInt32BE 573785173
true 9 readFloatLE 13482743300096
true 9 readFloatBE 2.4295198285501637e-18
true 9 readDoubleLE NaN
true 9 readDoubleBE NaN
true 10 readUInt8 51
true 10 readUInt16LE 17459
true 10 readUInt16BE 13124
true 10 readUInt32LE 1716864051
true 10 readUInt32BE 860116326
true 10 readInt8 51
true 10 readInt16LE 17459
true 10 readInt16BE 13124
true 10 readInt32LE 1716864051
true 10 readInt32BE 860116326
true 10 readFloatLE 2.5178052859638565e+23
true 10 readFloatBE 4.5712475582604384e-8
true 10 readDoubleLE NaN
true 10 readDoubleBE NaN
true 11 readUInt8 68
true 11 readUInt16LE 21828
true 11 readUInt16BE 17493
true 11 readUInt32LE 2003195204
true 11 readUInt32BE 1146447479
true 11 readInt8 68
true 11 readInt16LE 21828
true 11 readInt16BE 17493
true 11 readInt32LE 2003195204
true 11 readInt32BE 1146447479
true 11 readFloatLE 4.6717096476342645e+33
true 11 readFloatBE 853.6010131835938
true 11 readDoubleLE NaN
true 11 readDoubleBE NaN
true 12 readUInt8 85
true 12 readUInt16LE 26197
true 12 readUInt16BE 21862
true 12 readUInt32LE 2289526357
true 12 readUInt32BE 1432778632
true 12 readInt8 85
true 12 readInt16LE 26197
true 12 readInt16BE 21862
true 12 readInt32LE -2005440939
true 12 readInt32BE 1432778632
true 12 readFloatLE -7.444914951583743e-34
true 12 readFloatBE 15837566074880
true 12 readDoubleLE NaN
true 12 readDoubleBE NaN
true 13 readUInt8 102
true 13 readUInt16LE 30566
true 13 readUInt16BE 26231
true 13 readUInt32LE NaN
true 13 readUInt32BE NaN
true 13 readInt8 102
true 13 readInt16LE 30566
true 13 readInt16BE 26231
true 13 readInt32LE NaN
true 13 readInt32BE NaN
true 13 readFloatLE NaN
true 13 readFloatBE NaN
true 13 readDoubleLE NaN
true 13 readDoubleBE NaN
true 14 readUInt8 119
true 14 readUInt16LE 34935
true 14 readUInt16BE 30600
true 14 readUInt32LE NaN
true 14 readUInt32BE NaN
true 14 readInt8 119
true 14 readInt16LE -30601
true 14 readInt16BE 30600
true 14 readInt32LE NaN
true 14 readInt32BE NaN
true 14 readFloatLE NaN
true 14 readFloatBE NaN
true 14 readDoubleLE NaN
true 14 readDoubleBE NaN
true 15 readUInt8 136
true 15 readUInt16LE NaN
true 15 readUInt16BE NaN
true 15 readUInt32LE NaN
true 15 readUInt32BE NaN
true 15 readInt8 -120
true 15 readInt16LE NaN
true 15 readInt16BE NaN
true 15 readInt32LE NaN
true 15 readInt32BE NaN
true 15 readFloatLE NaN
true 15 readFloatBE NaN
true 15 readDoubleLE NaN
true 15 readDoubleBE NaN
true 16 readUInt8 NaN
true 16 readUInt16LE NaN
true 16 readUInt16BE NaN
true 16 readUInt32LE NaN
true 16 readUInt32BE NaN
true 16 readInt8 NaN
true 16 readInt16LE NaN
true 16 readInt16BE NaN
true 16 readInt32LE NaN
true 16 readInt32BE NaN
true 16 readFloatLE NaN
true 16 readFloatBE NaN
true 16 readDoubleLE NaN
true 16 readDoubleBE NaN
true 100 readUInt8 NaN
true 100 readUInt16LE NaN
true 100 readUInt16BE NaN
true 100 readUInt32LE NaN
true 100 readUInt32BE NaN
true 100 readInt8 NaN
true 100 readInt16LE NaN
true 100 readInt16BE NaN
true 100 readInt32LE NaN
true 100 readInt32BE NaN
true 100 readFloatLE NaN
true 100 readFloatBE NaN
true 100 readDoubleLE NaN
true 100 readDoubleBE NaN
===*/

/* Read field (fixed size) tests. */

function readFieldTest() {
    // DEADBEEF CAFEBABE 11223344 55667788
    var b = new Buffer([
        0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
    ]);

    print(b.length);
    printNodejsBuffer(b);

    // Node.js note: when noAssert==true and reading integers (!) from
    // negative indices, the result varies between 0, NaN, and undefined.
    // Duktape uses NaN (although 0 would also be reasonable), causing
    // expect string differences.
    //
    // The noAssert==true expect string has been generated by running Node.js,
    // fixing a few differences, and then replacing all RangeErrors with
    // an expected NaN result value.

    [ false, true ].forEach(function (noAssert) {
        [ -100, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3,
          8, 9, 10, 11, 12, 13, 14, 15, 16, 100 ].forEach(function (offset) {
            [ 'readUInt8', 'readUInt16LE', 'readUInt16BE', 'readUInt32LE', 'readUInt32BE',
              'readInt8', 'readInt16LE', 'readInt16BE', 'readInt32LE', 'readInt32BE',
              'readFloatLE', 'readFloatBE', 'readDoubleLE', 'readDoubleBE' ].forEach(function (funcname) {
                try {
                    print(noAssert, offset, funcname, b[funcname](offset, noAssert));
                } catch (e) {
                    print(noAssert, offset, funcname, e.name);
                }
            });
        });
    });
}

try {
    print('read field test');
    readFieldTest();
} catch (e) {
    print(e.stack || e);
}

/*===
readfield noAssert coercion test
0 RangeError
1 RangeError
2 NaN
3 RangeError
4 NaN
5 RangeError
6 NaN
7 RangeError
===*/

function readFieldNoAssertCoercionTest() {
    [ undefined, null, true, false, 123, 0, 'foo', '' ].forEach(function (noAssert, idx) {
        var buf = new Buffer('ABCDEFGH');
        try {
            print(idx, buf.readUInt32BE(6, noAssert));
        } catch (e) {
            print(idx, e.name);
        }
    });
}

try {
    print('readfield noAssert coercion test');
    readFieldNoAssertCoercionTest();
} catch (e) {
    print(e.stack || e);
}
