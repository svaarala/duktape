/*
 *  Create an IFF ILBM image using Node.js Buffer binding.
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
Rk9STQAAAFZJTEJNQk1IRAAAABQAEAAQAAAAAAEAAAAAAGRkAUAAyENNQVAAAAAG////AAD/Qk9EWQAAACAAAAAwAPwD8D8A/PAw8DzwDzwD/AP8A/wPMAwMPDwAAA==
===*/

function iffTest() {
    var buf = new Buffer(16384);
    var off = 0;
    var i;

    var bmhd_chunk = new Buffer(20);
    bmhd_chunk.writeUInt16BE(16, 0);   // width
    bmhd_chunk.writeUInt16BE(16, 2);   // height
    bmhd_chunk.writeUInt16BE(0, 4);    // left
    bmhd_chunk.writeUInt16BE(0, 6);    // top
    bmhd_chunk.writeUInt8(1, 8);       // bitplanes
    bmhd_chunk.writeUInt8(0, 9);       // masking
    bmhd_chunk.writeUInt8(0, 10);      // compress
    bmhd_chunk.writeUInt8(0, 11);      // padding
    bmhd_chunk.writeUInt16BE(0, 12);   // transparency
    bmhd_chunk.writeUInt8(100, 14);    // x aspect ratio
    bmhd_chunk.writeUInt8(100, 15);    // y aspect ratio
    bmhd_chunk.writeUInt16BE(320, 16); // page width
    bmhd_chunk.writeUInt16BE(200, 18); // page height

    var cmap_chunk = new Buffer(2 * 3);
    cmap_chunk[0] = 255;  // color 0: white
    cmap_chunk[1] = 255;
    cmap_chunk[2] = 255;
    cmap_chunk[3] = 0;    // color 1: blue
    cmap_chunk[4] = 0;
    cmap_chunk[5] = 255;

    var ____ = 0x00, ___x = 0x03, __x_ = 0x0c, __xx = 0x0f;
    var _x__ = 0x30, _x_x = 0x33, _xx_ = 0x3c, _xxx = 0x3f;
    var x___ = 0xc0, x__x = 0xc3, x_x_ = 0xcc, x_xx = 0xcf;
    var xx__ = 0xf0, xx_x = 0xf3, xxx_ = 0xfc, xxxx = 0xff;

    var body_chunk = new Buffer([
        // bitmap data, 2 bytes per row

        ____, ____,  ____, _x__,  ____, xxx_,  ___x, xx__,
        _xxx, ____,  xxx_, xx__,  _x__, xx__,  _xx_, xx__,
        __xx, _xx_,  ___x, xxx_,  ___x, xxx_,  ___x, xxx_,
        __xx, _x__,  __x_, __x_,  _xx_, _xx_,  ____, ____
    ]);

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
        if (typeof v === 'string') {
            // buf.write() doesn't accept a buffer argument
            off += buf.write(v, off);  // write() returns # bytes written
        } else if (Buffer.isBuffer(v)) {
            off += v.copy(buf, off);
        } else if (v instanceof Array) {
            v.forEach(function (elem) { write(elem); });
        } else if (v instanceof Object) {
            if (typeof v.type !== 'string' || v.type.length !== 4) {
                throw new Error('object missing type or type invalid');
            }
            off += buf.write(v.type, off);
            off_len = off;
            off += 4;
            off_start = off;
            write(v.data);
            data_len = off - off_start;
            while (off & 0x01) {
                // pad to even length
                off = buf.writeUInt8(0, off);  // writeUint8() returns off + #bytes written!
            }
            buf.writeUInt32BE(data_len, off_len);
        } else {
            throw new Error('failed to encode, invalid value');
        }
    }

    write(chunks);
    var res = new Buffer(off);
    buf.copy(res);

    if (typeof Duktape === 'object') {
        print(Duktape.enc('base64', Duktape.Buffer(res)));
    } else {
        print(res.toString('base64'));
    }
}

try {
    iffTest();
} catch (e) {
    print(e.stack || e);
}
