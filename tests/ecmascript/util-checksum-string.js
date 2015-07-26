/* Trivial string checksum used to summarize brute force output lines
 * (minimizes test case size).
 */
function checksumString(x) {
    var i, n;
    var res = 0;
    var mult = [ 1, 3, 5, 7, 11, 13, 17, 19, 23 ];

    n = x.length;
    for (i = 0; i < n; i++) {
        res += x.charCodeAt(i) * mult[i % mult.length];
        res = res >>> 0;  // coerce to 32 bits
    }

    return res;
}
