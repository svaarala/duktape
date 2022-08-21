/* Previous versions of CBOR extra would encode Symbols (represented
 * internally as strings) as is:
 *
 * duk> CBOR.encode(Symbol('foo'))
 * = |6881666f6fff302d31|
 *
 * Check that decoded strings representing any kind of Symbol are
 * rejected on decode, at least for now.
 */

/*===
TypeError
===*/

try {
    print(String(CBOR.decode(new Uint8Array([ 0x68, 0x81, 0x66, 0x6f, 0x6f, 0xff, 0x30, 0x2d, 0x31 ]))));
} catch (e) {
    print(e.name);
}
