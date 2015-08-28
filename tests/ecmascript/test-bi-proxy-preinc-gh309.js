/*
 *  Preincrement applied to a Proxy (GH-309).
 */

/*===
get
set
4500.5
get
9001
===*/

function test() {
    p = new Proxy({ val: 4499.5 }, {
        get: function(t, prop) { print('get'); return t[prop]; },
        set: function(t, prop, value) { print('set'); t[prop] = value * 2; }
    });

    // Preincrement is evaluated as:
    //    (1) read p.val -> 4499.5
    //    (2) increment 4499.5 -> 4500.5 (this becomes the expression value)
    //    (3) write 4500.5 to p.val -> 'set' trap writes 9001 to target
    //
    // In this case the result may not be intuitive as one might
    // expect 9001 as a result (i.e. step 4: read p.val back).
    // The required behavior is described in:
    // http://www.ecma-international.org/ecma-262/6.0/index.html#sec-prefix-increment-operator

    print(++p.val);  // 4500.5 (from step 2)
    print(p.val);    // 9001
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
