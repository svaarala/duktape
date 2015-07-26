/*
 *  If an error has own property fileName and/or lineNumber, they override
 *  fileName and/or lineNumber computed by the Error.prototype accessors of
 *  the same.
 *
 *  If tracebacks are disabled, behavior is similar: if an error object has
 *  own property fileName and/or lineNumber, they are not overwritten when
 *  the error is augmented.
 *
 *  The own properties need to be set using Object.defineProperty() because
 *  otherwise the inherited setter would be invoked, and that operation is
 *  currently a no-op.
 */

/*===
error 1
my filename
error 2
12345
===*/

function FooError() {
	Object.defineProperty(this, 'fileName', { value: 'my filename' });
}
FooError.prototype = Error.prototype;

function BarError() {
	Object.defineProperty(this, 'lineNumber', { value: 12345 });
}
BarError.prototype = Error.prototype;

var e1 = new FooError();
print('error 1');
print(e1.fileName);

var e2 = new BarError();
print('error 2');
print(e2.lineNumber);
