/*
 *  Basic property read performance for an inherited property
 */

function test() {
    var root = { xxx1: 1, xxx2: 2, xxx3: 3, xxx4: 4, foo: 123, 1234: 'foo' };
    var i;
    var ign;
    var obj = Object.create(root);
    obj = Object.create(obj);  // two levels of inheritance
    if (typeof Duktape !== 'undefined') { Duktape.compact(root); Duktape.compact(obj); }

    for (i = 0; i < 1e7; i++) {
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
    }
    print(ign);
}

test();
