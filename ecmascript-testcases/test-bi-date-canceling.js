/*===
1970-01-01T00:00:00.000Z
1970-01-01T00:00:00.000Z
===*/

function hugeCancelingComponents() {
    var d = new Date(0);
    print(d.toISOString());

    // This call does not change the internal time value of the Date:
    // the second and millisecond counts will be converted with
    // ToInteger() and will cancel out before TimeClip() is applied.
    //
    // Note, in particular, that huge components (which are outside
    // even 64-bit range) must not lead to an invalid time value if
    // they "cancel out".  For the implementation, this means that
    // Date setters must operate with doubles.

    d.setUTCSeconds(1e120,-1e123)
    print(d.toISOString());
}

try {
    hugeCancelingComponents();
} catch (e) {
    print(e);
}
