/*
 *  Visualize bytecode jumps
 */

'use strict';

const { decodeInstruction } = require('./instruction');

function prepareBytecodeJumps(doc, opts) {
    var jumps = [];
    if (!opts.dukOpcodes) {
        return;
    }

    doc.instructions.forEach((instr, idx) => {
        var dec = decodeInstruction(instr.ins, idx, opts.dukOpcodes);
        if (typeof dec.jumpTarget === 'number') {
            let pcSrc = idx;
            let pcDst = dec.jumpTarget;
            jumps.push({ source: pcSrc, target: pcDst });
        }
    });

    // First sort jumps by their lowest PC (source or target).
    jumps.sort((a, b) => {
        let an = Math.min(a.source, a.target);
        let bn = Math.min(b.source, b.target);
        if (an > bn) { return 1; }
        if (an < bn) { return -1; }
        return 0;
    });

    function jumpsOverlap(a, b) {
        let aMin = Math.min(a.source, a.target);
        let aMax = Math.max(a.source, a.target);
        let bMin = Math.min(b.source, b.target);
        let bMax = Math.max(b.source, b.target);
        if (aMax < bMin) {
            return false;
        }
        if (aMin > bMax) {
            return false;
        }
        return true;

    }
    function overlaps(level, jump) {
        for (let i = 0; i < level.length; i++) {
            if (jumpsOverlap(level[i], jump)) {
                return true;
            }
        }
        return false;
    }

    var levels = [];
    while (jumps.length > 0) {
        // Sort remaining jumps primarily by their length, and
        // secondarily by their lowest PC.
        jumps.sort((a, b) => {
            var an = Math.abs(a.source - a.target);
            var bn = Math.abs(b.source - b.target);
            if (an > bn) { return 1; }
            if (an < bn) { return -1; }
            an = Math.min(a.source, a.target);
            bn = Math.min(b.source, b.target);
            if (an > bn) { return 1; }
            if (an < bn) { return -1; }
            return 0;
        });

        // Create a new level by picking up the shortest remaining
        // jumps that don't overlap.
        let level = [];
        for (let i = 0; i < jumps.length;) {
            let jump = jumps[i];
            if (!overlaps(level, jump)) {
                void jumps.splice(i, 1);
                level.push(jump);
            } else {
                i++;
            }
        }
        levels.push(level);
    }
    //console.log(levels);
    return levels;
}

function drawBytecodeJumps(levels, numLines) {
    var lines = [];
    for (let i = 0; i < numLines; i++) {
        let columns = [];
        while (columns.length < 3 * levels.length) {
            columns.push(' ');
        }
        lines.push(columns);
    }

    levels.forEach((level, levelIdx) => {
        let levOff = levels.length * 3 - levelIdx * 3 - 3;
        level.forEach((jump) => {
            if (jump.target === jump.source) {
                let pc = jump.source;
                lines[pc][levOff + 0] = ' ';
                lines[pc][levOff + 1] = '<';
                lines[pc][levOff + 2] = '>';
            } else {
                if (jump.target > jump.source) {
                    lines[jump.source][levOff + 0] = ' ';
                    lines[jump.source][levOff + 1] = '.';
                    lines[jump.source][levOff + 2] = '-';
                    for (let pc = jump.source + 1; pc <= jump.target - 1; pc++) {
                        lines[pc][levOff + 0] = ' ';
                        lines[pc][levOff + 1] = ':';  // forward jump
                        lines[pc][levOff + 2] = ' ';
                    }
                    lines[jump.target][levOff + 0] = ' ';
                    lines[jump.target][levOff + 1] = '`';
                    lines[jump.target][levOff + 2] = '>';
                } else {
                    lines[jump.source][levOff + 0] = ' ';
                    lines[jump.source][levOff + 1] = '`';
                    lines[jump.source][levOff + 2] = '-';
                    for (let pc = jump.source - 1; pc >= jump.target + 1; pc--) {
                        lines[pc][levOff + 0] = ' ';
                        lines[pc][levOff + 1] = '|';  // backward jump
                        lines[pc][levOff + 2] = ' ';
                    }
                    lines[jump.target][levOff + 0] = ' ';
                    lines[jump.target][levOff + 1] = '.';
                    lines[jump.target][levOff + 2] = '>';
                }
            }
        });
    });

    // Get final jump lines, strip off leading space.
    var jumpLines = lines.map((line) => { return line.join('').substring(1); });
    return jumpLines;
}

function drawBytecodeJumpLines(doc, opts) {
    var jumpLevels = prepareBytecodeJumps(doc, opts);
    var jumpLines = drawBytecodeJumps(jumpLevels, doc.instructions.length);
    return jumpLines;
}
exports.drawBytecodeJumpLines = drawBytecodeJumpLines;
