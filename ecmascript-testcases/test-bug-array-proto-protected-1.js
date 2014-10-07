/*
 *  If the Array.prototype contains numeric elements which are write protected
 *  (but configurable), it should be possible to create an array literal with
 *  an overriding value.  A normal property assignment would fail because the
 *  inherited property is not writable.  An array initializer is supposed to
 *  use [[DefineOwnProperty]] which allows an own property to be created even
 *  if there is an inherited property which would normally prevent a write.
 */

/*===
defineProperty success
0 inherit undefined
2 foo bar
0 inherit undefined
2 shouldsucceed bar
===*/

function test() {
    var arr;

    Object.defineProperty(Array.prototype,
                          '0',
                          {
                              value: 'inherit',
                              writable: false,
                              enumerable: true,
                              configurable: true
                          });
    print('defineProperty success');

    // No problem here: '0' not accessed
    arr = [];
    print(arr.length, arr[0], arr[1]);

    // Again should succeed
    var arr = ['foo', 'bar'];
    print(arr.length, arr[0], arr[1]);

    // Assignment should not work unless an own property already exists
    arr = [];
    arr[0] = 'shouldfail';  // failure is silent
    print(arr.length, arr[0], arr[1]);

    arr = ['foo', 'bar'];
    arr[0] = 'shouldsucceed';
    print(arr.length, arr[0], arr[1]);
}

try {
    test();
} catch (e) {
    print(e);
}
