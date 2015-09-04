/*
 *  Labelled statement (E5 Section 12.12).
 */

/*===
label block
label variable
label empty
label expression
label if
then
label iteration
label continue
label break
label return
label with
123
label switch
label throw
myerror
label try
myerror2
label debugger
label another label
multi-labelled statement
done
===*/

/* Basic syntax check: label may be added to any Statement. */

function basicSyntaxTest() {
    var i;

    print('label block');
    label_block:
    {
    }

    print('label variable');
    label_variable:
    var test = 123;

    print('label empty');
    label_empty:
    ;

    print('label expression');
    label_expression:
    1 + 2;

    print('label if');
    label_if:
    if (1) { print('then'); } else { print('else'); }

    print('label iteration');
    label_iteration_dowhile:
    do { } while (0);
    label_iteration_while:
    while (0) { };
    label_iteration_for1:
    for (i = 0; i < 10; i++) { }
    label_iteration_for2:
    for (var j = 0; j < 10; j++) { }
    label_iteration_for3:
    for (i in [1, 2, 3]) { }
    label_iteration_for4:
    for (var k in [4, 5, 6]) { }

    print('label continue');
    for (i = 0; i < 10; i++) {
        label_continue:
        continue;
    }

    print('label break');
    for (;;) {
        label_break:
        break;
    }

    print('label return');
    (function returntest() { label_return: return 123; })();

    print('label with');
    label_with:
    with ({ foo: 123 }) {
        print(foo);
    }

    print('label switch');
    label_switch:
    switch (123) {
    }

    print('label throw');
    try {
        label_throw:
        throw 'myerror';
    } catch (e) {
        print(e);
    }

    print('label try');
    label_try:
    try {
        throw 'myerror2';
    } catch (e) {
        print(e);
    }

    print('label debugger');
    label_debugger:
    debugger;

    print('label another label');
    label_label1:
    label_label2:
    label_label3:
    print('multi-labelled statement');

    print('done');
}

try {
    basicSyntaxTest();
} catch (e) {
    print(e.stack || e);
}

/*===
ok
0 undefined
ok
ok
1 undefined
2 SyntaxError
3 SyntaxError
===*/

/* Label must not be repeated in the same active label set. */

function duplicateLabelTest() {
    [
        // base case
        'label: print("ok");',

        // label may be reused in a different active label set
        'label: print("ok");\nlabel: print("ok");',

        // duplicate label directly
        'label: label: print("fail");',

        // duplicate label in nested statement
        'label: for (;false;) { label: print("fail"); }'
    ].forEach(function (evalCode, i) {
        try {
            var t = eval(evalCode);
            print(i, t);
        } catch (e) {
            print(i, e.name);
        }
    });
}

try {
    duplicateLabelTest();
} catch (e) {
    print(e.stack || e);
}

/* XXX: Add basic break/continue and label behavior tests.  These
 * are covered by other tests though.
 */
