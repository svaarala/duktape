/*
 *  https://github.com/svaarala/duktape/issues/115
 *
 *  Corner case in compiler:
 *
 *    - Pass 1 indicates shuffling is not needed
 *    - Pass 2 turns out to need shuffling after all
 *    - Shuffle registers will all be zero
 *
 *  If an instruction needs just one shuffle register, it will use register 0
 *  and corrupt function 1st argument.  If an instruction needs multiple shuffle
 *  registers, they will trample on each other and can result e.g in "CSPROPI
 *  target is not a number".
 *
 *  The root issue is that because variables are mapped to registers, pass 2
 *  may disagree on whether or not shuffling is needed: pass 1 has more registers
 *  available.
 */

/*===
test 1
test 2
test 3
done
===*/

function genfunc(nvar, nconst, gencsprop, avoidmputobj) {
    var res = [];
    var i;

    res.push('(function test(arg1) {');

    // arg1 eats one registers, variables eat a register each
    for (i = 0; i < nvar; i++) {
        res.push('    var v' + i + ';');
    }

    // each constant eats a constant index
    for (i = 0; i < nconst; i++) {
        res.push('    v0 = "tempstr' + i + '";');
    }

    // generate a CSPROP(I)
    if (gencsprop) {
        if (avoidmputobj) {
            res.push('    v' + (nvar - 1) + ' = Object.create(Object.prototype);');
            res.push('    v' + (nvar - 1) + '.fn = function() { return "csprop return"; };');
        } else {
            res.push('    v' + (nvar - 1) + ' = { fn: function() { return "csprop return"; } };');
        }
        res.push('    void v' + (nvar - 1) + '.fn();');
    }

    res.push('    return arg1;');
    res.push('})');
    return res.join('\n');
}

function test() {
    var i, j, src, fn, ret;

    // Triggers clobbering of arg1
    print('test 1');
    for (i = 1; i < 512; i++) {
        try {
            src = genfunc(i, i, false, false);
            fn = eval(src);
            ret = fn('foo');
            if (ret != 'foo') {
                print(i, ret);
            }
        } catch (e) {
            print(i, e);
        }
    }

    // Triggers "MPUTOBJ target not an object"
    print('test 2');
    for (i = 1; i < 512; i++) {
        try {
           src = genfunc(i, i, true, false);
            fn = eval(src);
            ret = fn('foo');
            if (ret != 'foo') {
                print(i, ret);
            }
        } catch (e) {
            print(i, e);
        }
    }

    // Tried to trigger the CSPROPI issue (but avoid triggering the MPUTOBJ
    // issue above), but didn't work
    print('test 3');
    for (i = 200; i < 400; i++) {
        for (j = -10; j <= 10; j++) {
            try {
                src = genfunc(i, i + j, true, false);
                fn = eval(src);
                ret = fn('foo');
                if (ret != 'foo') {
                    print(i, j, ret);
                }
            } catch (e) {
                print(i, e);
            }
        }
    }

    print('done');
}

try {
    test('foo');
} catch (e) {
    print(e.stack || e);
}
