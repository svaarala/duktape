/*
 *  Convert bytecode instruction to a readable format
 *  based on opcode metadata.
 */

'use strict';

const { hexEncode } = require('../util/hex');
const { assert } = require('../util/assert');

function decodeInstruction(ins, pc, dukOpcodes) {
    assert(typeof ins === 'number');
    assert(typeof dukOpcodes === 'object' && dukOpcodes !== null);
    var opcode = ins & 0xff;
    var A = (ins >>> 8) & 0xff;
    var B = (ins >>> 16) & 0xff;
    var C = (ins >>> 24) & 0xff;
    var BC = (ins >>> 16) & 0xffff;
    var ABC = (ins >>> 8) & 0xffffff;
    var Bconst = op & 0x01;
    var Cconst = op & 0x02;
    var op = dukOpcodes.opcodes[opcode];
    var name;
    var args = [];
    var rawArgs = [];
    var comments = [];
    var res = {};

    if (!op) {
        name = 'UNKNOWN';
        args.push('?');
        comments.push('unknown opcode ' + opcode);
        return { ins, name, args, comments };
    }

    name = op.name;
    if (op.args) {
        for (let i = 0; i < op.args.length; i++) {
            switch (op.args[i]) {
            case 'A_R':   rawArgs.push(A); args.push('r' + A); break;
            case 'A_RI':  rawArgs.push(A); args.push('r' + A + '(indirect)'); break;
            case 'A_C':   rawArgs.push(A); args.push('c' + A); break;
            case 'A_H':   rawArgs.push(A); args.push('0x' + A.toString(16)); break;
            case 'A_I':   rawArgs.push(A); args.push(A.toString(10)); break;
            case 'A_B':   rawArgs.push(A); args.push(A ? 'true' : 'false'); break;
            case 'B_RC':  rawArgs.push(B); args.push((Bconst ? 'c' : 'r') + B); break;
            case 'B_R':   rawArgs.push(B); args.push('r' + B); break;
            case 'B_RI':  rawArgs.push(B); args.push('r' + B + '(indirect)'); break;
            case 'B_C':   rawArgs.push(B); args.push('c' + B); break;
            case 'B_H':   rawArgs.push(B); args.push('0x' + B.toString(16)); break;
            case 'B_I':   rawArgs.push(B); args.push(B.toString(10)); break;
            case 'C_RC':  rawArgs.push(C); args.push((Cconst ? 'c' : 'r') + C); break;
            case 'C_R':   rawArgs.push(C); args.push('r' + C); break;
            case 'C_RI':  rawArgs.push(C); args.push('r' + C + '(indirect)'); break;
            case 'C_C':   rawArgs.push(C); args.push('c' + C); break;
            case 'C_H':   rawArgs.push(C); args.push('0x' + C.toString(16)); break;
            case 'C_I':   rawArgs.push(C); args.push(C.toString(10)); break;
            case 'BC_R':  rawArgs.push(BC); args.push('r' + BC); break;
            case 'BC_C':  rawArgs.push(BC); args.push('c' + BC); break;
            case 'BC_H':  rawArgs.push(BC); args.push('0x' + BC.toString(16)); break;
            case 'BC_I':  rawArgs.push(BC); args.push(BC.toString(10)); break;
            case 'ABC_H': rawArgs.push(ABC); args.push(ABC.toString(16)); break;
            case 'ABC_I': rawArgs.push(ABC); args.push(ABC.toString(10)); break;
            case 'BC_LDINT': rawArgs.push(BC); args.push(BC - (1 << 15)); break;
            case 'BC_LDINTX': rawArgs.push(BC); args.push(BC - 0); break;  // no bias in LDINTX
            case 'ABC_JUMP': {
                let pc_add = ABC - (1 << 23) + 1;  // pc is preincremented before adding
                let pc_dst = pc + pc_add;
                rawArgs.push(ABC);
                res.jumpTarget = pc_dst;
                args.push(pc_dst + ' (' + (pc_add >= 0 ? '+' : '') + pc_add + ')');
                break;
            }
            default:      args.push('?'); break;
            }
        }
    }
    if (op.flags) {
        for (let i = 0; i < op.flags.length; i++) {
            if (ins & op.flags[i].mask) {
                comments.push(op.flags[i].name);
            } else {
                comments.push('!' + op.flags[i].name);
            }
        }
    }
    //comments.push('raw args ' + JSON.stringify(rawArgs));
    Object.assign(res, { ins, name, args, rawArgs, comments });
    return res;
}
exports.decodeInstruction = decodeInstruction;

function formatInstruction(doc, pc) {
    var parts = [];
    parts.push(('00000' + pc).substr(-5));
    parts.push(hexEncode(doc.ins, 8));
    parts.push(' ');
    parts.push((doc.name + ' '.repeat(12)).substr(0, 12));
    if (doc.args.length > 0) {
        parts.push(doc.args.join(', '));
    }
    var tmp = parts.join(' ');
    if (doc.comments.length > 0) {
        tmp += ' '.repeat(44 - tmp.length);
        tmp += ' ; ' + doc.comments.join(', ');
    }
    return tmp;
}

function prettyInstructionLine(ins, pc, dukOpcodes) {
    var doc = decodeInstruction(ins, pc, dukOpcodes);
    return formatInstruction(doc, pc);
}
exports.prettyInstructionLine = prettyInstructionLine;
