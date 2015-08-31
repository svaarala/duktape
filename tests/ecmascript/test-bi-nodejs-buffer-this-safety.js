/*
 *  Basic sanity test of invalid 'this' bindings given to Node.js Buffer
 *  related calls.  No expected output except that nothing breaks.
 */

/*---
{
    "custom": true
}
---*/

/*===
Node.js Buffer "this" sanity test
total: 5508
done
===*/

function nodejsBufferThisSanityTest() {
    var buf = new Buffer('ABCD');
    var thisValues = [
        undefined, null, true, false, 123.0, 'foo',
        { foo: 'bar' },
        [ 'foo', 'bar' ],
        Duktape.dec('hex', 'deadbeef'),
        new Duktape.Buffer(Duktape.dec('hex', 'deadfeed')),
        new Buffer('ABCDEFGH'),
        function func() {}
    ];
    var funcValues = [
        Buffer,

        Buffer.isEncoding,
        Buffer.isBuffer,
        Buffer.byteLength,
        Buffer.concat,
        Buffer.compare,

        buf.length,
        buf.write,
        buf.writeUIntLE,
        buf.writeUIntBE,
        buf.writeIntLE,
        buf.writeIntBE,
        buf.readUIntLE,
        buf.readUIntBE,
        buf.readIntLE,
        buf.readIntBE,
        buf.toString,
        buf.toJSON,
        buf.equals,
        buf.compare,
        buf.copy,
        buf.slice,
        buf.fill,

        buf.readUInt8,
        buf.readUInt16LE,
        buf.readUInt16BE,
        buf.readUInt32LE,
        buf.readUInt32BE,
        buf.readInt8,
        buf.readInt16LE,
        buf.readInt16BE,
        buf.readInt32LE,
        buf.readInt32BE,
        buf.readFloatLE,
        buf.readFloatBE,
        buf.readDoubleLE,
        buf.readDoubleBE,
        buf.writeUInt8,
        buf.writeUInt16LE,
        buf.writeUInt16BE,
        buf.writeUInt32LE,
        buf.writeUInt32BE,
        buf.writeInt8,
        buf.writeInt16LE,
        buf.writeInt16BE,
        buf.writeInt32LE,
        buf.writeInt32BE,
        buf.writeFloatLE,
        buf.writeFloatBE,
        buf.writeDoubleLE,
        buf.writeDoubleBE
    ];
    // Pretty dummy
    var argsValues = [
        [],
        [ undefined ],
        [ buf ],
        [ 123, ],
        [ 'quux', ],
        [ buf, undefined ],
        [ buf, buf ],
        [ buf, 123 ],
        [ buf, 'quux' ],
    ];

    var totalCount = 0;
    var errorCount = 0;

    thisValues.forEach(function (thisVal) {
        funcValues.forEach(function (funcVal) {
            argsValues.forEach(function (args) {
                totalCount++;
                try {
                    funcVal.apply(thisVal, args);
                } catch (e) {
                    //print(e);
                    errorCount++;
                }
            });
        });
    });

    print('total:', totalCount);
    //print('errors:', errorCount);  // varies when details changed
}

try {
    print('Node.js Buffer "this" sanity test');
    nodejsBufferThisSanityTest();
    print('done');
} catch (e) {
    print(e.stack || e);
}
