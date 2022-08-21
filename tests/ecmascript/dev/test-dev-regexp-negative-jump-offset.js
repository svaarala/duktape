/*
 *  Exercise some corner cases for RegExp compiler negative jump offset
 *  computation.
 */

/*===
62
1021
32764
1048571
33554426
1073741817 skipped
done
===*/

function build(n) {
    var any = '.'.repeat(n);
    var re_str = '^(?:' + any + ')+x$';
    var input = any + any + 'x';
    return { re_str: re_str, input: input };
}

function test() {
    [ 0x3e, 0x3fd, 0x7ffc, 0xffffb, 0x1fffffa, 0x3ffffff9 ].forEach(function (v) {
        var i;

        if (v >= 64 * 1024 * 1024) {
            print(v, 'skipped');  // cannot test
            return;
        }

        print(v);
        for (i = -20; i <= 3; i++) {
            if (v + i < 1) { continue; }
            //print(v, v.toString(16), i);
            var t = build(v + i);
            //print(Duktape.enc('jx', t));
            var re = new RegExp(t.re_str);
            var res = re.test(t.input);
            //print(v, i, v + i, res);
            if (res !== true) {
                throw new Error('failed for v + i = ' + (v + i));
            }
        }
    });
}

try {
    test();
    print('done');
} catch (e) {
    print((e.stack || e).substr(0, 1000));
}
