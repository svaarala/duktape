/*
 *  Corner case in Arguments [[DefineOwnProperty]] when write protecting
 *  a mapped index without a [[Value]].  In this case the property [[Value]]
 *  is updated from the current mapped variable value before write
 *  protecting the index and removing the mapping.
 */

/*===
{"0":"foo","1":"bar","2":"quux"}
foo bar quux
{"0":123,"1":"bar","2":"quux"}
123 bar quux
{"0":123,"1":234,"2":"quux"}
123 234 quux
set b to 100
{"0":123,"1":100,"2":"quux"}
123 100 quux
make "1" nonwritable, no [[Value]]
{"0":123,"1":100,"2":"quux"}
123 100 quux
set b to 200
{"0":123,"1":100,"2":"quux"}
123 200 quux
make "1" 999, no longer mapped
{"0":123,"1":999,"2":"quux"}
123 200 quux
set b to 888
{"0":123,"1":999,"2":"quux"}
123 888 quux
===*/

function func(a, b, c) {
    return [ arguments, function evaler(code) { return eval(code); } ];
}

var tmp = func('foo', 'bar', 'quux');
var args = tmp[0];
var evaler = tmp[1];
function dump() {
    print(JSON.stringify(args));
    print(evaler('a'), evaler('b'), evaler('c'));
}

dump();

Object.defineProperty(args, '0', { value: 123 });
dump();

Object.defineProperty(args, '1', { value: 234 });
dump();

print('set b to 100');
evaler('b = 100');
dump();
print('make "1" nonwritable, no [[Value]]');
Object.defineProperty(args, '1', { writable: false });  // here 100 is copied to arguments
dump();
print('set b to 200');
evaler('b = 200');  // b is updated, arguments stuck at 100 (not 234)
dump();
print('make "1" 999, no longer mapped');
Object.defineProperty(args, '1', { value: 999 });  // no longer mapped
dump();
print('set b to 888');
evaler('b = 888');
dump();
