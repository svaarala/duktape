/*===
SyntaxError
SyntaxError
===*/

/*---
{
    "use_strict": true
}
---*/

'use strict';

// Labelled function declarations are rejected altogether.

try {
     eval('label1:\n' +
          '  function a() {\n' +
          '    print("a called");\n' +
          '  }\n' +
          'a();\n');
} catch (e) {
    print(e.name);
}

function test() {
    try {
        eval('label2:\n' +
             '  function b() {\n' +
             '     print("b called");\n' +
             '  }\n' +
             'b();\n');
    } catch (e) {
        print(e.name);
    }
}
test();
