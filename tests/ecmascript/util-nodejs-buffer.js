/*
 *  Utilities for Node.js Buffer tests
 */

// Helper to print out Node.js Buffer prototype chains.
function getPrototypeChain(x) {
    var res = [];
    var protos = [
        { ref: Object.prototype, name: 'Object.prototype' },
        { ref: Array.prototype, name: 'Array.prototype' },
        { ref: Buffer.prototype, name: 'Buffer.prototype' },
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

function printableNodejsBuffer(buf) {
    var tmp = [];
    var i, n;

    for (i = 0, n = Math.min(buf.length, 32); i < n; i++) {
        tmp.push(('00' + buf[i].toString(16)).substr(-2));
    }
    if (buf.length > 32) { tmp.push('...'); }
    return tmp.join('');
}

function printNodejsBuffer(buf) {
    print(buf.length + ' bytes: ' + printableNodejsBuffer(buf));
}

// Escape a string with codepoint escaping.
function safeEscape(s) {
    var tmp = [];
    var i, c;

    for (i = 0; i < s.length; i++) {
        // When CESU-8 / extended UTF-8 decoding fails, Duktape currently throws
        // for charCodeAt().
        try {
            c = s.charCodeAt(i);
            if (c < 0x20 || c > 0x7e || c == '<' || c == '>' || c == '"' || c == '\'') {
                tmp.push('<U+' + (('0000') + c.toString(16)).substr(-4).toUpperCase() + '>');
            } else {
                tmp.push(String.fromCharCode(c));
            }
        } catch (e) {
            tmp.push('<' + e.name + '>');
        }
    }

    return '"' + tmp.join('') + '"'
}

function safePrint(s) {
    print(safeEscape(s));
}
