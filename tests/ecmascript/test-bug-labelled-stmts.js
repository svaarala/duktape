/*
 *  Bug testcases for a few labelled statement cases.
 */

/*===
if 1
if 3
block 1
block 3
with 1
with 3
===*/

function testIf() {
    test:
    if (1) {
        print('if 1');
        break test;
        print('if 2');
    }
    print('if 3');
}

function testBlock() {
    test:
    {
        print('block 1');
        break test;
        print('block 2');
    }
    print('block 3');
}

function testWith() {

    test:
    with ({}) {
        print('with 1');
        break test;
        print('with 2');
    }
    print('with 3');
}

try {
    testIf();
} catch (e) {
    print(e.stack || e);
}
try {
    testBlock();
} catch (e) {
    print(e.stack || e);
}
try {
    testWith();
} catch (e) {
    print(e.stack || e);
}
