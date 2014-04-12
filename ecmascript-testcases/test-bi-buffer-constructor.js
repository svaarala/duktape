/*
 *  Buffer constructor tests.
 */

/*===
plain fixed 10 bytes: 0 0 0 0 0 0 0 0 0 0
plain dynamic 10 bytes: 0 0 0 0 0 0 0 0 0 0
plain fixed 10 bytes: 0 0 0 0 0 0 0 0 0 0
plain fixed 10 bytes: 0 0 0 0 0 0 0 0 0 0
object fixed 10 bytes: 0 0 0 0 0 0 0 0 0 0
object dynamic 10 bytes: 0 0 0 0 0 0 0 0 0 0
object fixed 10 bytes: 0 0 0 0 0 0 0 0 0 0
object fixed 10 bytes: 0 0 0 0 0 0 0 0 0 0
plain fixed 9 bytes: 102 111 111 98 97 114 225 136 180
plain dynamic 9 bytes: 102 111 111 98 97 114 225 136 180
plain fixed 9 bytes: 102 111 111 98 97 114 225 136 180
plain fixed 9 bytes: 102 111 111 98 97 114 225 136 180
object fixed 9 bytes: 102 111 111 98 97 114 225 136 180
object dynamic 9 bytes: 102 111 111 98 97 114 225 136 180
object fixed 9 bytes: 102 111 111 98 97 114 225 136 180
object fixed 9 bytes: 102 111 111 98 97 114 225 136 180
plain fixed 3 bytes: 102 111 111
plain fixed 3 bytes: 102 111 111
plain fixed 3 bytes: 102 111 111
plain fixed 3 bytes: 102 111 111
buffer is the same: true
object fixed 3 bytes: 102 111 111
object fixed 3 bytes: 102 111 111
object fixed 3 bytes: 102 111 111
object fixed 3 bytes: 102 111 111
buffer is the same: true
plain dynamic 3 bytes: 98 97 114
plain dynamic 3 bytes: 98 97 114
plain dynamic 3 bytes: 98 97 114
plain dynamic 3 bytes: 98 97 114
buffer is the same: true
object dynamic 3 bytes: 98 97 114
object dynamic 3 bytes: 98 97 114
object dynamic 3 bytes: 98 97 114
object dynamic 3 bytes: 98 97 114
buffer is the same: true
plain fixed 4 bytes: 113 117 117 120
buffer is the same: true
object fixed 4 bytes: 113 117 117 120
buffer is the same: true
object is the same: false
undefined TypeError
undefined TypeError
null TypeError
null TypeError
boolean TypeError
boolean TypeError
boolean TypeError
boolean TypeError
object TypeError
object TypeError
object TypeError
object TypeError
function TypeError
function TypeError
pointer TypeError
pointer TypeError
===*/

function isFixed(buf) {
    /* Currently this is hard to see from Ecmascript side, use
     * Duktape.info().
     */
    if (typeof buf !== 'buffer') {
        throw new Error('not a buffer');
    }

    var t = Duktape.info(buf);
    if (t[4] !== undefined) {
        return false;
    }
    return true;
}

function dump(buf) {
    var is_plain;
    if (typeof buf === 'buffer') {
        is_plain = true;
    } else if (buf instanceof Duktape.Buffer) {
        is_plain = false;
        buf = buf.valueOf();
    } else {
        throw new Error('not a buffer');
    }

    print(is_plain ? "plain" : "object",
          isFixed(buf) ? 'fixed' : 'dynamic',
          buf.length,
          'bytes:',
          Array.prototype.map.call(buf, function(v) { return String(v); }).join(' '));
}

function test() {
    var b;
    var str = 'foobar\u1234';

    /* Number argument: creates new buffer of specified size,
     * filled with zeroes by default (this can be disabled with
     * an extra option but is the standard behavior).
     */

    dump(Duktape.Buffer(10));
    dump(Duktape.Buffer(10, true));
    dump(Duktape.Buffer(10, false));
    dump(Duktape.Buffer(10, false, 'ignored'));

    dump(new Duktape.Buffer(10));
    dump(new Duktape.Buffer(10, true));
    dump(new Duktape.Buffer(10, false));
    dump(new Duktape.Buffer(10, false, 'ignored'));

    /* String argument: create a buffer with bytes from the string
     * internal representation.
     */

    dump(Duktape.Buffer(str));
    dump(Duktape.Buffer(str, true));
    dump(Duktape.Buffer(str, false));
    dump(Duktape.Buffer(str, false, 'ignored'));

    dump(new Duktape.Buffer(str));
    dump(new Duktape.Buffer(str, true));
    dump(new Duktape.Buffer(str, false));
    dump(new Duktape.Buffer(str, false, 'ignored'));

    /* Plain buffer argument: return as is, no new buffer is created,
     * and the buffer dynamic/fixed nature is not changed.
     */

    b = Duktape.Buffer('foo');

    dump(Duktape.Buffer(b));
    dump(Duktape.Buffer(b, true));
    dump(Duktape.Buffer(b, false));
    dump(Duktape.Buffer(b, false, 'ignored'));
    print('buffer is the same:', (Duktape.Buffer(b)).valueOf() === b);

    dump(new Duktape.Buffer(b));
    dump(new Duktape.Buffer(b, true));
    dump(new Duktape.Buffer(b, false));
    dump(new Duktape.Buffer(b, false, 'ignored'));
    print('buffer is the same:', (new Duktape.Buffer(b)).valueOf() === b);

    b = Duktape.Buffer('bar', true);

    dump(Duktape.Buffer(b));
    dump(Duktape.Buffer(b, true));
    dump(Duktape.Buffer(b, false));
    dump(Duktape.Buffer(b, false, 'ignored'));
    print('buffer is the same:', (Duktape.Buffer(b)).valueOf() === b);

    dump(new Duktape.Buffer(b));
    dump(new Duktape.Buffer(b, true));
    dump(new Duktape.Buffer(b, false));
    dump(new Duktape.Buffer(b, false, 'ignored'));
    print('buffer is the same:', (new Duktape.Buffer(b)).valueOf() === b);

    /* Buffer object argument: if called as a plain function, return the
     * plain buffer; if called as a constructor, create a new Buffer object
     * with the same plain value inside.  This is the same as String behavior.
     */

    b = new Duktape.Buffer('quux');

    dump(Duktape.Buffer(b));
    print('buffer is the same:', b.valueOf() === Duktape.Buffer(b));

    dump(new Duktape.Buffer(b));
    print('buffer is the same:', b.valueOf() === new Duktape.Buffer(b).valueOf());
    print('object is the same:', b === new Duktape.Buffer(b));

    /* Other types: TypeErrors for now. */

    function catchtest(v) {
        try {
            dump(Duktape.Buffer(v));
        } catch (e) {
            print(v === null ? 'null' : typeof v, e.name);
        }

        try {
            dump(new Duktape.Buffer(v));
        } catch (e) {
            print(v === null ? 'null' : typeof v, e.name);
        }
    }

    catchtest(undefined);
    catchtest(null);
    catchtest(true);
    catchtest(false);
    catchtest({ foo: 1, bar:2 });
    catchtest([ 'foo', 'bar' ]);
    catchtest(function () {});
    catchtest(Duktape.Pointer('dummy'));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
