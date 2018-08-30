/*===
String(P)
Symbol(Symbol.toPrimitive)
toString
valueOf
TypeError
Number(P)
Symbol(Symbol.toPrimitive)
valueOf
toString
TypeError
done
===*/

var P = new Proxy({}, {
    get: function (a,b,c) { console.log(String(b)); return void 0; }
});

print('String(P)');
try {
    print(String(P));
} catch (e) {
    print(e.name);
}

print('Number(P)');
try {
    print(Number(P));
} catch (e) {
    print(e.name);
}

print('done');
