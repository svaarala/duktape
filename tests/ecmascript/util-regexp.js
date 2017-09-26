if (typeof RegExpUtil !== 'object') {
    Object.defineProperty(new Function('return this')(), 'RegExpUtil', {
        value: {}, writable: false, enumerable: false, configurable: false
    });
}

// Get a list of character repeats in an input string.  For example, for
// 'aabbbbbbc' returns 'a2 b6 c1'.  Useful for e.g. RegExp tests.
RegExpUtil.getCharRepeats = function (x) {
    var res = [];
    var i, n, c;
    var prev = null;
    var count = 0;
    for (i = 0, n = x.length; i < n; i++) {
        c = x.charCodeAt(i);
        if (c === prev) {
            count++;
        } else {
            if (prev) {
                res.push(String.fromCharCode(prev) + count);
            }
            prev = c;
            count = 1;
        }
    }
    if (prev) {
        res.push(String.fromCharCode(prev) + count);
    }
    return res.join(' ');
};

RegExpUtil.getAllCodepointsString = function () {
    var res = [];
    for (var i = 0; i <= 0xffff; i++) {
        res.push(i);
    }
    res = String.fromCharCode.apply(String, res);
    return res;
};

RegExpUtil.getRegExpSingleCharMatches = function (re) {
    var input;
    var t = [];
    var trueStart = null, trueEnd = null;

    // Force 'global' flag for input regexp.
    if (re.flags.indexOf('g') < 0) {
        re = new RegExp(re.source, re.flags + 'g');
    }

    // Match regexp against a string containing all codepoints to speed up
    // the matching process.
    input = RegExpUtil.getAllCodepointsString();
    input.replace(re, function (v) {
        var cp = v.charCodeAt(0);
        if (trueStart !== null) {
            trueEnd++;
            if (cp == trueEnd) {
                // 'true' continues
            } else {
                // 'true' broken; here trueEnd is exclusive (+1)
                t.push('T' + (trueEnd - trueStart));
                t.push('F' + (cp - trueEnd));
                trueStart = cp;
                trueEnd = cp;
            }
        } else {
            if (cp > 0) {
                t.push('F' + cp);
            }
            trueStart = cp;
            trueEnd = cp;
        }
    });
    if (trueStart === null) {
        t.push('F' + 65536);
    } else {
        t.push('T' + (trueEnd - trueStart + 1));
        if (trueEnd < 65535) {
            t.push('F' + (65536 - trueEnd - 1));
        }
    }

    return t.join(' ');
};

RegExpUtil.getRegExpUniEsc = function (x) {
    return '\\u' + ('0000' + x.toString(16)).substr(-4);
};

RegExpUtil.makeCaseInsensitiveCharClassRegExp = function (start, end) {
    var src = '[' + RegExpUtil.getRegExpUniEsc(start) + '-' +
              RegExpUtil.getRegExpUniEsc(end) + ']';
    return new RegExp(src, 'i');
};
