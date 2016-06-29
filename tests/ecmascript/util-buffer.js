/*
 *  Utilities for buffer and TypedArray tests
 */

var integerEndianness;
var doubleEndianness;
var isBigEndian;
var isLittleEndian;
var isMixedDouble;

// Detect plain buffer, in Duktape 2.x plain buffers mimic ArrayBuffer so check
// indirectly.
function isPlainBuffer(x) {
    var tag;
    tag = Duktape.info(x)[0];  // api tag, 7=plain buffer, 6=object
    return tag === 7;
}

// Helper to print out TypedArray prototype chains.
function getPrototypeChain(x) {
    var res = [];
    var protos = [
        { ref: Object.prototype, name: 'Object.prototype' },
        { ref: Array.prototype, name: 'Array.prototype' },

        { ref: Duktape.Buffer.prototype, name: 'Duktape.Buffer.prototype' },

        { ref: ArrayBuffer.prototype, name: 'ArrayBuffer.prototype' },
        { ref: DataView.prototype, name: 'DataView.prototype' },
        { ref: Int8Array.prototype, name: 'Int8Array.prototype' },
        { ref: Uint8Array.prototype, name: 'Uint8Array.prototype' },
        { ref: Uint8ClampedArray.prototype, name: 'Uint8ClampedArray.prototype' },
        { ref: Int16Array.prototype, name: 'Int16Array.prototype' },
        { ref: Uint16Array.prototype, name: 'Uint16Array.prototype' },
        { ref: Int32Array.prototype, name: 'Int32Array.prototype' },
        { ref: Uint32Array.prototype, name: 'Uint32Array.prototype' },
        { ref: Float32Array.prototype, name: 'Float32Array.prototype' },
        { ref: Float64Array.prototype, name: 'Float64Array.prototype' },

        // Duktape specific prototype which provides e.g. .subarray() for all views
        { ref: Object.getPrototypeOf(Uint8Array.prototype), name: 'TypedArray.prototype' },
        true  // end marker
    ];
    var i;

    for (;;) {
        x = Object.getPrototypeOf(x);
        if (!x) { break; }
        for (i = 0; i < protos.length; i++) {
            if (protos[i] === true) { res.push('???'); break; }
            if (!protos[i].ref) { continue; }
            if (protos[i].ref === x) { res.push(protos[i].name); break; }
        }
    }

    return res;
}

// Useful quick summary of a prototype chain, also stringifies
// source object using Object.prototype.toString().
function printPrototypeChain(x) {
    print(Object.prototype.toString.call(x) + ' -> ' + getPrototypeChain(x).join(' -> '));
}

function dumpOwnNonIndexProperties(x, dumpValue) {
    var names = Object.getOwnPropertyNames(x);
    var rem = [];
    names.forEach(function (v) {
        var n = Number(v);
        if (n >= 0 && Math.floor(n) === n) {
            return;  // accept number indices silently here
        }
        rem.push(v);  // non-index keys
    });
    rem.sort();
    rem.forEach(function (k) {
        if (dumpValue) {
            print(k, (typeof x[k] === 'object' || typeof x[k] === 'function') ? Object.prototype.toString.call(x[k]) : x[k]);
        } else {
            print(k);
        }
    });
}

// Get a list of test objects shared by multiple tests.  Includes all
// object types from typedarray spec.
function getTestObjectList() {
    var buf = new ArrayBuffer(16);
    var values = [
        buf,
        new DataView(buf),
        new Int8Array(buf),
        new Uint8Array(buf),
        new Uint8ClampedArray(buf),
        new Int16Array(buf),
        new Uint16Array(buf),
        new Int32Array(buf),
        new Uint32Array(buf),
        new Float32Array(buf),
        new Float64Array(buf)
    ];
    return values;
}

// Number to string, preserve negative zero sign.
function num2str(v) {
    if (v !== 0) { return String(v); }
    return (1 / v > 0) ? '0' : '-0';
}

// Buffer/view to hex
function buf2hex(b) {
    var res = [];
    var i;
    if (b.buffer) {
        b = b.buffer;
    }
    b = new Uint8Array(b);  // use uint8 view to get bytes without relying on ArrayBuffer index props
    for (i = 0; i < b.length; i++) {
        res.push(('00' + b[i].toString(16)).substr(-2));
    }
    return res.join('');
}

function printableBufferRaw(b) {
    var res = [];
    var slice_start, slice_end;
    var i, n;

    if (isPlainBuffer(b)) {
    } else if (Object.prototype.toString.call(b) === '[object ArrayBuffer]') {
        slice_start = 0;
        slice_end = b.byteLength;
    } else if (b.buffer) {
        slice_start = b.byteOffset;
        slice_end = b.byteOffset + b.byteLength;
        b = b.buffer;
    } else {
        return 'UNKNOWN';
    }
    b = new Uint8Array(b);  // use uint8 view to get bytes without relying on ArrayBuffer index props
    for (i = slice_start, n = Math.min(slice_end, slice_start + 32); i < n; i++) {
        res.push(('00' + b[i].toString(16)).substr(-2));
    }
    if (b.length > 32) { res.push('...'); }
    return {
        sliceBytes: (slice_end - slice_start),
        data: res.join('')
    }
}

function printableBuffer(b) {
    return printableBufferRaw(b).data;
}

// Pretty print a Buffer/view
function printBuffer(b) {
    var tmp = printableBufferRaw(b);
    print(tmp.sliceBytes + ' bytes: ' + tmp.data);
}

// Convert any buffer type into a plain string, using the buffer bytes
// directly as the internal string representation (same as Duktape 1.x
// ToString(plainBuffer) coercion).  This is no longer directly possible
// using String(plainBuffer) in Duktape 2.x; this helper hides the mechanism
// needed, as it may change later.

function bufferToString(buf) {
    // Node.js Buffer .toString() does the raw conversion and accepts buffer
    // objects of any type in Duktape (but not plain buffers so coerce them
    // to full ArrayBuffer objects).
    return Buffer.prototype.toString.call(Object(buf));
}

// Convert any string into a buffer, interpreting the internal string
// representation as the buffer bytes without interpretation.

function stringToBuffer(str) {
    // Node.js Buffer constructor currently uses string bytes 1:1.
    // Return an ArrayBuffer for now.
    return new Uint8Array(new Buffer(str)).buffer;
}

function isPlainBuffer(x) {
    return Duktape.info(x)[0] === 7;   // tag 7: plain buffer
}

// Detect endianness and setup globals.
function detectEndianness() {
    var b = new ArrayBuffer(8);
    var v1 = new Uint8Array(b);
    var v2 = new Uint32Array(b);
    var v3 = new Float64Array(b);

    /*
     *  Uint32 endianness
     */

    //print('detect uint32 endianness');

    v1[0] = 0x11;
    v1[1] = 0x22;
    v1[2] = 0x33;
    v1[3] = 0x44;

    //print(v2[0].toString(16));

    if (v2[0] == 0x11223344) {
        integerEndianness = 'big';
    } else if (v2[0] == 0x44332211) {
        integerEndianness = 'little';
    } else {
        throw new Error('cannot determine uint32 endianness');
    }

    /*
     *  Float64 endianness
     *
     *  Some ARM platforms have IEEE doubles in 'mixed endian' order.
     *
     *  Bytes in memory (big endian / network order):
     *
     *      00 11 22 33 44 55 66 77     big endian
     *      77 66 55 44 33 22 11 00     little endian
     *      33 22 11 00 77 66 55 44     mixed endian
     *
     *      >>> struct.unpack('>d', '0011223344556677'.decode('hex'))
     *      (2.3827196142341166e-308,)
     *      >>> struct.unpack('>d', '7766554433221100'.decode('hex'))
     *      (1.4402392267837606e+267,)
     *      >>> struct.unpack('>d', '3322110077665544'.decode('hex'))
     *      (2.195850906203525e-62,)
     *
     *  Avoid NaN values because they may not be present with packed
     *  value representation.
     */

    //print('detect float64 endianness');

    v1[0] = 0x00;
    v1[1] = 0x11;
    v1[2] = 0x22;
    v1[3] = 0x33;
    v1[4] = 0x44;
    v1[5] = 0x55;
    v1[6] = 0x66;
    v1[7] = 0x77;

    //print(v3[0]);

    if (v3[0] > 1e-308 && v3[0] < 1e-307) {
        doubleEndianness = 'big';
    } else if (v3[0] > 1e-62 && v3[0] < 1e-61) {
        doubleEndianness = 'mixed';
    } else if (v3[0] > 1e267 && v3[0] < 1e268) {
        doubleEndianness = 'little';
    } else {
        throw new Error('cannot determine float64 endianness');
    }

    // Sanity check
    if (integerEndianness == doubleEndianness || doubleEndianness == 'mixed') {
    } else {
        throw new Error('cannot determine endianness (sanity check failed)');
    }

    // Convenience values
    isBigEndian = (integerEndianness == 'big');
    isLittleEndian = (integerEndianness == 'little');
    isMixedDouble = (doubleEndianness == 'mixed');

    //print(integerEndianness);
    //print(doubleEndianness);
    //print(isBigEndian);
    //print(isLittleEndian);
    //print(isMixedDouble);
}

try {
    detectEndianness();
} catch (e) {
    print(e.stack || e);
}
