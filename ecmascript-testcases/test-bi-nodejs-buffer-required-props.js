/*
 *  Required properties
 */

/*@include util-nodejs-buffer.js@*/

/*===
properties test
Buffer
value exists: true
isEncoding: true
isBuffer: true
byteLength: true
concat: true
compare: true
Buffer.prototype
value exists: true
write: true
writeUIntLE: true
writeUIntBE: true
writeIntLE: true
writeIntBE: true
readUIntLE: true
readUIntBE: true
readIntLE: true
readIntBE: true
toString: true
toJSON: true
equals: true
compare: true
copy: true
slice: true
readUInt8: true
readUInt16LE: true
readUInt16BE: true
readUInt32LE: true
readUInt32BE: true
readInt8: true
readInt16LE: true
readInt16BE: true
readInt32LE: true
readInt32BE: true
readFloatLE: true
readFloatBE: true
readDoubleLE: true
readDoubleBE: true
writeUInt8: true
writeUInt16LE: true
writeUInt16BE: true
writeUInt32LE: true
writeUInt32BE: true
writeInt8: true
writeInt16LE: true
writeInt16BE: true
writeInt32LE: true
writeInt32BE: true
writeFloatLE: true
writeFloatBE: true
writeDoubleLE: true
writeDoubleBE: true
fill: true
Buffer instance
value exists: true
write: false
writeUIntLE: false
writeUIntBE: false
writeIntLE: false
writeIntBE: false
readUIntLE: false
readUIntBE: false
readIntLE: false
readIntBE: false
toString: false
toJSON: false
equals: false
compare: false
copy: false
slice: false
readUInt8: false
readUInt16LE: false
readUInt16BE: false
readUInt32LE: false
readUInt32BE: false
readInt8: false
readInt16LE: false
readInt16BE: false
readInt32LE: false
readInt32BE: false
readFloatLE: false
readFloatBE: false
readDoubleLE: false
readDoubleBE: false
writeUInt8: false
writeUInt16LE: false
writeUInt16BE: false
writeUInt32LE: false
writeUInt32BE: false
writeInt8: false
writeInt16LE: false
writeInt16BE: false
writeInt32LE: false
writeInt32BE: false
writeFloatLE: false
writeFloatBE: false
writeDoubleLE: false
writeDoubleBE: false
fill: false
value exists: true
length: true
0: true
1: true
2: true
3: true
4: true
5: true
6: true
7: true
8: true
9: true
===*/

function propertiesTest() {
    var b;

    function check(val, names) {
        print('value exists: ' + (val !== undefined));
        names.forEach(function (k) {
            print(k + ': ' + Object.prototype.hasOwnProperty.call(val, k));
        });
    }

    var Buffer_props = [
        'isEncoding', 'isBuffer', 'byteLength', 'concat', 'compare'
    ];
    var Buffer_prototype_props = [
        'write',
        'writeUIntLE', 'writeUIntBE', 'writeIntLE', 'writeIntBE',
        'readUIntLE', 'readUIntBE', 'readIntLE', 'readIntBE',
        'toString', 'toJSON',
        'equals', 'compare', 'copy', 'slice',
        'readUInt8', 'readUInt16LE', 'readUInt16BE', 'readUInt32LE', 'readUInt32BE',
        'readInt8', 'readInt16LE', 'readInt16BE', 'readInt32LE', 'readInt32BE',
        'readFloatLE', 'readFloatBE', 'readDoubleLE', 'readDoubleBE',
        'writeUInt8', 'writeUInt16LE', 'writeUInt16BE', 'writeUInt32LE', 'writeUInt32BE',
        'writeInt8', 'writeInt16LE', 'writeInt16BE', 'writeInt32LE', 'writeInt32BE',
        'writeFloatLE', 'writeFloatBE', 'writeDoubleLE', 'writeDoubleBE',
        'fill'
    ];
    var Buffer_instance_props = [
        // 'parent' is not tested for, not currently implemented
        'length',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
    ];

    print('Buffer');
    check(Buffer, Buffer_props);

    print('Buffer.prototype');
    check(Buffer.prototype, Buffer_prototype_props);

    print('Buffer instance');
    b = new Buffer(10);
    check(b, Buffer_prototype_props);  // these are NOT present
    check(b, Buffer_instance_props);
}

try {
    print('properties test');
    propertiesTest();
} catch (e) {
    print(e.stack || e);
}
