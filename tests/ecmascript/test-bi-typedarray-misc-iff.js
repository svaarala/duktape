/*
 *  Create an IFF ILBM image using TypedArray bindings.
 *
 *  The output is base64 encoded to avoid terminal issues; decode as:
 *
 *      $ cat output.txt | base64 -d > /tmp/test.iff
 *
 *  You probably need to convert to view:
 *
 *      $ convert /tmp/test.iff /tmp/test.png
 *      $ display /tmp/test.png
 *
 *  http://fileformats.archiveteam.org/wiki/ILBM
 *  http://wiki.amigaos.net/wiki/ILBM_IFF_Interleaved_Bitmap
 *  http://www.fileformat.info/format/iff/egff.htm
 */

/*===
Rk9STQAAAFZJTEJNQk1IRAAAABQAEAAQAAAAAAEAAAAAAGRkAUAAyENNQVAAAAAGAAAA//8AQk9EWQAAACAAADPMD/ADwAP8D8wP/D/MM8w//zPwAzAPMAwwDzwAAA==
===*/

function iffTest() {
    var buf = new ArrayBuffer(16384);
    var buf_u8 = new Uint8Array(buf);
    var buf_dv = new DataView(buf);
    var off = 0;
    var i, v;

    var bmhd_chunk = new ArrayBuffer(20);
    v = new DataView(bmhd_chunk);
    v.setUint16(0, 16);    // width
    v.setUint16(2, 16);    // height
    v.setUint16(4, 0);     // left
    v.setUint16(6, 0);     // top
    v.setUint8(8, 1);      // bitplanes
    v.setUint8(9, 0);      // masking
    v.setUint8(10, 0);     // compress
    v.setUint8(11, 0);     // padding
    v.setUint16(12, 0);    // transparency
    v.setUint8(14, 100);   // x aspect ratio
    v.setUint8(15, 100);   // y aspect ratio
    v.setUint16(16, 320);  // page width
    v.setUint16(18, 200);  // page height

    var cmap_chunk = new ArrayBuffer(2 * 3);
    v = new Uint8Array(cmap_chunk);
    v[0] = 0;    // color 0: black
    v[1] = 0;
    v[2] = 0;
    v[3] = 255;    // color 1: yellow
    v[4] = 255;
    v[5] = 0;

    var body_chunk = new ArrayBuffer((16 / 8) * 16);
    v = new DataView(body_chunk);
    [
        0x0000, 0x33cc, 0x0ff0, 0x03c0,
        0x03fc, 0x0fcc, 0x0ffc, 0x3fcc,
        0x33cc, 0x3fff, 0x33f0, 0x0330,
        0x0f30, 0x0c30, 0x0f3c, 0x0000
    ].forEach(function (x, i) {
        v.setUint16(i * 2, x);
    });

    var chunks = [
        {
            type: 'FORM',
            data: [
                'ILBM',
                {
                    type: 'BMHD',
                    data: bmhd_chunk
                },
                {
                    type: 'CMAP',
                    data: cmap_chunk
                },
                {
                    type: 'BODY',
                    data: body_chunk
                }
            ]
        }
    ];

    function write(v) {
        var off_len, off_start, data_len;
        var i;
        if (typeof v === 'string') {
            for (i = 0; i < v.length; i++) {
                buf_u8[off++] = v.charCodeAt(i);
            }
        } else if (v instanceof ArrayBuffer) {
            buf_u8.set(v, off);
            off += v.byteLength;
        } else if (v instanceof Array) {
            v.forEach(function (elem) { write(elem); });
        } else if (v instanceof Object) {
            if (typeof v.type !== 'string' || v.type.length !== 4) {
                throw new Error('object missing type or type invalid');
            }
            for (i = 0; i < v.type.length; i++) {
                buf_u8[off++] = v.type.charCodeAt(i);
            }
            off_len = off;
            off += 4;
            off_start = off;
            write(v.data);
            data_len = off - off_start;
            while (off & 0x01) {
                // pad to even length
                buf_u8[off++] = 0;
            }
            buf_dv.setUint32(off_len, data_len);
        } else {
            throw new Error('failed to encode, invalid value');
        }
    }

    write(chunks);
    var res = buf.slice(0, off);
    var res_u8 = new Uint8Array(res);

    if (typeof Duktape === 'object') {
        print(Duktape.enc('base64', Duktape.Buffer(res)));
    } else {
        // For Node.js
        var nodebuf = new Buffer(res.byteLength);
        for (i = 0; i < res.byteLength; i++) {
            nodebuf[i] = res_u8[i];
        }
        print(nodebuf.toString('base64'));
    }
}

try {
    iffTest();
} catch (e) {
    print(e.stack || e);
}
