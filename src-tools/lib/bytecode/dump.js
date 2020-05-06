/*
 *  Bytecode dumper
 */

'use strict';

const { prettyInstructionLine } = require('./instruction');
const { drawBytecodeJumpLines } = require('./jump');
const { hexEncode } = require('../util/hex');
const { createBareObject } = require('../util/bare');
const { assert } = require('../util/assert');

function dumpString(doc, opts) {
    void opts;
    if (doc.type !== 'string') {
        throw new TypeError('internal error, doc.type: ' + doc.type);
    }
    // Ideally we'd pass through valid UTF-8 and just escape invalid UTF-8.
    // But for now, pass through ASCII and escape most of the rest.
    return '\'' + Array.prototype.map.call(doc.value, (v) => {
        if (v < 0x20 || v > 0x7e || v == 0x27 || v == 0x5c) {
            return '\\x' + hexEncode(v, 2);
        }
        return String.fromCharCode(v);
    }).join('') + '\'';
}

function dumpBuffer(doc, opts) {
    void opts;
    var res = [];
    var val = doc.value;
    for (let i = 0; i < val.length; i++) {
        res.push(val[i]);
    }
    return '[' + res.join(',') + ']';
}

function dumpInstruction(doc, idx, res, indent, jumpLines, opts) {
    let prefix = jumpLines ? jumpLines[idx] + ' ' : '';
    if (opts.dukOpcodes) {
        res.push([ indent, prefix + prettyInstructionLine(doc.ins, idx, opts.dukOpcodes) ]);
    } else {
        res.push([ indent, prefix + idx + ': ' + hexEncode(doc.ins, 8) ]);
    }
}

function prettyDoubleU8(u8) {
    assert(u8 instanceof Uint8Array);
    assert(u8.length === 8);
    var tmp = new Uint8Array(u8);  // copy with no offset
    var dv = new DataView(tmp.buffer);
    var dbl = dv.getFloat64(0);
    if (dbl === 0) {
        if (1 / dbl > 0) {
            return '0';
        } else {
            return '-0';
        }
    } else {
        return String(dbl);
    }
}

function dumpConstant(doc, idx, res, indent, opts) {
    let tmp;
    switch (doc.type) {
    case 'string':
        tmp = dumpString(doc, opts);
        break;
    case 'double':
        tmp = prettyDoubleU8(doc.value) + ' <= ' + dumpBuffer(doc, opts);
        break;
    default:
        throw new TypeError('internal error');
    }
    res.push([ indent, idx + ': ' + tmp ]);
}

function dumpVarmapEntry(doc, idx, res, indent, opts) {
    res.push([ indent, dumpString(doc.name, opts) + ' => ' + doc.reg ]);
}

function dumpFormalsEntry(doc, idx, res, indent, opts) {
    res.push([ indent, dumpString(doc.name, opts) + ' => ' + doc.position ]);
}

function dumpFunction(doc, res, indent, opts) {
    res.push([ indent, 'Instructions: ' + doc.countInstr ]);
    res.push([ indent, 'Constants: ' + doc.countConst ]);
    res.push([ indent, 'Inner functions: ' + doc.countFunc ]);
    res.push([ indent, 'Nregs: ' + doc.nregs ]);
    res.push([ indent, 'Nargs: ' + doc.nargs ]);
    res.push([ indent, 'Start line number: ' + doc.startLine ]);
    res.push([ indent, 'End line number: ' + doc.endLine ]);
    res.push([ indent, 'duk_hcompfunc flags: 0x' + hexEncode(doc.compfuncFlags, 4) ]);
    res.push([ indent, '.length: ' + doc.length ]);
    res.push([ indent, '.name: ' + dumpString(doc.name, opts) ]);
    res.push([ indent, '.fileName: ' + dumpString(doc.fileName, opts) ]);
    res.push([ indent, 'Opcodes:' ]);
    var jumpLines;
    if (opts.jumps) {
        jumpLines = drawBytecodeJumpLines(doc, opts);
    }
    doc.instructions.forEach((instr, idx) => {
        dumpInstruction(instr, idx, res, indent + 1, jumpLines, opts);
    });
    res.push([ indent, 'Constants:' ]);
    doc.constants.forEach((elem, idx) => {
        dumpConstant(elem, idx, res, indent + 1, opts);
    });
    res.push([ indent, '_Varmap:' ]);
    doc.varmap.forEach((elem, idx) => {
        dumpVarmapEntry(elem, idx, res, indent + 1, opts);
    });
    if (doc.formals) {
        res.push([ indent, '_Formals:' ]);
        doc.formals.forEach((elem, idx) => {
            dumpFormalsEntry(elem, idx, res, indent + 1, opts);
        });
    } else {
        res.push([ indent, '_Formals: missing' ]);
    }
    res.push([ indent, '_Pc2line: ' + dumpBuffer(doc.pc2line, opts) ]);
    res.push([ indent, 'Inner functions:' ]);
    doc.functions.forEach((func) => {
        dumpFunction(func, res, indent + 1, opts);
    });
}

function dumpBytecodeRaw(doc, res, indent, opts) {
    res.push([ indent, '[Bytecode signature 0x' + hexEncode(doc.sig, 2) + ']' ]);
    dumpFunction(doc.func, res, indent + 1, opts);
}

function dumpBytecode(doc, opts) {
    const indentStep = '  ';
    opts = opts || createBareObject({});
    var res = [];
    dumpBytecodeRaw(doc, res, 0, opts);
    var indented = res.map((ent) => indentStep.repeat(ent[0]) + ent[1]);
    return indented.join('\n');
}
exports.dumpBytecode = dumpBytecode;
