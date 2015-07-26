/*
 *  Test that we support the invalid but out-in-the-wild variation
 *  described in:
 *
 *    https://bugs.ecmascript.org/show_bug.cgi?id=8
 */

/*===
loop
false
===*/

try {
    print(eval("do{print('loop')}while(false)false"));
} catch (e) {
    print(e.name);
}
