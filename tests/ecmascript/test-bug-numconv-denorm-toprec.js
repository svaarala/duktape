/*===
1e-323
9.8813129168249308835e-324
===*/

/* Demonstrate a bug where a denormal would be formatted in exponential
 * notation with a zero leading digit:
 *
 *    0.9881312916824930884e-323
 *
 * The bug happens because Dragon4 is used for fixed form (instead of
 * free form) output without proper adaptation.  Dragon4 will determine
 * the 'k' parameter based on the assumption that free form (shortest)
 * output is used; the output will be rounded to 1e-323 in that case
 * (so the 'k' is correct for that case).
 *
 * This bug will be fixed by proper fixed format Dragon4 changes.
 */

try {
    print((0.9881312916824930884e-323).toString());
    print((0.9881312916824930884e-323).toPrecision(20));
} catch (e) {
    print(e);
}
