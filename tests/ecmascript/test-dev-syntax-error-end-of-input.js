/*---
{
    "custom": true
}
---*/

/*===
ERROR, contains "end of input": true
ERROR, contains "end of input": true
ERROR, contains "end of input": false
ERROR, contains "end of input": true
ERROR, contains "end of input": true
ERROR, contains "end of input": true
ERROR, contains "end of input": true
ERROR, contains "end of input": true
ERROR, contains "end of input": true
ERROR, contains "end of input": true
ERROR, contains "end of input": true
ERROR, contains "end of input": true
ERROR, contains "end of input": false
SUCCESS: undefined
SUCCESS: 123
===*/

[
    'var x =',
    'var x =;',
    'var x =; var y = 1;',  // error not at end of input
    'var x = 1e',
    'var x = 1 +',
    'var x = "',
    'var x = "foo bar',
    'var x = { foo: 123',
    'var x = { foo: 123, ',
    'var x = { foo: 123, bar',
    'var x = { foo: 123, bar:',
    'var x = { foo: 123, bar: 234',
    'var x = { foo: 123, bar: 234 var z = 1;',  // error not at end of input
    'var x = { foo: 123, bar: 234 }',  // no error
    '123'  // no error
].forEach(function (v) {
    try {
        print('SUCCESS: ' + eval(v));
    } catch (e) {
        //print(e.message);
        print('ERROR, contains "end of input": ' + (e.message.indexOf('end of input') >= 0));
    }
});
