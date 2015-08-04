/*
 *  Testcase for a bug reported by Aurélien Bouilland:
 *
 *  The do-while loop never terminates in Duktape 0.7.0.
 */

/*===
20
22
24
26
===*/

var i = 0;
do {
    if (i < 20) continue;
    print(i);
    if (i > 25) break;
    i++;
} while (++i < 30);
