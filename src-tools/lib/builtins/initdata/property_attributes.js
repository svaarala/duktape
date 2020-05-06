'use strict';

const { createBareObject } = require('../../util/bare');

// Property attributes lookup, map metadata attribute string into a
// C initializer.  Repeated a lot of times so use a short define.
const propAttrLookup = createBareObject({
    '':    'DUK__PDF_NONE',
    'w':   'DUK__PDF_W',
    'e':   'DUK__PDF_E',
    'c':   'DUK__PDF_C',
    'we':  'DUK__PDF_WE',
    'wc':  'DUK__PDF_WC',
    'ec':  'DUK__PDF_EC',
    'wec': 'DUK__PDF_WEC',
    'a':   'DUK__PDF_A',
    'ea':  'DUK__PDF_EA',
    'ca':  'DUK__PDF_CA',
    'eca': 'DUK__PDF_ECA'
});
exports.propAttrLookup = propAttrLookup;

function emitPropAttrDefines(genc) {
    genc.emitLine('/* Shorthand for property flag defines to reduce file size. */');
    genc.emitDefine('DUK__PDF_NONE', 'DUK_PROPDESC_FLAGS_NONE');
    genc.emitDefine('DUK__PDF_W', 'DUK_PROPDESC_FLAGS_W');
    genc.emitDefine('DUK__PDF_E', 'DUK_PROPDESC_FLAGS_E');
    genc.emitDefine('DUK__PDF_C', 'DUK_PROPDESC_FLAGS_C');
    genc.emitDefine('DUK__PDF_WE', 'DUK_PROPDESC_FLAGS_WE');
    genc.emitDefine('DUK__PDF_WC', 'DUK_PROPDESC_FLAGS_WC');
    genc.emitDefine('DUK__PDF_EC', 'DUK_PROPDESC_FLAGS_EC');
    genc.emitDefine('DUK__PDF_WEC', 'DUK_PROPDESC_FLAGS_WEC');
    genc.emitDefine('DUK__PDF_A', '(DUK_PROPDESC_FLAGS_NONE|DUK_PROPDESC_FLAG_ACCESSOR)');
    genc.emitDefine('DUK__PDF_EA', '(DUK_PROPDESC_FLAGS_E|DUK_PROPDESC_FLAG_ACCESSOR)');
    genc.emitDefine('DUK__PDF_CA', '(DUK_PROPDESC_FLAGS_C|DUK_PROPDESC_FLAG_ACCESSOR)');
    genc.emitDefine('DUK__PDF_ECA', '(DUK_PROPDESC_FLAGS_EC|DUK_PROPDESC_FLAG_ACCESSOR)');
}
exports.emitPropAttrDefines = emitPropAttrDefines;
