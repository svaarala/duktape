/*===
9
true on line 12
constructor on line 16
getter on line 21
123
===*/

print(Duktape.line())

if (true) {
    print("true on line", Duktape.line())
}

function MyConstructor() {
    print("constructor on line", Duktape.line());
}
new MyConstructor();

var obj = {
    get x() { print('getter on line', Duktape.line()); return 123; }
}
print(obj.x);

