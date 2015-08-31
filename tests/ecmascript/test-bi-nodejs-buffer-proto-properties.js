/*
 *  Node.js Buffer.prototype properties
 */

function encValue(x) {
    if (typeof x === 'function') { return 'function'; }
    return String(x);
}

/*===
Node.js Buffer prototype properties test
constructor true function function
write true function function
toString true function function
toJSON true function function
equals true function function
compare true function function
copy true function function
slice true function function
fill true function function
writeUIntLE true function function
writeUIntBE true function function
writeIntLE true function function
writeIntBE true function function
readUIntLE true function function
readUIntBE true function function
readIntLE true function function
readIntBE true function function
writeUInt8 true function function
writeUInt16LE true function function
writeUInt16BE true function function
writeUInt32LE true function function
writeUInt32BE true function function
writeInt8 true function function
writeInt16LE true function function
writeInt16BE true function function
writeInt32LE true function function
writeInt32BE true function function
writeFloatLE true function function
writeFloatBE true function function
writeDoubleLE true function function
writeDoubleBE true function function
readUInt8 true function function
readUInt16LE true function function
readUInt16BE true function function
readUInt32LE true function function
readUInt32BE true function function
readInt8 true function function
readInt16LE true function function
readInt16BE true function function
readInt32LE true function function
readInt32BE true function function
readFloatLE true function function
readFloatBE true function function
readDoubleLE true function function
readDoubleBE true function function
true
===*/

function nodejsBufferPrototypePropertiesTest() {
    var props = [
        'constructor',

        'write',
        'toString',
        'toJSON',
        'equals',
        'compare',
        'copy',
        'slice',
        'fill',

        'writeUIntLE',
        'writeUIntBE',
        'writeIntLE',
        'writeIntBE',

        'readUIntLE',
        'readUIntBE',
        'readIntLE',
        'readIntBE',

        'writeUInt8',
        'writeUInt16LE',
        'writeUInt16BE',
        'writeUInt32LE',
        'writeUInt32BE',
        'writeInt8',
        'writeInt16LE',
        'writeInt16BE',
        'writeInt32LE',
        'writeInt32BE',
        'writeFloatLE',
        'writeFloatBE',
        'writeDoubleLE',
        'writeDoubleBE',

        'readUInt8',
        'readUInt16LE',
        'readUInt16BE',
        'readUInt32LE',
        'readUInt32BE',
        'readInt8',
        'readInt16LE',
        'readInt16BE',
        'readInt32LE',
        'readInt32BE',
        'readFloatLE',
        'readFloatBE',
        'readDoubleLE',
        'readDoubleBE'
    ];

    props.forEach(function (propname) {
        var obj = Buffer.prototype;
        var val = obj[propname];
        print(propname, propname in obj, typeof val, encValue(val));
    });

    print(Buffer.prototype.constructor === Buffer);
}

try {
    print('Node.js Buffer prototype properties test');
    nodejsBufferPrototypePropertiesTest();
} catch (e) {
    print(e.stack || e);
}
