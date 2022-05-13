/*===
0
TypeError
0
TypeError
number 0
TypeError
number 0
TypeError
string 0
TypeError
string -0
set -0
string +0
set +0
string 0.0
set 0.0
number 1
TypeError
string 1
TypeError
string 1.0
set 1.0
number 2
TypeError
string 2
TypeError
string 2.0
set 2.0
number 3
set 3
string 3
set 3
string 3.0
set 3.0
number 4
set 4
string 4
set 4
string 4.0
set 4.0
===*/

function test() {
    'use strict';

    [ '0', '-0', '+0', '0.0', '1', '1.0', '2', '2.0', '3', '3.0', '4', '4.0' ].forEach(function (v) {
        Object.defineProperty(String.prototype, v, { get: function () { print('get ' + v); }, set: function () { print('set ' + v); } });
    });

    var str = 'foo';

    print('0');
    try {
        str[0] = 'f';
    } catch (e) {
        print(e.name);
    }
    print('0');
    try {
        str[0] = 'x';
    } catch (e) {
        print(e.name);
    }

    [ 0, -0, '0', '-0', '+0', '0.0',
      1, '1', '1.0',
      2, '2', '2.0',
      3, '3', '3.0',
      4, '4', '4.0' ].forEach(function (v) {
        print(typeof v, v);
        try {
            str[v] = 'x';
        } catch (e) {
            print(e.name);
        }
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
