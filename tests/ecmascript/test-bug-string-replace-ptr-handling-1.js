/*
 *  Reported by https://github.com/bzyo.
 */

/*===
443016350
7
done
===*/

function main() {
    var v2 = "EPSILON".repeat(63288050);
    print(v2.length);
    var v3 = "EPSILON".replace(v2);
    print(v3.length);
}
try {
    main();
} catch (e) {
    print(e.stack || e);
}
print('done');
