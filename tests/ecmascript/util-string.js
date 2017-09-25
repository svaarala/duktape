/* Trivial string checksum used to summarize brute force output lines
 * (minimizes test case size).
 */
function checksumString(x) {
    var i, n;
    var res = 0;
    var mult = [ 1, 3, 5, 7, 11, 13, 17, 19, 23 ];

    n = x.length;
    for (i = 0; i < n; i++) {
        res += x.charCodeAt(i) * mult[i % mult.length];
        res = res >>> 0;  // coerce to 32 bits
    }

    return res;
}

// Escape a string with codepoint escaping.
function safeEscapeString(s) {
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

function safePrintString(s) {
    print(safeEscapeString(s));
}

function sanitizeTraceback(x) {
    x = x.replace(/\(.*?test-.*?\.js:/g, '(TESTCASE:');
    x = x.replace(/:\d+/g, ':NNN');
    x = x.replace(/\/\*/g, '(*').replace(/\*\u002f/g, '*)');
    x = x.replace(/light_[0-9a-fA-F]+_/g, 'light_PTR_', x);
    x = x.replace(/LIGHT_[0-9a-fA-F]+_/g, 'LIGHT_PTR_', x);
    return x;
}

function sanitizePointers(x) {
    return x.replace(/\((?:0x)?[0-9a-fA-F]+\)/g, '(PTR)');
}
