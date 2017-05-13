/*
 *  Test that .bind(), .call(), and .apply() can be used in various
 *  combinations.
 */

/*===
[object global] 1 2 3 undefined undefined undefined undefined undefined undefined undefined
arguments: 3 1 2 3 undefined undefined undefined undefined undefined undefined undefined undefined undefined
mythis 101 102 undefined undefined undefined undefined undefined undefined undefined undefined
arguments: 2 101 102 undefined undefined undefined undefined undefined undefined undefined undefined undefined undefined
mythis 101 102 103 undefined undefined undefined undefined undefined undefined undefined
arguments: 3 101 102 103 undefined undefined undefined undefined undefined undefined undefined undefined undefined
mythis 101 102 103 104 undefined undefined undefined undefined undefined undefined
arguments: 4 101 102 103 104 undefined undefined undefined undefined undefined undefined undefined undefined
mythis 101 102 103 104 105 undefined undefined undefined undefined undefined
arguments: 5 101 102 103 104 105 undefined undefined undefined undefined undefined undefined undefined
mythis 101 102 103 104 105 106 undefined undefined undefined undefined
arguments: 6 101 102 103 104 105 106 undefined undefined undefined undefined undefined undefined
mythis 101 102 103 104 105 106 107 undefined undefined undefined
arguments: 7 101 102 103 104 105 106 107 undefined undefined undefined undefined undefined
boundcallthis 201 202 303 304 undefined undefined undefined undefined undefined undefined
arguments: 4 201 202 303 304 undefined undefined undefined undefined undefined undefined undefined undefined
boundcallthis 201 202 303 304 undefined undefined undefined undefined undefined undefined
arguments: 4 201 202 303 304 undefined undefined undefined undefined undefined undefined undefined undefined
mythis 101 202 203 304 305 undefined undefined undefined undefined undefined
arguments: 5 101 202 203 304 305 undefined undefined undefined undefined undefined undefined undefined
mythis 101 202 203 304 305 undefined undefined undefined undefined undefined
arguments: 5 101 202 203 304 305 undefined undefined undefined undefined undefined undefined undefined
boundapplythis 201 202 undefined undefined undefined undefined undefined undefined undefined undefined
arguments: 2 201 202 undefined undefined undefined undefined undefined undefined undefined undefined undefined undefined
boundapplythis 201 202 undefined undefined undefined undefined undefined undefined undefined undefined
arguments: 2 201 202 undefined undefined undefined undefined undefined undefined undefined undefined undefined undefined
mythis 101 202 203 undefined undefined undefined undefined undefined undefined undefined
arguments: 3 101 202 203 undefined undefined undefined undefined undefined undefined undefined undefined undefined
mythis 101 202 203 undefined undefined undefined undefined undefined undefined undefined
arguments: 3 101 202 203 undefined undefined undefined undefined undefined undefined undefined undefined undefined
===*/

function test() {
    var f0 = function f0(a, b, c, d, e, f, g, h, i, j) {
        print(this, a, b, c, d, e, f, g, h, i, j);
        print('arguments:', arguments.length,
              arguments[0], arguments[1], arguments[2], arguments[3],
              arguments[4], arguments[5], arguments[6], arguments[7],
              arguments[8], arguments[9], arguments[10], arguments[11]);
    };

    // Direct call.
    f0(1, 2, 3);

    // Bound call.
    var f1 = f0.bind('mythis', 101);
    f1(102);

    // Bound call, call via .call().
    f1.call('callthis', 102, 103);

    // Bound call, call via .call(); but use .apply() to call .call().
    f1.call.apply(f1, [ 'callthis', 102, 103, 104 ]);

    // Bound call, call via .apply().
    f1.apply('applythis', [ 102, 103, 104, 105 ]);

    // Bound call, call via .apply(); but use .call() to call .apply().
    f1.apply.call(f1, 'applythis', [ 102, 103, 104, 105, 106 ]);

    // Bound call, call via .apply(); but use .apply() to call .apply().
    f1.apply.apply(f1, [ 'applythis', [ 102, 103, 104, 105, 106, 107 ] ]);

    // Use .bind() on .call().
    var f2 = f0.call.bind(f0, 'boundcallthis', 201, 202);
    f2(303, 304);
    f2.call('ignored', 303, 304);
    var f3 = f1.call.bind(f1, 'boundcallthis', 202, 203);
    f3(304, 305);
    f3.apply('ignored', [ 304, 305 ]);

    // Use .bind() on .apply().
    var f4 = f0.apply.bind(f0, 'boundapplythis', [ 201, 202 ]);
    f4(303, 304);
    f4.call('ignored', 303, 304);  // these args get ignored because they don't affect [ 201, 202 ]
    var f5 = f1.apply.bind(f1, 'boundapplythis', [ 202, 203 ]);
    f5(304, 305);
    f5.apply('ignored', [ 304, 305 ]);  // same here, [ 304, 305 ] is ignored by .apply()
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
