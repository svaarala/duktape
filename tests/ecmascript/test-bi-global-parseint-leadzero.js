/*
 *  parseInt() has no auto octal behavior and will parse octal
 *  look-alikes as decimal.
 */

/*===
777
778
779
===*/

try {
    print(parseInt('0777'));
    print(parseInt('0778'));
    print(parseInt('0779'));
} catch (e) {
    print(e.stack || e);
}
