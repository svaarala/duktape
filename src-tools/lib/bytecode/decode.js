/*
 *  Bytecode decoder
 */

'use strict';

const { hexEncode } = require('../util/hex');

'use strict';

function decodeStringConst(dv, off) {
    var strlen = dv.getUint32(off);
    off += 4;
    var strdata = new Uint8Array(strlen);
    for (let i = 0; i < strlen; i++) {
        strdata[i] = dv.getUint8(off + i);
    }
    off += strlen;
    return [off, { type: 'string', value: strdata }];
}

function decodeBufferConst(dv, off) {
    var buflen = dv.getUint32(off);
    off += 4;
    var bufdata = new Uint8Array(buflen);
    for (let i = 0; i < buflen; i++) {
        bufdata[i] = dv.getUint8(off + i);
    }
    off += buflen;
    return [off, { type: 'buffer', value: bufdata }];
}

function decodeDoubleConst(dv, off) {
    var dbldata = new Uint8Array(8);
    for (let i = 0; i < 8; i++) {
        dbldata[i] = dv.getUint8(off + i);
    }
    off += 8;
    return [off, { type: 'double', value: dbldata }];
}

function decodeFunction(dv, off, opts) {
    var res;
    var countInstr, countConst, countFunc;
    var nregs, nargs, startLine, endLine;
    var compfuncFlags;
    var numFormals;
    var instructions = [];
    var constants = [];
    var functions = [];
    var varmap = [];
    var formals = [];
    var length, name, fileName, pc2line;

    countInstr = dv.getUint32(off);
    countConst = dv.getUint32(off + 4);
    countFunc = dv.getUint32(off + 8);
    off += 12;

    nregs = dv.getUint16(off);
    nargs = dv.getUint16(off + 2);
    startLine = dv.getUint32(off + 4);  // if no debugger support, line numbers are present but 0
    endLine = dv.getUint32(off + 8);
    off += 12;

    compfuncFlags = dv.getUint32(off);
    off += 4;

    for (let i = 0; i < countInstr; i++) {
        let ins = dv.getUint32(off);
        off += 4;
        instructions.push({ ins });
    }

    for (let i = 0; i < countConst; i++) {
        let constType = dv.getUint8(off);
        off++;
        let constValue;

        switch (constType) {
        case 0:  // string
            [off, constValue] = decodeStringConst(dv, off);
            break;
        case 1:  // double
            [off, constValue] = decodeDoubleConst(dv, off);
            break;
        default:
            throw new TypeError('invalid constant type: ' + constType);
        }
        constants.push(constValue);
    }

    for (let i = 0; i < countFunc; i++) {
        let func;
        [off, func] = decodeFunction(dv, off, opts);
        functions.push(func);
    }

    length = dv.getUint32(off);
    off += 4;
    [off, name] = decodeStringConst(dv, off);
    [off, fileName] = decodeStringConst(dv, off);
    [off, pc2line] = decodeBufferConst(dv, off);

    for (;;) {
        let varName, varReg;
        [off, varName] = decodeStringConst(dv, off);
        if (varName.value.length === 0) {
            break;
        }
        varReg = dv.getUint32(off);
        off += 4;
        varmap.push({ name: varName, reg: varReg });
    }

    numFormals = dv.getUint32(off);
    off += 4;
    if (numFormals != 0xffffffff) {
        for (let i = 0; i < numFormals; i++) {
            let argName;
            [off, argName] = decodeStringConst(dv, off);
            formals.push({ name: argName, position: i });
        }
    }

    res = {
        countInstr,
        countConst,
        countFunc,
        nregs,
        nargs,
        startLine,
        endLine,
        compfuncFlags,
        numFormals,
        instructions,
        constants,
        functions,
        varmap,
        formals,
        length,
        name,
        fileName,
        pc2line
    };

    return [off, res];
}

function decodeBytecode(dv, off, opts) {
    var res = {};
    var sig;

    sig = dv.getUint8(dv, off);
    off++;
    if (sig === 0xff) {
        throw new TypeError('pre-Duktape 2.2 0xFF signature byte (signature byte is 0xBF since Duktape 2.2)');
    }
    if (sig !== 0xbf) {
        throw new TypeError('invalid signature byte: 0x' + hexEncode(sig, 2));
    }
    res.sig = sig;

    [off, res.func] = decodeFunction(dv, off, opts);

    return [off, res];
}

function decodeBytecodeUint8Array(u8, opts) {
    var off = 0;
    var dv = new DataView(u8.buffer, u8.byteOffset, u8.byteLength);
    var res;

    // Opts reserved for e.g. indicating bytecode version or config
    // options affecting bytecode format.
    void opts;

    [off, res] = decodeBytecode(dv, off, opts);
    if (off < dv.byteLength) {
        throw new TypeError('trailing garbage');
    } else if (off > dv.byteLength) {
        throw new TypeError('internal error');
    }
    return res;
}
exports.decodeBytecodeUint8Array = decodeBytecodeUint8Array;
