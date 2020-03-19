/*
 *  Pretty print metadata objects and strings in a human readable from.
 */

'use strict';

const { walkObjectsAndProperties, walkStrings } = require('./util');
const { hexDecode } = require('../../util/hex');

function prettyString(x) {
    return x.replace(/[\u0000-\u001f\u007f-\uffff<>"]/g, (c) => '<' + c.charCodeAt(0).toString(16) + '>');
}

function prettyAttributes(x) {
    return '[' +
        (x.indexOf('w') >= 0 ? 'w' : '-') +
        (x.indexOf('e') >= 0 ? 'e' : '-') +
        (x.indexOf('c') >= 0 ? 'c' : '-') +
        (x.indexOf('a') >= 0 ? 'a' : '-') +
        ']';
}

function prettyValue(x) {
    if (typeof x === 'string') {
        return '"' + prettyString(x) + '"';
    }
    if (typeof x === 'object' && x !== null) {
        switch (x.type) {
        case 'object': {
            return '-> ' + x.id;
        }
        case 'accessor': {
            return 'get -> ' + (typeof x.getter_id === 'string' ? x.getter_id : 'null') +
                   ', set -> ' + (typeof x.setter_id === 'string' ? x.setter_id : 'null');
        }
        case 'double': {
            let u8 = hexDecode(x.bytes);
            let dv = new DataView(u8.buffer);
            return '<' + x.bytes + '> ' + dv.getFloat64(0);
        }
        }
    }

    return JSON.stringify(x);
}

function prettyPropertyEntry(p) {
    let tmp = '    - ' + prettyAttributes(p.attributes) + ' ' + prettyString(p.key);
    while (tmp.length < 40) {
        tmp += ' ';
    }
    return tmp + ' ' + prettyValue(p.value);
}

function prettyStringEntry(s) {
    var extra = [];
    if (s.stridx !== void 0) {
        extra.push('stridx ' + s.stridx);
    }
    if (s.stridx_used) {
        extra.push('stridx_used');
    }
    return '    - "' + prettyString(s.str) + '"' + (extra.length > 0 ? ' ' : '') + extra.map((e) => '[' + e + ']').join(' ');
}

function prettyObjectEntry(o) {
    var extra = [];
    extra.push('class ' + o.class);
    extra.push('iproto ' + o.internal_prototype);
    if (o.callable) {
        extra.push('callable');
    }
    if (o.constructable) {
        extra.push('constructable');
    }
    if (o.native) {
        extra.push('native ' + o.native);
    }
    if (o.varargs !== void 0) {
        extra.push('varargs');
    }
    if (o.nargs !== void 0) {
        extra.push('nargs ' + o.nargs);
    }
    if (o.bidx !== void 0) {
        extra.push('bidx ' + o.bidx);
    }
    if (o.bidx_used) {
        extra.push('bidx_used');
    }

    return '[OBJECT ' + o.id + ']' + (extra.length > 0 ? ' ' : '') + extra.map((e) => '[' + e + ']').join(' ');
}

function prettyPrintMetadata(meta) {
    var res = [];
    walkObjectsAndProperties(meta, (o) => {
        res.push(prettyObjectEntry(o));
    }, (p, o) => {
        void o;
        res.push(prettyPropertyEntry(p));
    });
    res.push('[STRINGS]');
    walkStrings(meta, (s) => {
        res.push(prettyStringEntry(s));
    });
    return res.join('\n');
}
exports.prettyPrintMetadata = prettyPrintMetadata;
