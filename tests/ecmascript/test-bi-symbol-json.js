/*
 *  Symbols in JSON.
 */

/*@include util-symbol.js@*/

/*===
symbol json
undefined
undefined
[1,null,null,4]
{}
"<:Symbol(foo)>"
"<:Symbol(fooX)>"
[1,"<1:Symbol(fooXYZ)>","<2:Symbol(barXYZW)>",4]
{"foo":"<foo:Symbol(foo)>","bar":"<bar:Symbol(barX)>"}
111
112
[1,114,115,4]
{"foo":111,"bar":112}
{"foo":123}
{
>>>"foo": 123
}
{"foo":123}
[1,2,null,null,null,{}]
[1,2,null,null,null,{}]
[null,null,{},{}]
{"foo":{}}
{"foo":{"foo":"bar"}}
toJSON called
{"foo":"dummy"}
===*/

function symbolJsonTest() {
    function id(k, v) {
        return v;
    }
    function replacer1(key, value) {
        if (typeof value === 'symbol') {
            return '<' + key + ':' + String(value) + '>';
        } else {
            return value;
        }
    }
    function replacer2(key, value) {
        if (typeof value === 'symbol') {
            return 100 + String(value).length;
        } else {
            return value;
        }
    }

    var v1 = Symbol('foo');
    var v2 = Symbol.for('fooX');
    var v3 = [ 1, Symbol('fooXYZ'), Symbol.for('barXYZW'), 4 ];
    var v4 = {
        foo: Symbol('foo'),
        bar: Symbol.for('barX'),
        [ Symbol('quuxXY') ]: 'quux',
        [ Symbol.for('bazXYZ') ]: 'baz'
    };
    var tmp;

    // Symbols are ignored when serializing a value.
    print(JSON.stringify(v1));
    print(JSON.stringify(v2));
    print(JSON.stringify(v3));
    print(JSON.stringify(v4));

    // A replacer can allow symbol values to be serialized (after transforming
    // them to an allowed type like string or number) but symbol keys are
    // still ignored.
    print(JSON.stringify(v1, replacer1));
    print(JSON.stringify(v2, replacer1));
    print(JSON.stringify(v3, replacer1));
    print(JSON.stringify(v4, replacer1));
    print(JSON.stringify(v1, replacer2));
    print(JSON.stringify(v2, replacer2));
    print(JSON.stringify(v3, replacer2));
    print(JSON.stringify(v4, replacer2));

    // Symbols are ignored in a JSON.stringify() proplist.
    print(JSON.stringify({ foo: 123, bar: 234, [Symbol.for('quux')]: 345 }, [ 'foo', Symbol.for('quux') ]))

    // Symbols are ignored as gap values.
    print(JSON.stringify({ foo: 123 }, null, '>>>'));  // normal string case
    print(JSON.stringify({ foo: 123 }, null, Symbol.for('>>>')));  // same as no 'gap' argument

    // Random development time test: symbols in arrays serialize as 'null',
    // symbols as keys are ignored.  The 'id' replacer forces slow path.
    print(JSON.stringify([1, 2, Symbol(), Symbol('foo'), Symbol.for('bar'), { foo: Symbol('foo'), [Symbol.for('bar')]: 'quux' }]));
    print(JSON.stringify([1, 2, Symbol(), Symbol('foo'), Symbol.for('bar'), { foo: Symbol('foo'), [Symbol.for('bar')]: 'quux' }], id));

    // Symbol objects are serialized as is and usually end up as '{}'.
    // In particular, the internal symbol value is NOT looked up, as is
    // done for e.g. String objects.  .toJSON() works as expected.
    print(JSON.stringify([Symbol('foo'), Symbol.for('foo'), Object(Symbol('foo')), Object(Symbol.for('foo'))]));
    tmp = Object(Symbol('foo'));
    print(JSON.stringify({ foo: tmp }));
    tmp.foo = 'bar';
    print(JSON.stringify({ foo: tmp }));
    tmp.toJSON = function () { print('toJSON called'); return 'dummy' };
    print(JSON.stringify({ foo: tmp }));
}

try {
    print('symbol json');
    symbolJsonTest();
} catch (e) {
    print(e.stack || e);
}
