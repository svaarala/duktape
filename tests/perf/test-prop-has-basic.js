/*
 *  Basic property has performance
 */

function test() {
    var obj = {};
    var i;
    var ign;

    for (i = 0; i < 1024 - 1; i++) {
        obj['prop-' + i] = 1;
    }
    obj['foo'] = 123;

    for (i = 0; i < 1e7; i++) {
        ign = 'foo' in obj;
        ign = 'foo' in obj;
        ign = 'foo' in obj;
        ign = 'foo' in obj;
        ign = 'foo' in obj;
        ign = 'foo' in obj;
        ign = 'foo' in obj;
        ign = 'foo' in obj;
        ign = 'foo' in obj;
        ign = 'foo' in obj;
    }
}

test();
