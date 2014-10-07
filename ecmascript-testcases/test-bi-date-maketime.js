/*===
0
3723004
10923000
-3477000
3723004
3723004
-3723004
-3723004
===*/

/* MakeTime() is not exposed directly, but setUTCHours() will call MakeTime()
 * with parameters given in the call.  It will then combine the result with
 * MakeDate(), keeping the day number same as before.  By using Jan 1, 1970
 * as the date, the day number will be 0, and the time value will thus be
 * MakeTime() result.
 *
 * However: the arguments to MakeTime() will be ToNumber() coerced by
 * setUTCHours(), and the result will go through TimeClip().
 */

function MakeTime(hour, min, sec, msec) {
    var d = new Date(0);
    d.setUTCHours(hour, min, sec, msec);
    return d.getTime();
}

try {
    // simple tests
    print(MakeTime(0, 0, 0, 0));
    print(MakeTime(1, 2, 3, 4));

    // components don't need to be normalized, and will just be added up
    // here: 1*3600e3 + 2*60e3 + 3e3 + 2*3600e3
    print(MakeTime(1, 2, 3, 2*3600e3));

    // components may also be negative
    // here: 1*3600e3 + 2*60e3 + 3e3 - 2*3600e3
    print(MakeTime(1, 2, 3, -2*3600e3));

    // components are ToInteger() rounded, which rounds towards zero
    print(MakeTime(1.1, 2.2, 3.3, 4.4));
    print(MakeTime(1.9, 2.9, 3.9, 4.9));
    print(MakeTime(-1.1, -2.1, -3.1, -4.1));
    print(MakeTime(-1.9, -2.9, -3.9, -4.9));
} catch (e) {
    print(e.name);
}
