/*===
65535
foo-0
foo-65535
undefined
undefined
===*/

function target() {
    print(arguments.length);
    print(this);
    print(arguments[65534]);
    print(arguments[65535]);
    print(arguments[65536]);
}

var args = [];
while (args.length < 65536) {
    args.push('foo-' + args.length);
}
var f = Function.prototype.bind.apply(target, args);
f();
