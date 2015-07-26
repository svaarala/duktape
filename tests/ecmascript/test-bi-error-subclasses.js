/*
 *  Test Error subclasses (TypeError etc) and their inheritance.
 */

/*===
Error my Error
EvalError my EvalError
RangeError my RangeError
ReferenceError my ReferenceError
SyntaxError my SyntaxError
TypeError my TypeError
URIError my URIError
===*/

try {
    // custom property for checking inheritance explicitly
    Error.prototype.my_class = 'Error';
    EvalError.prototype.my_class = 'EvalError';
    RangeError.prototype.my_class = 'RangeError';
    ReferenceError.prototype.my_class = 'ReferenceError';
    SyntaxError.prototype.my_class = 'SyntaxError';
    TypeError.prototype.my_class = 'TypeError';
    URIError.prototype.my_class = 'URIError';

    var errors = [
        new Error('my Error'),
        new EvalError('my EvalError'),
        new RangeError('my RangeError'),
        new ReferenceError('my ReferenceError'),
        new SyntaxError('my SyntaxError'),
        new TypeError('my TypeError'),
        new URIError('my URIError')
    ];

    var i;
    for (i = 0; i < errors.length; i++) {
        var e = errors[i];
        print(e.my_class, e.message);
    }
} catch (e) {
    print(e);
}
