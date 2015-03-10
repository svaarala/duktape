/*
 *  https://github.com/svaarala/duktape/issues/132
 *
 *  Does not seem to capture all of the problematic behavior in babel.js
 *  referenced in the issue.
 */

/*===
done
===*/

function genfunc(n) {
    var res = [];
    var i;
    res.push('(function test' + n + '() {');
    for (i = 0; i < n; i++) {
        res.push('    var _v' + i + ' = ' + i + ';');
    }
    res.push('    var k;');
    res.push('    var ret = 0;');
    res.push('    for (k in { "foo": "bar", "bar": "quux" }) {');
    res.push('        ret++;');
    res.push('    }');
    res.push('    return ret;');
    res.push('})');
    return res.join('\n');
}

function test() {
    var i;

    for (i = 128; i < 384; i++) {
        try {
            var src = genfunc(i);
            //print(src);
            var fn = eval(src);
            var ret = fn();
            if (ret != 2) { throw new Error('invalid return value: ' + ret); }
        } catch (e) {
            print(i, e);
        }
    }

    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
