/*
 *  Use eval() indirectly with call() or apply().
 */

/*===
apply
hello from eval!
call
hello from eval!
bind
hello from eval!
===*/

function test() {
    var code = 'print("hello from eval!");';
    var f;

    print('apply');
    eval.apply(this, [ code ]);

    print('call');
    eval.call(this, code);

    /* Also test indirect use of eval through bind() */
    print('bind');
    f = eval.bind(this, code);
    f();
}

try {
    test();
} catch (e) {
    print(e);
}
