/*
 *  Create Object using a literal
 */

function test() {
    var obj;
    var i;

    for (i = 0; i < 1e6; i++) {
        obj = {
            key1: 'val1',
            key2: 'val2',
            key3: 'val3'
        };
    }
}

test();
