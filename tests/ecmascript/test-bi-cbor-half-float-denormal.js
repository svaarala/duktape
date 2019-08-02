/*
 *  Handling of half-float denormals.
 *
 *  Decoding is covered by full coverage decode test, so only encode needs
 *  testing here.
 */

/*===
5.960464477539063e-8
fa33800000
done
===*/

// Current behavior is not to support half-float subnormals.  It's not
// required, but would be nice for completeness.

print(CBOR.decode(new Uint8Array([ 0xf9, 0x00, 0x01 ])));
print(Duktape.enc('hex', CBOR.encode(5.960464477539063e-8)));

print('done');
