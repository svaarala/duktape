/*
 *  Mutate an LHS property access in the RHS.
 */

/*===
{"foo":234,"bar":345}
{"foo":234}
{"foo":234,"bar":345}
{"foo":234}
===*/

function test1() {
    var alt1 = { foo: 234 };
    var alt2 = { foo: 234 };
    var obj;

    obj = alt1;
    obj.bar = (obj = alt2, 345);
    print(JSON.stringify(alt1));
    print(JSON.stringify(alt2));
}

function test2() {
    var alt1 = { foo: 234 };
    var alt2 = { foo: 234 };
    var obj;

    obj = alt1;
    obj['bar'] = (obj = alt2, 345);
    print(JSON.stringify(alt1));
    print(JSON.stringify(alt2));
}

try {
    test1();
    test2();
} catch (e) {
    print(e.stack || e);
}
