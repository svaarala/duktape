/*
 *  Full coverage for decoding all half-floats.  Compared against cbor-js.
 */

/*===
f90000 0
f90001 5.960464477539063e-8
  diff: 5.960464477539063e-8
f90801 0.00012218952178955078
  diff: 1.1920928955078125e-7
f90c01 0.00024437904357910156
  diff: 2.384185791015625e-7
f91001 0.0004887580871582031
  diff: 4.76837158203125e-7
f91401 0.0009775161743164063
  diff: 9.5367431640625e-7
f91801 0.0019550323486328125
  diff: 0.0000019073486328125
f91c01 0.003910064697265625
  diff: 0.000003814697265625
f92001 0.00782012939453125
  diff: 0.00000762939453125
f92401 0.0156402587890625
  diff: 0.0000152587890625
f92801 0.031280517578125
  diff: 0.000030517578125
f92c01 0.06256103515625
  diff: 0.00006103515625
f93001 0.1251220703125
  diff: 0.0001220703125
f93401 0.250244140625
  diff: 0.000244140625
f93801 0.50048828125
  diff: 0.00048828125
f93c01 1.0009765625
  diff: 0.0009765625
f94001 2.001953125
  diff: 0.001953125
f94401 4.00390625
  diff: 0.00390625
f94801 8.0078125
  diff: 0.0078125
f94c01 16.015625
  diff: 0.015625
f95001 32.03125
  diff: 0.03125
f95401 64.0625
  diff: 0.0625
f95801 128.125
  diff: 0.125
f95c01 256.25
  diff: 0.25
f96001 512.5
  diff: 0.5
f96401 1025
  diff: 1
f96801 2050
  diff: 2
f96c01 4100
  diff: 4
f97001 8200
  diff: 8
f97401 16400
  diff: 16
f97801 32800
  diff: 32
f97c00 Infinity
f97c01 NaN
f97c02 NaN
  diff: 0
f98000 -0
f98001 -5.960464477539063e-8
  diff: -5.960464477539063e-8
f98801 -0.00012218952178955078
  diff: -1.1920928955078125e-7
f98c01 -0.00024437904357910156
  diff: -2.384185791015625e-7
f99001 -0.0004887580871582031
  diff: -4.76837158203125e-7
f99401 -0.0009775161743164063
  diff: -9.5367431640625e-7
f99801 -0.0019550323486328125
  diff: -0.0000019073486328125
f99c01 -0.003910064697265625
  diff: -0.000003814697265625
f9a001 -0.00782012939453125
  diff: -0.00000762939453125
f9a401 -0.0156402587890625
  diff: -0.0000152587890625
f9a801 -0.031280517578125
  diff: -0.000030517578125
f9ac01 -0.06256103515625
  diff: -0.00006103515625
f9b001 -0.1251220703125
  diff: -0.0001220703125
f9b401 -0.250244140625
  diff: -0.000244140625
f9b801 -0.50048828125
  diff: -0.00048828125
f9bc01 -1.0009765625
  diff: -0.0009765625
f9c001 -2.001953125
  diff: -0.001953125
f9c401 -4.00390625
  diff: -0.00390625
f9c801 -8.0078125
  diff: -0.0078125
f9cc01 -16.015625
  diff: -0.015625
f9d001 -32.03125
  diff: -0.03125
f9d401 -64.0625
  diff: -0.0625
f9d801 -128.125
  diff: -0.125
f9dc01 -256.25
  diff: -0.25
f9e001 -512.5
  diff: -0.5
f9e401 -1025
  diff: -1
f9e801 -2050
  diff: -2
f9ec01 -4100
  diff: -4
f9f001 -8200
  diff: -8
f9f401 -16400
  diff: -16
f9f801 -32800
  diff: -32
f9fc00 -Infinity
f9fc01 NaN
f9fc02 NaN
  diff: 0
done
===*/

function test() {
    var b1, b2, prev = null, prevdiff = null;

    var res = [];

    for (b1 = 0; b1 < 256; b1++) {
        for (b2 = 0; b2 < 256; b2++) {
            var data = new Uint8Array([ 0xf9, b1, b2 ]).buffer;
            var val = CBOR.decode(data);
            var obj = { input: Duktape.enc('hex', data), value: val };
            if (prev !== null && Number.isNaN(prev) && Number.isNaN(val)) {
                obj.diff = 0;
            } else if (prev !== null && Number.isFinite(prev) && Number.isFinite(val)) {
                obj.diff = val - prev;
            }
            prev = val;
            res.push(obj);
        }
    }

    var diff_printed = false;
    for (i = 0; i < res.length; i++) {
        var prev = (i > 0 ? res[i - 1] : {});
        var curr = res[i];
        if (prev.diff === curr.diff && typeof curr.diff === 'number') {
            if (diff_printed) {
            } else {
                print('  diff: ' + prev.diff);
                diff_printed = true;
            }
        } else {
            print(curr.input, Duktape.enc('jx', curr.value));
            diff_printed = false;
        }
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}

print('done');
