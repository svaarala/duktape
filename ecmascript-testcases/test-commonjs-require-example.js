/*
 *  Example adapted from http://wiki.commonjs.org/wiki/Modules/1.1.1
 */

/*===
Duktape.modSearch program
Duktape.modSearch increment
Duktape.modSearch math
2
===*/

Duktape.modSearch = function (id) {
    print('Duktape.modSearch', id);
    return {
        math: 'exports.add = function() {\n' +
              '    var sum = 0, i = 0, args = arguments, l = args.length;\n' +
              '    while (i < l) {\n' +
              '        sum += args[i++];\n' +
              '    }\n' +
              '    return sum;\n' +
              '};\n',
        increment: 'var add = require(\'math\').add;\n' +
                   'exports.increment = function(val) {\n' +
                   '    return add(val, 1);\n' +
                   '};\n',
        program: 'var inc = require(\'increment\').increment;\n' +
                 'var a = 1;\n' +
                 'print(inc(a));\n'
    }[id];
};

try {
    require('program');
} catch (e) {
    print(e);
}
