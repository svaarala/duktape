/*
 *  Test compiler handling of CSVAR output shuffling.
 */

/*---
{
    "custom": true
}
---*/

/*===
globalFunc called: 0
globalFunc called: 1
globalFunc called: 2
globalFunc called: 3
globalFunc called: 4
globalFunc called: 5
globalFunc called: 6
globalFunc called: 7
globalFunc called: 8
globalFunc called: 9
globalFunc called: 10
globalFunc called: 11
globalFunc called: 12
globalFunc called: 13
globalFunc called: 14
globalFunc called: 15
globalFunc called: 16
globalFunc called: 17
globalFunc called: 18
globalFunc called: 19
globalFunc called: 20
globalFunc called: 21
globalFunc called: 22
globalFunc called: 23
globalFunc called: 24
globalFunc called: 25
globalFunc called: 26
globalFunc called: 27
globalFunc called: 28
globalFunc called: 29
globalFunc called: 30
globalFunc called: 31
globalFunc called: 32
globalFunc called: 33
globalFunc called: 34
globalFunc called: 35
globalFunc called: 36
globalFunc called: 37
globalFunc called: 38
globalFunc called: 39
globalFunc called: 40
globalFunc called: 41
globalFunc called: 42
globalFunc called: 43
globalFunc called: 44
globalFunc called: 45
globalFunc called: 46
globalFunc called: 47
globalFunc called: 48
globalFunc called: 49
globalFunc called: 50
globalFunc called: 51
globalFunc called: 52
globalFunc called: 53
globalFunc called: 54
globalFunc called: 55
globalFunc called: 56
globalFunc called: 57
globalFunc called: 58
globalFunc called: 59
globalFunc called: 60
globalFunc called: 61
globalFunc called: 62
globalFunc called: 63
globalFunc called: 64
globalFunc called: 65
globalFunc called: 66
globalFunc called: 67
globalFunc called: 68
globalFunc called: 69
globalFunc called: 70
globalFunc called: 71
globalFunc called: 72
globalFunc called: 73
globalFunc called: 74
globalFunc called: 75
globalFunc called: 76
globalFunc called: 77
globalFunc called: 78
globalFunc called: 79
globalFunc called: 80
globalFunc called: 81
globalFunc called: 82
globalFunc called: 83
globalFunc called: 84
globalFunc called: 85
globalFunc called: 86
globalFunc called: 87
globalFunc called: 88
globalFunc called: 89
globalFunc called: 90
globalFunc called: 91
globalFunc called: 92
globalFunc called: 93
globalFunc called: 94
globalFunc called: 95
globalFunc called: 96
globalFunc called: 97
globalFunc called: 98
globalFunc called: 99
globalFunc called: 100
globalFunc called: 101
globalFunc called: 102
globalFunc called: 103
globalFunc called: 104
globalFunc called: 105
globalFunc called: 106
globalFunc called: 107
globalFunc called: 108
globalFunc called: 109
globalFunc called: 110
globalFunc called: 111
globalFunc called: 112
globalFunc called: 113
globalFunc called: 114
globalFunc called: 115
globalFunc called: 116
globalFunc called: 117
globalFunc called: 118
globalFunc called: 119
globalFunc called: 120
globalFunc called: 121
globalFunc called: 122
globalFunc called: 123
globalFunc called: 124
globalFunc called: 125
globalFunc called: 126
globalFunc called: 127
globalFunc called: 128
globalFunc called: 129
globalFunc called: 130
globalFunc called: 131
globalFunc called: 132
globalFunc called: 133
globalFunc called: 134
globalFunc called: 135
globalFunc called: 136
globalFunc called: 137
globalFunc called: 138
globalFunc called: 139
globalFunc called: 140
globalFunc called: 141
globalFunc called: 142
globalFunc called: 143
globalFunc called: 144
globalFunc called: 145
globalFunc called: 146
globalFunc called: 147
globalFunc called: 148
globalFunc called: 149
globalFunc called: 150
globalFunc called: 151
globalFunc called: 152
globalFunc called: 153
globalFunc called: 154
globalFunc called: 155
globalFunc called: 156
globalFunc called: 157
globalFunc called: 158
globalFunc called: 159
globalFunc called: 160
globalFunc called: 161
globalFunc called: 162
globalFunc called: 163
globalFunc called: 164
globalFunc called: 165
globalFunc called: 166
globalFunc called: 167
globalFunc called: 168
globalFunc called: 169
globalFunc called: 170
globalFunc called: 171
globalFunc called: 172
globalFunc called: 173
globalFunc called: 174
globalFunc called: 175
globalFunc called: 176
globalFunc called: 177
globalFunc called: 178
globalFunc called: 179
globalFunc called: 180
globalFunc called: 181
globalFunc called: 182
globalFunc called: 183
globalFunc called: 184
globalFunc called: 185
globalFunc called: 186
globalFunc called: 187
globalFunc called: 188
globalFunc called: 189
globalFunc called: 190
globalFunc called: 191
globalFunc called: 192
globalFunc called: 193
globalFunc called: 194
globalFunc called: 195
globalFunc called: 196
globalFunc called: 197
globalFunc called: 198
globalFunc called: 199
globalFunc called: 200
globalFunc called: 201
globalFunc called: 202
globalFunc called: 203
globalFunc called: 204
globalFunc called: 205
globalFunc called: 206
globalFunc called: 207
globalFunc called: 208
globalFunc called: 209
globalFunc called: 210
globalFunc called: 211
globalFunc called: 212
globalFunc called: 213
globalFunc called: 214
globalFunc called: 215
globalFunc called: 216
globalFunc called: 217
globalFunc called: 218
globalFunc called: 219
globalFunc called: 220
globalFunc called: 221
globalFunc called: 222
globalFunc called: 223
globalFunc called: 224
globalFunc called: 225
globalFunc called: 226
globalFunc called: 227
globalFunc called: 228
globalFunc called: 229
globalFunc called: 230
globalFunc called: 231
globalFunc called: 232
globalFunc called: 233
globalFunc called: 234
globalFunc called: 235
globalFunc called: 236
globalFunc called: 237
globalFunc called: 238
globalFunc called: 239
globalFunc called: 240
globalFunc called: 241
globalFunc called: 242
globalFunc called: 243
globalFunc called: 244
globalFunc called: 245
globalFunc called: 246
globalFunc called: 247
globalFunc called: 248
globalFunc called: 249
globalFunc called: 250
globalFunc called: 251
globalFunc called: 252
globalFunc called: 253
globalFunc called: 254
globalFunc called: 255
262130 RangeError
262131 RangeError
262132 RangeError
262133 RangeError
262134 RangeError
262135 RangeError
262136 RangeError
262137 RangeError
262138 RangeError
262139 RangeError
262140 RangeError
262141 RangeError
262142 RangeError
262143 RangeError
262144 RangeError
262145 RangeError
262146 RangeError
262147 RangeError
262148 RangeError
262149 RangeError
===*/

function createFunc(count) {
    var res = [];
    var i;

    res.push('(function test' + count + '() {');
    for (i = 0; i < count; i++) {
        // Use dummy variables to control where the call setup target
        // (CSVAR 'a' field) goes.  We must cross the shuffle boundary
        // exactly, and also exercise the BC limit for shuffling which
        // has special handling in the compiler.
        res.push('var unused' + i + ' = 123;');
    }
    res.push('globalFunc(' + i + ');');
    res.push('})');
    return eval(res.join('\n'));
}

function test() {
    var i;

    globalFunc = function globalFunc(arg) {
        print('globalFunc called:', arg);
    };

    function test(count) {
        try {
            createFunc(count)();
        } catch (e) {
            print(count, e.name);
        }
    }

    for (i = 0; i < 256; i++) {
        test(i);
    }
    for (i = 262130; i < 262150; i++) {
        test(i);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
