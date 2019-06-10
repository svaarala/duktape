/*
 *  https://github.com/svaarala/duktape/issues/2032
 */

/*===
RangeError
done
===*/

try {
    f = function( ) { };
    Object.defineProperty(f, 'length', { get : new Proxy ( Uint32Array , TextEncoder )});

    // Expected result is call stack exhaustion, due to call handling
    // triggering a Proxy trap (= another call), and that trap then
    // triggering another Proxy trap, etc.
    //
    // In more detail:
    //
    //   1. The .bind() call reads f.length which is a getter.
    //   2. The getter is a callable Proxy (Uint32Array constructor) so
    //      its 'apply' trap is read.
    //   3. Because the handler object is TextEncoder, also callable,
    //      it has an 'apply' function which is identified as the trap
    //      (although it's not intended to be used as one).
    //   4. The apply "trap" (Function.prototype.apply) gets called
    //      (this is handled inline by duk_js_call.c in practice),
    //      with arguments: (target, thisArg, argArray).
    //   5. Function.prototype.apply identifies its first parameter
    //      as a 'this' binding, and the second parameter as an argument
    //      array (or array-like), here 'f'.  It will then try to unpack
    //      the "argument array" (here 'f') to the value stack, and to
    //      do that, f.length is read again.  This causes a proxy trap,
    //      leading back to step 2, and infinite loop.

    JSON = f.bind(null, /x{11,111111111}/ );
} catch (e) {
    print(e.name);
}

print('done');
