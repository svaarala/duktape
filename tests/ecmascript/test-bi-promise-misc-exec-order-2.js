//
//    P
//    `--->   A         B         C
//           /|\       /|\       /|\
//          / | \     / | \     / | \
//         D  E  F   G  H  I   J  K  L
//               |      |      |
//               M      N      O
//                             |
//                             P

/*---
{
    "skip": true
}
---*/

/*===
done
A 123
B 123
C 123
D 123
E 123
F 123
G 123
H 123
I 123
J 123
K 123
L 123
M 123
N 123
O 123
P 123
===*/

var resolveP, rejectP;
var P = new Promise(function (resolve, reject) {
    resolveP = resolve;
    rejectP = reject;
});

function printer(name) {
    return function (v) {
        print(name, v);
        return v;
    };
}

var A = P.then(printer('A'));
var B = P.then(printer('B'));
var C = P.then(printer('C'));

var D = A.then(printer('D'));
var E = A.then(printer('E'));
var F = A.then(printer('F'));

var M = F.then(printer('M'));

var G = B.then(printer('G'));
var H = B.then(printer('H'));
var I = B.then(printer('I'));

var N = H.then(printer('N'));

var J = C.then(printer('J'));
var K = C.then(printer('K'));
var L = C.then(printer('L'));

var O = J.then(printer('O'));

var P = O.then(printer('P'));

resolveP(123);

print('done');
