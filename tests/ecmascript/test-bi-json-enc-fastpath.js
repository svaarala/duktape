/*
 *  JSON.stringify() fast path tests
 *
 *  Try to exercise all code paths in the fast path, and ensure that falling
 *  back to the slow path is transparent.
 */

/*===
basic test
{"foo":123,"bar":234,"quux":{"val2":null,"val3":true,"val4":false,"val5":123,"val6":123.456,"val7":"foo"},"baz":[null,null,true,false,123,123.456,"foo"]}
===*/

/* Fast path success case which should exercise all fast path code paths
 * if possible (but not aborting the fast path).
 */

function jsonStringifyFastPathBasic() {
    var val = {
        foo: 123,
        bar: 234,
        quux: {
            val1: undefined,
            val2: null,
            val3: true,
            val4: false,
            val5: 123,
            val6: 123.456,
            val7: 'foo'
        },
        baz: [
            undefined,
            null,
            true,
            false,
            123,
            123.456,
            'foo'
        ]
    };

    print(JSON.stringify(val));
}

try {
    print('basic test');
    jsonStringifyFastPathBasic();
} catch (e) {
    print(e.stack || e);
}

/*===
top level value test
0 undefined
1 null
2 true
3 false
4 123
5 123.456
6 "foo"
7 {"foo":123}
8 ["foo"]
9 undefined
10 "1970-01-01T00:00:00.123Z"
11 undefined
12 undefined
13 undefined
14 {"type":"Buffer","data":[65,66,67,68,69,70,71,72]}
===*/

/* Top level value */

function jsonStringifyFastPathTopLevelValueTest() {
    var values = [
        undefined, null, true, false, 123, 123.456, 'foo',
        { foo: 123 }, [ 'foo' ],
        function myfunc() {},
        new Date(123),
        Duktape.dec('hex', 'deadbeef'),
        new Duktape.Buffer(Duktape.dec('hex', 'deadbeef')),
        new ArrayBuffer(8),
        new Buffer('ABCDEFGH'),  // has toJSON
    ];

    values.forEach(function (v, i) {
        print(i, JSON.stringify(v));
    });
}

try {
    print('top level value test');
    jsonStringifyFastPathTopLevelValueTest();
} catch (e) {
    print(e.stack || e);
}

/*===
auto unbox test
0 123
1 "foo"
2 true
3 false
===*/

/* JSON requires automatic unboxing of the following primitive types:
 * Number, String, Boolean (E5 Section 15.12.3, Str() algorithm, step 4).
 */

function jsonStringifyFastPathAutoUnboxTest() {
    var values = [
        new Number(123),
        new String('foo'),
        new Boolean(true),
        new Boolean(false)
    ];

    values.forEach(function (v, i) {
        print(i, JSON.stringify(v));
    });
}

try {
    print('auto unbox test');
    jsonStringifyFastPathAutoUnboxTest();
} catch (e) {
    print(e.stack || e);
}

/*===
abort test
0 "foobar"
1 [null]
mygetter called
2 {}
3 [1,2,3,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,4]
4 {"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{"deeper":{}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
no-inherit [1,2,3,4,null,null,6]
inherit [1,2,3,4,null,"inherit",6]
===*/

/* Fast path is based on enumerating object properties directly without using
 * an explicit enumerator.  However, the fast path must be aborted if there's
 * danger of a side effect which might lead to mutation of the value(s) being
 * serialized.
 *
 * A value replacer might have such side effects but it's not supported in the
 * fast path at all.
 *
 * Presence of a .toJSON() method is another reason; the fast path will now
 * detect this case and abort.
 *
 * There are also some technical ones like sparse arrays etc.
 */

function jsonStringifyFastPathAbort() {
    var values = [];
    var obj;
    var i;

    // a .toJSON property aborts
    values.push({ toJSON: function () { return 'foobar'; } });

    // a lightfunc value might inherit a .toJSON, so lightfuncs always
    // cause an abort
    values.push([ Math.cos ]);  // only a lightfunc if "built-in lightfuncs" option set

    // a getter property aborts
    obj = {};
    Object.defineProperty(obj, 'mygetter', {
        get: function () {
            print('mygetter called');
            obj.foo = 'bar';  // mutate, shouldn't be visible in output
        },
        enumerable: true,
        configurable: true
    });
    values.push(obj);

    // a sparse Array aborts fast path
    obj = [ 1, 2, 3 ];
    obj[100] = 4;
    values.push(obj);

    // a non-cyclic structure which is larger than the fast path loop check
    // array (which has a fixed size, currently 32 elements) should abort
    // the fast path and *succeed* in the slow path which has a much larger
    // recursion limit.
    var deep = {};
    for (i = 0; i < 100; i++) {
        deep = { deeper: deep };
    }
    values.push(deep);

    values.forEach(function (v, i) {
        print(i, JSON.stringify(v));
    });

    // a dense gappy Array with a conflicting inherited property causes a
    // bailout (needs separate test because we modify Array.prototype)

    obj = [ 1, 2, 3, 4 ];
    obj[6] = 6;  // still dense
    print('no-inherit', JSON.stringify(obj));  // fast path OK
    Object.defineProperty(Array.prototype, '5', {
        writable: true, enumerable: true, configurable: true,
        value: 'inherit'
    });
    print('inherit', JSON.stringify(obj));  // fast path aborted
    delete Array.prototype['5'];
}

try {
    print('abort test');
    jsonStringifyFastPathAbort();
} catch (e) {
    print(e.stack || e);
}

/*===
inheritance test
parent foo
child bar
{"bar":"child bar"}
["foo","bar","quux","baz"]
["foo",null,"quux","baz"]
["foo","parent-bar","quux","baz"]
===*/

/* Property inheritance is quite interesting, test for correct behavior. */

function jsonStringifyFastPathInheritanceTest() {
    var obj1, obj2;

    /* For objects JSON.stringify() only looks at enumerable own properties so
     * it's very simple.
     */

    obj1 = { foo: 'parent foo' };
    obj2 = Object.create(obj1);
    obj2.bar = 'child bar';
    print(obj2.foo);
    print(obj2.bar);
    print(JSON.stringify(obj2));

    /* For arrays JSON.stringify() uses the [[Get]] operation which -does-
     * inherit through array gaps.  Ensure that works correctly.
     */
    obj1 = [ 'foo', 'bar', 'quux', 'baz' ];
    print(JSON.stringify(obj1));

    // create gap, nothing inherited; this is now supported in the fast path
    delete obj1[1];
    print(JSON.stringify(obj1));

    // inherit something through the gap; this now bails out of the fast path
    Array.prototype[1] = 'parent-bar';
    print(JSON.stringify(obj1));

    delete Array.prototype[1];  // restore sanity
}

try {
    print('inheritance test');
    jsonStringifyFastPathInheritanceTest();
} catch (e) {
    print(e.stack || e);
}

/*===
proxy test
["begin",{"foo":123},{"foo":345},{"foo":456},{"foo":567},"end"]
===*/

/* A Proxy object causes potential side effects and should cause abandonment
 * of the fast path.
 */

function jsonStringifyFastPathProxyTest() {
    var myValue;
    var target = { foo: 'bar' };

    // side effects chosen so that a restart will generate the same
    // result value sequence
    var p1 = new Proxy(target, {
        get: function() { myValue = 234; return 123; }
    });
    var p2 = new Proxy(target, {
        get: function() { var ret = myValue; myValue = 345; return ret; }
    });
    var p3 = new Proxy(target, {
        get: function() { var ret = myValue; myValue = 456; return ret; }
    });
    var p4 = new Proxy(target, {
        get: function() { var ret = myValue; myValue = 567; return ret; }
    });

    var obj = [ 'begin', p1, p2, p3, p4, 'end' ];

    myValue = 100;
    print(JSON.stringify(obj));
}

try {
    print('proxy test');
    jsonStringifyFastPathProxyTest();
} catch (e) {
    print(e.name);
}

/*===
jx/jc test
false
true
{"null":null,"true":true,"trueBoxed":true,"false":false,"falseBoxed":false,"number":123,"numberBoxed":123,"posZero":0,"negZero":0,"posInf":null,"negInf":null,"nan":null,"123mustquote":"must quote, non-identifier first char","mustquote\u0000":"must quote, NUL","mustquote<4660>":"must quote, non-ASCII","must_allow_unquoted123":"all chars ok","nonAsciiString":"nonascii: \u0000\u001e<127><4660><51966>","stringBoxed":"boxed string"}
{
    "null": null,
    "true": true,
    "trueBoxed": true,
    "false": false,
    "falseBoxed": false,
    "number": 123,
    "numberBoxed": 123,
    "posZero": 0,
    "negZero": 0,
    "posInf": null,
    "negInf": null,
    "nan": null,
    "123mustquote": "must quote, non-identifier first char",
    "mustquote\u0000": "must quote, NUL",
    "mustquote<4660>": "must quote, non-ASCII",
    "must_allow_unquoted123": "all chars ok",
    "nonAsciiString": "nonascii: \u0000\u001e<127><4660><51966>",
    "stringBoxed": "boxed string"
}
{
    "null": null,
    "true": true,
    "trueBoxed": true,
    "false": false,
    "falseBoxed": false,
    "number": 123,
    "numberBoxed": 123,
    "posZero": 0,
    "negZero": 0,
    "posInf": null,
    "negInf": null,
    "nan": null,
    "123mustquote": "must quote, non-identifier first char",
    "mustquote\u0000": "must quote, NUL",
    "mustquote<4660>": "must quote, non-ASCII",
    "must_allow_unquoted123": "all chars ok",
    "nonAsciiString": "nonascii: \u0000\u001e<127><4660><51966>",
    "stringBoxed": "boxed string"
}
true
{undefined:undefined,null:null,true:true,trueBoxed:true,false:false,falseBoxed:false,number:123,numberBoxed:123,posZero:0,negZero:-0,posInf:Infinity,negInf:-Infinity,nan:NaN,"123mustquote":"must quote, non-identifier first char","mustquote\x00":"must quote, NUL","mustquote\u1234":"must quote, non-ASCII",must_allow_unquoted123:"all chars ok",nonAsciiString:"nonascii: \x00\x1e\x7f\u1234\ucafe",stringBoxed:"boxed string",buffer:|deadbeef|,bufferBoxed:|deadbeef|,pointer:(<PTR>),pointerBoxed:(<PTR>),duktapeBuffer:|00000000000000|,nodejsBuffer:|4142434445464748494a4b4c|,nodejsBufferView:|44454647|,arrayBuffer:|6162636465666768|,dataView:|6162636465666768|,int8Array:|6162636465666768|,uint8Array:|6162636465666768|,uint8ArrayView:|62636465|,uint8ClampedArray:|6162636465666768|,int16Array:|6162636465666768|,int16ArrayView:|65666768|,uint16Array:|6162636465666768|,int32Array:|6162636465666768|,uint32Array:|6162636465666768|,float32Array:|6162636465666768|,float64Array:|6162636465666768|,function:{_func:true}}
{
    undefined: undefined,
    null: null,
    true: true,
    trueBoxed: true,
    false: false,
    falseBoxed: false,
    number: 123,
    numberBoxed: 123,
    posZero: 0,
    negZero: -0,
    posInf: Infinity,
    negInf: -Infinity,
    nan: NaN,
    "123mustquote": "must quote, non-identifier first char",
    "mustquote\x00": "must quote, NUL",
    "mustquote\u1234": "must quote, non-ASCII",
    must_allow_unquoted123: "all chars ok",
    nonAsciiString: "nonascii: \x00\x1e\x7f\u1234\ucafe",
    stringBoxed: "boxed string",
    buffer: |deadbeef|,
    bufferBoxed: |deadbeef|,
    pointer: (<PTR>),
    pointerBoxed: (<PTR>),
    duktapeBuffer: |00000000000000|,
    nodejsBuffer: |4142434445464748494a4b4c|,
    nodejsBufferView: |44454647|,
    arrayBuffer: |6162636465666768|,
    dataView: |6162636465666768|,
    int8Array: |6162636465666768|,
    uint8Array: |6162636465666768|,
    uint8ArrayView: |62636465|,
    uint8ClampedArray: |6162636465666768|,
    int16Array: |6162636465666768|,
    int16ArrayView: |65666768|,
    uint16Array: |6162636465666768|,
    int32Array: |6162636465666768|,
    uint32Array: |6162636465666768|,
    float32Array: |6162636465666768|,
    float64Array: |6162636465666768|,
    function: {_func:true}
}
{
    undefined: undefined,
    null: null,
    true: true,
    trueBoxed: true,
    false: false,
    falseBoxed: false,
    number: 123,
    numberBoxed: 123,
    posZero: 0,
    negZero: -0,
    posInf: Infinity,
    negInf: -Infinity,
    nan: NaN,
    "123mustquote": "must quote, non-identifier first char",
    "mustquote\x00": "must quote, NUL",
    "mustquote\u1234": "must quote, non-ASCII",
    must_allow_unquoted123: "all chars ok",
    nonAsciiString: "nonascii: \x00\x1e\x7f\u1234\ucafe",
    stringBoxed: "boxed string",
    buffer: |deadbeef|,
    bufferBoxed: |deadbeef|,
    pointer: (<PTR>),
    pointerBoxed: (<PTR>),
    duktapeBuffer: |00000000000000|,
    nodejsBuffer: |4142434445464748494a4b4c|,
    nodejsBufferView: |44454647|,
    arrayBuffer: |6162636465666768|,
    dataView: |6162636465666768|,
    int8Array: |6162636465666768|,
    uint8Array: |6162636465666768|,
    uint8ArrayView: |62636465|,
    uint8ClampedArray: |6162636465666768|,
    int16Array: |6162636465666768|,
    int16ArrayView: |65666768|,
    uint16Array: |6162636465666768|,
    int32Array: |6162636465666768|,
    uint32Array: |6162636465666768|,
    float32Array: |6162636465666768|,
    float64Array: |6162636465666768|,
    function: {_func:true}
}
true
{"undefined":{"_undef":true},"null":null,"true":true,"trueBoxed":true,"false":false,"falseBoxed":false,"number":123,"numberBoxed":123,"posZero":0,"negZero":-0,"posInf":{"_inf":true},"negInf":{"_ninf":true},"nan":{"_nan":true},"123mustquote":"must quote, non-identifier first char","mustquote\u0000":"must quote, NUL","mustquote\u1234":"must quote, non-ASCII","must_allow_unquoted123":"all chars ok","nonAsciiString":"nonascii: \u0000\u001e\u007f\u1234\ucafe","stringBoxed":"boxed string","buffer":{"_buf":"deadbeef"},"bufferBoxed":{"_buf":"deadbeef"},"pointer":{"_ptr":"<PTR>"},"pointerBoxed":{"_ptr":"<PTR>"},"duktapeBuffer":{"_buf":"00000000000000"},"nodejsBuffer":{"_buf":"4142434445464748494a4b4c"},"nodejsBufferView":{"_buf":"44454647"},"arrayBuffer":{"_buf":"6162636465666768"},"dataView":{"_buf":"6162636465666768"},"int8Array":{"_buf":"6162636465666768"},"uint8Array":{"_buf":"6162636465666768"},"uint8ArrayView":{"_buf":"62636465"},"uint8ClampedArray":{"_buf":"6162636465666768"},"int16Array":{"_buf":"6162636465666768"},"int16ArrayView":{"_buf":"65666768"},"uint16Array":{"_buf":"6162636465666768"},"int32Array":{"_buf":"6162636465666768"},"uint32Array":{"_buf":"6162636465666768"},"float32Array":{"_buf":"6162636465666768"},"float64Array":{"_buf":"6162636465666768"},"function":{"_func":true}}
{
    "undefined": {"_undef":true},
    "null": null,
    "true": true,
    "trueBoxed": true,
    "false": false,
    "falseBoxed": false,
    "number": 123,
    "numberBoxed": 123,
    "posZero": 0,
    "negZero": -0,
    "posInf": {"_inf":true},
    "negInf": {"_ninf":true},
    "nan": {"_nan":true},
    "123mustquote": "must quote, non-identifier first char",
    "mustquote\u0000": "must quote, NUL",
    "mustquote\u1234": "must quote, non-ASCII",
    "must_allow_unquoted123": "all chars ok",
    "nonAsciiString": "nonascii: \u0000\u001e\u007f\u1234\ucafe",
    "stringBoxed": "boxed string",
    "buffer": {"_buf":"deadbeef"},
    "bufferBoxed": {"_buf":"deadbeef"},
    "pointer": {"_ptr":"<PTR>"},
    "pointerBoxed": {"_ptr":"<PTR>"},
    "duktapeBuffer": {"_buf":"00000000000000"},
    "nodejsBuffer": {"_buf":"4142434445464748494a4b4c"},
    "nodejsBufferView": {"_buf":"44454647"},
    "arrayBuffer": {"_buf":"6162636465666768"},
    "dataView": {"_buf":"6162636465666768"},
    "int8Array": {"_buf":"6162636465666768"},
    "uint8Array": {"_buf":"6162636465666768"},
    "uint8ArrayView": {"_buf":"62636465"},
    "uint8ClampedArray": {"_buf":"6162636465666768"},
    "int16Array": {"_buf":"6162636465666768"},
    "int16ArrayView": {"_buf":"65666768"},
    "uint16Array": {"_buf":"6162636465666768"},
    "int32Array": {"_buf":"6162636465666768"},
    "uint32Array": {"_buf":"6162636465666768"},
    "float32Array": {"_buf":"6162636465666768"},
    "float64Array": {"_buf":"6162636465666768"},
    "function": {"_func":true}
}
{
    "undefined": {"_undef":true},
    "null": null,
    "true": true,
    "trueBoxed": true,
    "false": false,
    "falseBoxed": false,
    "number": 123,
    "numberBoxed": 123,
    "posZero": 0,
    "negZero": -0,
    "posInf": {"_inf":true},
    "negInf": {"_ninf":true},
    "nan": {"_nan":true},
    "123mustquote": "must quote, non-identifier first char",
    "mustquote\u0000": "must quote, NUL",
    "mustquote\u1234": "must quote, non-ASCII",
    "must_allow_unquoted123": "all chars ok",
    "nonAsciiString": "nonascii: \u0000\u001e\u007f\u1234\ucafe",
    "stringBoxed": "boxed string",
    "buffer": {"_buf":"deadbeef"},
    "bufferBoxed": {"_buf":"deadbeef"},
    "pointer": {"_ptr":"<PTR>"},
    "pointerBoxed": {"_ptr":"<PTR>"},
    "duktapeBuffer": {"_buf":"00000000000000"},
    "nodejsBuffer": {"_buf":"4142434445464748494a4b4c"},
    "nodejsBufferView": {"_buf":"44454647"},
    "arrayBuffer": {"_buf":"6162636465666768"},
    "dataView": {"_buf":"6162636465666768"},
    "int8Array": {"_buf":"6162636465666768"},
    "uint8Array": {"_buf":"6162636465666768"},
    "uint8ArrayView": {"_buf":"62636465"},
    "uint8ClampedArray": {"_buf":"6162636465666768"},
    "int16Array": {"_buf":"6162636465666768"},
    "int16ArrayView": {"_buf":"65666768"},
    "uint16Array": {"_buf":"6162636465666768"},
    "int32Array": {"_buf":"6162636465666768"},
    "uint32Array": {"_buf":"6162636465666768"},
    "float32Array": {"_buf":"6162636465666768"},
    "float64Array": {"_buf":"6162636465666768"},
    "function": {"_func":true}
}
===*/

/* Fast path for JX/JC. */

function jxJcFastPathTest() {
    var val;

    function id(k,v) { return v; }

    function cleanPrint(x) {
        x = x.replace(/[^\u0020-\u007e\u000a]/g, function (x) { return '<' + x.charCodeAt(0) + '>'; });
        x = x.replace(/0x[0-9a-fA-F]+/g, function (x) { return '<PTR>'; });
        print(x);
    }

    var arrayBuffer = new ArrayBuffer(8);
    arrayBuffer[0] = 0x61;
    arrayBuffer[1] = 0x62;
    arrayBuffer[2] = 0x63;
    arrayBuffer[3] = 0x64;
    arrayBuffer[4] = 0x65;
    arrayBuffer[5] = 0x66;
    arrayBuffer[6] = 0x67;
    arrayBuffer[7] = 0x68;

    // Remove Node.js buffer .toJSON() method here, because its presence
    // would cause a fastpath abort.
    delete Buffer.prototype.toJSON;
    print('toJSON' in Buffer.prototype);

    val = {
        undefined: void 0,
        null: null,
        true: true,
        trueBoxed: new Boolean(true),
        false: false,
        falseBoxed: new Boolean(false),
        number: 123,
        numberBoxed: new Number(123),
        posZero: +0,
        negZero: -0,
        posInf: 1/0,
        negInf: -1/0,
        nan: 0/0,
        '123mustquote': 'must quote, non-identifier first char',
        'mustquote\u0000': 'must quote, NUL',
        'mustquote\u1234': 'must quote, non-ASCII',
        'must_allow_unquoted123': 'all chars ok',
        nonAsciiString: 'nonascii: \u0000\u001e\u007f\u1234\ucafe',
        stringBoxed: new String('boxed string'),
        buffer: Duktape.dec('hex', 'deadbeef'),
        bufferBoxed: new Duktape.Buffer(Duktape.dec('hex', 'deadbeef')),
        pointer: Duktape.Pointer('dummy'),
        pointerBoxed: new Duktape.Pointer(Duktape.Pointer('dummy')),
        duktapeBuffer: new Duktape.Buffer(7),
        nodejsBuffer: new Buffer('ABCDEFGHIJKL'),
        nodejsBufferView: new Buffer('ABCDEFGHIJKL').slice(3, 7),
        arrayBuffer: arrayBuffer,
        dataView: new DataView(arrayBuffer),
        int8Array: new Int8Array(arrayBuffer),
        uint8Array: new Uint8Array(arrayBuffer),
        uint8ArrayView: new Uint8Array(arrayBuffer).subarray(1, 5),
        uint8ClampedArray: new Uint8ClampedArray(arrayBuffer),
        int16Array: new Int16Array(arrayBuffer),
        int16ArrayView: new Int16Array(arrayBuffer).subarray(2, 4),
        uint16Array: new Uint16Array(arrayBuffer),
        int32Array: new Int32Array(arrayBuffer),
        uint32Array: new Uint32Array(arrayBuffer),
        float32Array: new Float32Array(arrayBuffer),
        float64Array: new Float64Array(arrayBuffer),
        function: function test() {}
    };

    var json1 = JSON.stringify(val);
    var json2 = JSON.stringify(val, null, 4);
    var json3 = JSON.stringify(val, id, 4);  // replacer forces out of fast path

    print(json2 === json3);
    cleanPrint(json1);
    cleanPrint(json2);
    cleanPrint(json3);

    var jx1 = Duktape.enc('jx', val);
    var jx2 = Duktape.enc('jx', val, null, 4);
    var jx3 = Duktape.enc('jx', val, id, 4);

    print(jx2 === jx3);
    cleanPrint(jx1);
    cleanPrint(jx2);
    cleanPrint(jx3);

    var jc1 = Duktape.enc('jc', val);
    var jc2 = Duktape.enc('jc', val, null, 4);
    var jc3 = Duktape.enc('jc', val, id, 4);

    print(jc2 === jc3);
    cleanPrint(jc1);
    cleanPrint(jc2);
    cleanPrint(jc3);
}

try {
    print('jx/jc test');
    jxJcFastPathTest();
} catch (e) {
    print(e.stack || e);
}
