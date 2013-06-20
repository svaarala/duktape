/*===
object 4 1,2,3,4
===*/

try {
    var t1 = [ 1, 2 ];
    var t2 = [ 3, 4 ];
    var t3;
    t2.length = 4;

    // t2.length is 4, but in E5.1 Section 15.4.4.4 the result Array
    // does not get updated for any trailing non-existent elements of
    // t2.  The result should thus have length 4, but e.g. V8 and Rhino
    // will have length 6.

    t3 = t1.concat(t2);

    print(typeof t3, t3.length, t3);
} catch (e) {
    print(e);
}

