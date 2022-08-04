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

print(eval("do{print('loop')}while(false)false"));
