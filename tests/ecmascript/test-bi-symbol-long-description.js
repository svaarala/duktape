/*
 *  Symbol with a huge description.
 */

/*===
200000008
200000008
done
===*/

function test() {
    var s;

    s = Symbol('x\ucafe'.repeat(1e8));
    print(String(s).length);

    s = Symbol.for('x\ucafe'.repeat(1e8));
    print(String(s).length);

    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
