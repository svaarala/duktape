'use strict';

function formatSymbol(sym) {
    switch (sym.variant) {
    case 'global':
        return '\x80' + sym.string;
    case 'wellknown':
        // Well known symbols use an empty suffix which never occurs for
        // runtime local symbols.
        return '\x81' + sym.string + '\xff';
    case 'userhidden':
        return '\xff' + sym.string;
    case 'hidden':  // Duktape hidden Symbol
        return '\x82' + sym.string;
    }

    throw new TypeError('invalid symbol variant: ' + sym.variant);
}
exports.formatSymbol = formatSymbol;
