/*
 *  Test presence of custom Function properties.
 *
 *  This testcase breaks with DUK_OPT_NONSTD_FUNC_CALLER_PROPERTY.
 */

/*---
{
    "comment": "breaks with DUK_OPT_NONSTD_FUNC_CALLER_PROPERTY"
}
---*/

/*===
name: true, fileName: true, length: true, caller: false, arguments: false, callee: false, prototype: true
name: true, fileName: true, length: true, caller: true, arguments: true, callee: false, prototype: true
name: true, fileName: true, length: true, caller: true, arguments: true, callee: false, prototype: false
name: true, fileName: true, length: true, caller: true, arguments: true, callee: false, prototype: false
===*/

function test() {
    function f1() {};
    function f2() { 'use strict'; };
    var f3 = f1.bind('mythis');
    var f4 = f2.bind('mythis');

    function dump(f) {
        print('name: ' + f.hasOwnProperty('name') + ', ' +
              'fileName: ' + f.hasOwnProperty('fileName') + ', ' +
              'length: ' + f.hasOwnProperty('length') + ', ' +
              'caller: ' + f.hasOwnProperty('caller') + ', ' +
              'arguments: ' + f.hasOwnProperty('arguments') + ', ' +
              'callee: ' + f.hasOwnProperty('callee') + ', ' +  // never present, just checking
              'prototype: ' + f.hasOwnProperty('prototype'));   // explicitly mentioned in E5.1 that not present for bound functions
    }

    dump(f1);
    dump(f2);
    dump(f3);
    dump(f4);
}

try {
    test();
} catch(e) {
    print(e);
}
