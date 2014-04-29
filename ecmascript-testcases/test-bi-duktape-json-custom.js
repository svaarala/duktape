/*
 *  Test cases for extended JSON formats JSONX and JSONC
 *  (DUK_USE_JSONX, DUK_USE_JSONC).
 */

/*---
{
    "custom": true
}
---*/

function encJsonx(val) {
    return Duktape.enc('jsonx', val);
}

function decJsonx(val) {
    return Duktape.dec('jsonx', val);
}

function safedecJsonx(val) {
    try {
        return decJsonx(val);
    } catch (e) {
        return e.name;
    }
}

function encJsonc(val) {
    return Duktape.enc('jsonc', val);
}

function decJsonc(val) {
    return Duktape.dec('jsonc', val);
}

function safedecJsonc(val) {
    try {
        return decJsonc(val);
    } catch (e) {
        return e.name;
    }
}

/*===
primitive type encode
undefined undefined {"_undef":true}
object null null
boolean true true
boolean false false
number 123 123
number -0 -0
number 0 0
number NaN {"_nan":true}
number -Infinity {"_ninf":true}
number Infinity {"_inf":true}
string "foo" "foo"
object [1,2,3] [1,2,3]
object {test_key:123,"foo bar":321} {"test_key":123,"foo bar":321}
buffer |deadbeef| {"_buf":"deadbeef"}
object |deadbeef| {"_buf":"deadbeef"}
pointer (null) {"_ptr":"null"}
object (null) {"_ptr":"null"}
function {_func:true} {"_func":true}
pointer
jsonx non-null ptr true
jsonc non-null ptr true
object
jsonx non-null ptr true
jsonc non-null ptr true
primitive type decode
jsonx
undefined undefined
object null
boolean true
boolean false
number 123
number NaN
number -Infinity
number Infinity
string foo
object [1,2,3]
object {"test_key":"bar","foo bar":"quux"}
buffer deadbeef
pointer nonnull
pointer null
object functable
jsonc
object {"_undef":true}
object null
boolean true
boolean false
number 123
object {"_nan":true}
object {"_ninf":true}
object {"_inf":true}
string foo
object [1,2,3]
object {"test_key":"bar","foo bar":"quux"}
object {"_buf":"deadbeef"}
object {"_ptr":"(0x12345678)"}
object {"_ptr":"(null)"}
object functable
===*/

function typeEncodeTest() {
    var tmp;
    var ptr;
    var m_ptr_nonnull_jsonx = /^\([0-9a-fA-Fx:]+\)$/;  // e.g. (0x1345890)
    var m_ptr_nonnull_jsonc = /^\{\"_ptr\":\"[0-9a-fA-Fx:]+\"\}$/;  // e.g. {"_ptr":"0x1345890"}
    var func = function() { print('hello world'); };

    var values = [
        undefined, null, true, false, 123.0, -0, +0, 0 / 0, -1 / 0, 1 / 0,
        'foo', [1,2,3], { test_key: 123, 'foo bar': 321 },
        Duktape.dec('hex', 'deadbeef'),                      // plain buf
        new Duktape.Buffer(Duktape.dec('hex', 'deadbeef')),  // object buf
        Duktape.Pointer(),                                   // plain ptr; null pointer has a deterministic representation
        new Duktape.Pointer(),                               // object ptr
        function myfunc() {}
    ];

    // Test all primitive types except pointer, whose representation
    // is platform specific, and function, whose representation may
    // vary from version to version.
    values.forEach(function(v) {
        print(typeof v, encJsonx(v), encJsonc(v));
    });

    // pointer: match against a pattern since the contents are otherwise
    // platform dependent

    ptr = Duktape.Pointer('dummy');  // non-null plain ptr
    print(typeof ptr);
    tmp = encJsonx(ptr);
    print('jsonx non-null ptr', m_ptr_nonnull_jsonx.test(tmp));
    tmp = encJsonc(ptr);
    print('jsonc non-null ptr', m_ptr_nonnull_jsonc.test(tmp));

    ptr = new Duktape.Pointer('dummy');  // non-null object ptr
    print(typeof ptr);
    tmp = encJsonx(ptr);
    print('jsonx non-null ptr', m_ptr_nonnull_jsonx.test(tmp));
    tmp = encJsonc(ptr);
    print('jsonc non-null ptr', m_ptr_nonnull_jsonc.test(tmp));
}

function typeDecodeTest(dec) {
    function dval(t) {
        if (typeof t == 'buffer') {
            print(typeof t, Duktape.enc('hex', t));
        } else if (typeof t == 'pointer') {
            if (t === Duktape.Pointer()) {
                print(typeof t, 'null');
            } else {
                print(typeof t, 'nonnull');
            }
        } else if (typeof t == 'object' && t !== null && t._func) {
            print(typeof t, 'functable');  // function always decodes back as a table
        } else if (typeof t == 'object' && t !== null) {
            // although pointer values encode to platform dependent JSONC objects,
            // they decode back as plain JSON objects without any interpretation,
            // so we can print them also here.
            print(typeof t, JSON.stringify(t));
        } else {
            print(typeof t, String(t));
        }
    }

    var jsonxValues = [
        'undefined',
        'null',
        'true',
        'false',
        '123',
        'NaN',
        '-Infinity',
        'Infinity',
        '"foo"',
        '[1,2,3]',
        '{test_key:"bar","foo bar":"quux"}',
        '|deadbeef|',
        encJsonx(Duktape.Pointer('dummy')),  // pointer format is platform specific so use a pointer generated
                                             // by Duktape; this is obviously not the best idea for testing
        '(null)',
        '{_func:true}'
    ];

    var jsoncValues = [
        '{"_undef":true}',
        'null',
        'true',
        'false',
        '123',
        '{"_nan":true}',
        '{"_ninf":true}',
        '{"_inf":true}',
        '"foo"',
        '[1,2,3]',
        '{"test_key":"bar","foo bar":"quux"}',
        '{"_buf":"deadbeef"}',
        '{"_ptr":"(0x12345678)"}',  // because this decodes into a normal object without
                                    // pointer parsing, a specific value can be used here
        '{"_ptr":"(null)"}',
        '{"_func":true}'
    ];

    print('jsonx');
    jsonxValues.forEach(function(v) {
        try {
            dval(decJsonx(v));
        } catch (e) {
            print(e);
        }
    });

    print('jsonc');
    jsoncValues.forEach(function(v) {
        try {
            dval(decJsonc(v));
        } catch (e) {
            print(e);
        }
    });
}

print('primitive type encode');
try {
    typeEncodeTest();
} catch (e) {
    print(e);
}

print('primitive type decode');
try {
    typeDecodeTest();
} catch (e) {
    print(e);
}

/*===
signed zero
0
0
-0
0
-0
0
number 0 neg
number 0 pos
number 0 neg
number 0 pos
number 0 neg
number 0 pos
===*/

/* Although JSON syntax allows negative zero, the standard algorithm serializes
 * negative zero as "0".  Curiously, the standard algorithm still parses a '-0'
 * back as a negative zero.
 *
 * JSONX/JSONC serialize a negative zero as '-0' and should parse it back also
 * without losing the sign.
 */

function signedZeroTest() {
    function prVal(v) {
        print(typeof v, v, (1 / v > 0 ? 'pos' : 'neg'));
    }

    print(JSON.stringify(-0));
    print(JSON.stringify(+0));
    print(Duktape.enc('jsonx', -0));
    print(Duktape.enc('jsonx', +0));
    print(Duktape.enc('jsonc', -0));
    print(Duktape.enc('jsonc', +0));

    prVal(JSON.parse('-0'));
    prVal(JSON.parse('0'));
    prVal(Duktape.dec('jsonx', '-0'));
    prVal(Duktape.dec('jsonx', '0'));
    prVal(Duktape.dec('jsonc', '-0'));
    prVal(Duktape.dec('jsonc', '0'));
}

print('signed zero');

try {
    signedZeroTest();
} catch (e) {
    print(e);
}

/*===
character escape encode
"foo" "foo"
"\xfc" "\u00fc"
"\uabcd" "\uabcd"
"\U0010fedc" "U+0010fedc"
"\Udeadbeef" "U+deadbeef"
character escape decode
jsonx 0a0a0a0a0a0a0a
SyntaxError
jsonx c3bec3bec3bec3bec3bec3be
SyntaxError
jsonx e18aafe18aafe18aafe18aaf
SyntaxError
jsonx fe839eab9bbbaffe839eab9bbbaf
SyntaxError
===*/

/* In addition to the standard '\uNNNN' format, the extended format supports
 * '\xNN' and '\UNNNNNNNN'.  The encoding is shortest possible while any
 * format is accepted for decoding.
 */

function characterEscapeEncodeTest() {
    function mk(hex) {
        return String(Duktape.dec('hex', hex));
    }

    var values = [
        '666f6f',         // foo
        'c3bc',           // U+00FC
        'eaaf8d',         // U+ABCD
        'f48fbb9c',       // U+0010FEDC
        'fe839eab9bbbaf'  // U+DEADBEEF
    ];

    values.forEach(function (v) {
        var t = mk(v);
        print(encJsonx(t), encJsonc(t));
    });
}

function characterEscapeDecodeTest() {
    function dump(val) {
        return String(Duktape.enc('hex', val));
    }

    var values = [
        '"\\n\\x0a\\x0A\\u000a\\u000A\\U0000000a\\U0000000A"',
        '"\\xfe\\xFE\\u00fe\\u00FE\\U000000fe\\U000000FE"',
        '"\\u12aF\\u12Af\\U000012aF\\U000012Af"',
        '"\\UDeAdBeEf\\UdEaDbEeF"'
    ];

    // only makes sense for JSONX, but check the JSONC SyntaxError also

    values.forEach(function (v) {
        try {
            print('jsonx', dump(decJsonx(v)));
        } catch (e) {
            print(e.name);
        }
        try {
            print('jsonc', dump(decJsonc(v)));
        } catch (e) {
            print(e.name);
        }

    });
}

print('character escape encode');
try {
    characterEscapeEncodeTest();
} catch (e) {
    print(e);
}

print('character escape decode');
try {
    characterEscapeDecodeTest();
} catch (e) {
    print(e);
}

/*===
ascii only output
jsonx "\x00\x01\x02\x03\x04\x05\x06\x07\b\t\n\x0b\f\r\x0e\x0f"
jsonc "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\n\u000b\f\r\u000e\u000f"
json  "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\n\u000b\f\r\u000e\u000f"
jsonx "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
jsonc "\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017\u0018\u0019\u001a\u001b\u001c\u001d\u001e\u001f"
json  "\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017\u0018\u0019\u001a\u001b\u001c\u001d\u001e\u001f"
jsonx " !\"#$%&'()*+,-./"
jsonc " !\"#$%&'()*+,-./"
json  " !\"#$%&'()*+,-./"
jsonx "0123456789:;<=>?"
jsonc "0123456789:;<=>?"
json  "0123456789:;<=>?"
jsonx "@ABCDEFGHIJKLMNO"
jsonc "@ABCDEFGHIJKLMNO"
json  "@ABCDEFGHIJKLMNO"
jsonx "PQRSTUVWXYZ[\\]^_"
jsonc "PQRSTUVWXYZ[\\]^_"
json  "PQRSTUVWXYZ[\\]^_"
jsonx "`abcdefghijklmno"
jsonc "`abcdefghijklmno"
json  "`abcdefghijklmno"
jsonx "pqrstuvwxyz{|}~\x7f"
jsonc "pqrstuvwxyz{|}~\u007f"
jsonx "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f"
jsonc "\u0080\u0081\u0082\u0083\u0084\u0085\u0086\u0087\u0088\u0089\u008a\u008b\u008c\u008d\u008e\u008f"
jsonx "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f"
jsonc "\u0090\u0091\u0092\u0093\u0094\u0095\u0096\u0097\u0098\u0099\u009a\u009b\u009c\u009d\u009e\u009f"
jsonx "\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf"
jsonc "\u00a0\u00a1\u00a2\u00a3\u00a4\u00a5\u00a6\u00a7\u00a8\u00a9\u00aa\u00ab\u00ac\u00ad\u00ae\u00af"
jsonx "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf"
jsonc "\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba\u00bb\u00bc\u00bd\u00be\u00bf"
jsonx "\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf"
jsonc "\u00c0\u00c1\u00c2\u00c3\u00c4\u00c5\u00c6\u00c7\u00c8\u00c9\u00ca\u00cb\u00cc\u00cd\u00ce\u00cf"
jsonx "\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
jsonc "\u00d0\u00d1\u00d2\u00d3\u00d4\u00d5\u00d6\u00d7\u00d8\u00d9\u00da\u00db\u00dc\u00dd\u00de\u00df"
jsonx "\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef"
jsonc "\u00e0\u00e1\u00e2\u00e3\u00e4\u00e5\u00e6\u00e7\u00e8\u00e9\u00ea\u00eb\u00ec\u00ed\u00ee\u00ef"
jsonx "\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff"
jsonc "\u00f0\u00f1\u00f2\u00f3\u00f4\u00f5\u00f6\u00f7\u00f8\u00f9\u00fa\u00fb\u00fc\u00fd\u00fe\u00ff"
jsonx "\u0100\u0101\u0102\u0103\u0104\u0105\u0106\u0107\u0108\u0109\u010a\u010b\u010c\u010d\u010e\u010f"
jsonc "\u0100\u0101\u0102\u0103\u0104\u0105\u0106\u0107\u0108\u0109\u010a\u010b\u010c\u010d\u010e\u010f"
jsonx "\u0110\u0111\u0112\u0113\u0114\u0115\u0116\u0117\u0118\u0119\u011a\u011b\u011c\u011d\u011e\u011f"
jsonc "\u0110\u0111\u0112\u0113\u0114\u0115\u0116\u0117\u0118\u0119\u011a\u011b\u011c\u011d\u011e\u011f"
jsonx "\u0120\u0121\u0122\u0123\u0124\u0125\u0126\u0127\u0128\u0129\u012a\u012b\u012c\u012d\u012e\u012f"
jsonc "\u0120\u0121\u0122\u0123\u0124\u0125\u0126\u0127\u0128\u0129\u012a\u012b\u012c\u012d\u012e\u012f"
jsonx "\u0130\u0131\u0132\u0133\u0134\u0135\u0136\u0137\u0138\u0139\u013a\u013b\u013c\u013d\u013e\u013f"
jsonc "\u0130\u0131\u0132\u0133\u0134\u0135\u0136\u0137\u0138\u0139\u013a\u013b\u013c\u013d\u013e\u013f"
jsonx "\u0140\u0141\u0142\u0143\u0144\u0145\u0146\u0147\u0148\u0149\u014a\u014b\u014c\u014d\u014e\u014f"
jsonc "\u0140\u0141\u0142\u0143\u0144\u0145\u0146\u0147\u0148\u0149\u014a\u014b\u014c\u014d\u014e\u014f"
jsonx "\u0150\u0151\u0152\u0153\u0154\u0155\u0156\u0157\u0158\u0159\u015a\u015b\u015c\u015d\u015e\u015f"
jsonc "\u0150\u0151\u0152\u0153\u0154\u0155\u0156\u0157\u0158\u0159\u015a\u015b\u015c\u015d\u015e\u015f"
jsonx "\u0160\u0161\u0162\u0163\u0164\u0165\u0166\u0167\u0168\u0169\u016a\u016b\u016c\u016d\u016e\u016f"
jsonc "\u0160\u0161\u0162\u0163\u0164\u0165\u0166\u0167\u0168\u0169\u016a\u016b\u016c\u016d\u016e\u016f"
jsonx "\u0170\u0171\u0172\u0173\u0174\u0175\u0176\u0177\u0178\u0179\u017a\u017b\u017c\u017d\u017e\u017f"
jsonc "\u0170\u0171\u0172\u0173\u0174\u0175\u0176\u0177\u0178\u0179\u017a\u017b\u017c\u017d\u017e\u017f"
jsonx "\u0180\u0181\u0182\u0183\u0184\u0185\u0186\u0187\u0188\u0189\u018a\u018b\u018c\u018d\u018e\u018f"
jsonc "\u0180\u0181\u0182\u0183\u0184\u0185\u0186\u0187\u0188\u0189\u018a\u018b\u018c\u018d\u018e\u018f"
jsonx "\u0190\u0191\u0192\u0193\u0194\u0195\u0196\u0197\u0198\u0199\u019a\u019b\u019c\u019d\u019e\u019f"
jsonc "\u0190\u0191\u0192\u0193\u0194\u0195\u0196\u0197\u0198\u0199\u019a\u019b\u019c\u019d\u019e\u019f"
jsonx "\u01a0\u01a1\u01a2\u01a3\u01a4\u01a5\u01a6\u01a7\u01a8\u01a9\u01aa\u01ab\u01ac\u01ad\u01ae\u01af"
jsonc "\u01a0\u01a1\u01a2\u01a3\u01a4\u01a5\u01a6\u01a7\u01a8\u01a9\u01aa\u01ab\u01ac\u01ad\u01ae\u01af"
jsonx "\u01b0\u01b1\u01b2\u01b3\u01b4\u01b5\u01b6\u01b7\u01b8\u01b9\u01ba\u01bb\u01bc\u01bd\u01be\u01bf"
jsonc "\u01b0\u01b1\u01b2\u01b3\u01b4\u01b5\u01b6\u01b7\u01b8\u01b9\u01ba\u01bb\u01bc\u01bd\u01be\u01bf"
jsonx "\u01c0\u01c1\u01c2\u01c3\u01c4\u01c5\u01c6\u01c7\u01c8\u01c9\u01ca\u01cb\u01cc\u01cd\u01ce\u01cf"
jsonc "\u01c0\u01c1\u01c2\u01c3\u01c4\u01c5\u01c6\u01c7\u01c8\u01c9\u01ca\u01cb\u01cc\u01cd\u01ce\u01cf"
jsonx "\u01d0\u01d1\u01d2\u01d3\u01d4\u01d5\u01d6\u01d7\u01d8\u01d9\u01da\u01db\u01dc\u01dd\u01de\u01df"
jsonc "\u01d0\u01d1\u01d2\u01d3\u01d4\u01d5\u01d6\u01d7\u01d8\u01d9\u01da\u01db\u01dc\u01dd\u01de\u01df"
jsonx "\u01e0\u01e1\u01e2\u01e3\u01e4\u01e5\u01e6\u01e7\u01e8\u01e9\u01ea\u01eb\u01ec\u01ed\u01ee\u01ef"
jsonc "\u01e0\u01e1\u01e2\u01e3\u01e4\u01e5\u01e6\u01e7\u01e8\u01e9\u01ea\u01eb\u01ec\u01ed\u01ee\u01ef"
jsonx "\u01f0\u01f1\u01f2\u01f3\u01f4\u01f5\u01f6\u01f7\u01f8\u01f9\u01fa\u01fb\u01fc\u01fd\u01fe\u01ff"
jsonc "\u01f0\u01f1\u01f2\u01f3\u01f4\u01f5\u01f6\u01f7\u01f8\u01f9\u01fa\u01fb\u01fc\u01fd\u01fe\u01ff"
jsonx "\u0567"
jsonc "\u0567"
jsonx "\u1234"
jsonc "\u1234"
jsonx "\uffff"
jsonc "\uffff"
===*/

function asciiOnlyTest() {
    var i, j;
    var tmp;

    // go through first 512 codepoints exhaustively
    for (i = 0; i < 32; i++) {
        tmp = '';
        for (j = 0; j < 16; j++) {
            tmp += String.fromCharCode(i*16 + j);
        }
        print('jsonx', encJsonx(tmp));
        print('jsonc', encJsonc(tmp));
        if (i < 7) {  // ASCII range, does not contain 0x7f (which standard JSON doesn't escape)
            print('json ', JSON.stringify(tmp));
        }
    }

    // then sample a few codepoints above (these are already tested to
    // some extent by the character escape test case above)
    [ 0x0567, 0x1234, 0xffff ].forEach(function (cp) {
        var t = String.fromCharCode(cp);
        print('jsonx', encJsonx(t));
        print('jsonc', encJsonc(t));
    });
}

print('ascii only output');
try {
    asciiOnlyTest();
} catch (e) {
    print(e);
}

/*===
avoid key quotes
jsonx {$:true,_:true,a:true,z:true,A:true,Z:true,"0":false,"9":false,"":false}
jsonc {"$":true,"_":true,"a":true,"z":true,"A":true,"Z":true,"0":false,"9":false,"":false}
jsonx {$$:true,__:true,a$:true,z_:true,A0:true,Z9:true,"0$":false,"1_":false,"2a":false,"3z":false,"4A":false,"5A":false,"60":false,"79":false}
jsonc {"$$":true,"__":true,"a$":true,"z_":true,"A0":true,"Z9":true,"0$":false,"1_":false,"2a":false,"3z":false,"4A":false,"5A":false,"60":false,"79":false}
jsonx {test:true,test_key:true,_test_key:true,$test_key:true}
jsonc {"test":true,"test_key":true,"_test_key":true,"$test_key":true}
jsonx {"%foo":false,"foo%":false,"foo-bar":false,"foo bar":false}
jsonc {"%foo":false,"foo%":false,"foo-bar":false,"foo bar":false}
1 2 3 4 5 6
SyntaxError
1 2 3 4
SyntaxError
jsonx SyntaxError
jsonc SyntaxError
jsonx SyntaxError
jsonc SyntaxError
jsonx SyntaxError
jsonc SyntaxError
jsonx SyntaxError
jsonc SyntaxError
===*/

function avoidKeyQuotesTest() {
    var t, inp;

    var obj1 = {
        $: true,
        _: true,
        a: true,
        z: true,
        A: true,
        Z: true,
        '0': false,
        '9': false,
        '': false   // empty is specifically not accepted, although it would be unambiguous
    };
    var obj2 = {
        $$: true,
        __: true,
        a$: true,
        z_: true,
        A0: true,
        Z9: true,
        '0$': false,
        '1_': false,
        '2a': false,
        '3z': false,
        '4A': false,
        '5A': false,
        '60': false,
        '79': false
    };
    var obj3 = {
        test: true,
        test_key: true,
        _test_key: true,
        $test_key: true,
    };
    var obj4 = {
        '%foo': false,
        'foo%': false,
        'foo-bar': false,
        'foo bar': false
    };

    // encode tests

    [ obj1, obj2, obj3, obj4 ].forEach(function (v) {
        print('jsonx', encJsonx(v));
        print('jsonc', encJsonc(v));
    });

    // decode tests, only make sense for JSONX but JSONC is also
    // tried to display a SyntaxError.

    inp = '{ $: 1, _: 2, a: 3, z: 4, A: 5, Z: 6 }';
    t = safedecJsonx(inp);
    print(t['$'], t['_'], t['a'], t['z'], t['A'], t['Z']);
    print(safedecJsonc(inp));

    inp = '{ test: 1, test_key: 2, _test_key: 3, $test_key: 4 }';
    t = safedecJsonx(inp);
    print(t['test'], t['test_key'], t['_test_key'], t['$test_key']);
    print(safedecJsonc(inp));

    [ '{ %foo: 1 }',
      '{ foo%: 1 }',
      '{ foo-bar: 1 }',
      '{ foo bar: 1 }' ].forEach(function (v) {
        print('jsonx', safedecJsonx(v));
        print('jsonc', safedecJsonc(v));
    });
}

print('avoid key quotes');
try {
    avoidKeyQuotesTest();
} catch (e) {
    print(e);
}

/*===
non-default encoding options
{
    foo: "BAR",
    bar: |deadbeef|,
    quux: [
        123,
        NaN,
        Infinity,
        -Infinity
    ],
    baz: {_func:true}
}
{
->foo: "bar",
->bar: |deadbeef|,
->quux: [
->->123,
->->NaN,
->->Infinity,
->->-Infinity
->]
}
{
    "foo": "BAR",
    "bar": {"_buf":"deadbeef"},
    "quux": [
        123,
        {"_nan":true},
        {"_inf":true},
        {"_ninf":true}
    ],
    "baz": {"_func":true}
}
{
->"foo": "bar",
->"bar": {"_buf":"deadbeef"},
->"quux": [
->->123,
->->{"_nan":true},
->->{"_inf":true},
->->{"_ninf":true}
->]
}
===*/

/* Test that non-default encoding options still work with the custom
 * formats.
 */

function nonDefaultEncodingTest() {
    var val = {
        foo: 'bar',
        bar: Duktape.dec('hex', 'deadbeef'),
        quux: [
            123,
            0 / 0,
            1 / 0,
            -1 / 0
        ],
        baz: function() {}
    };

    function ucStrings(k,v) {
        if (typeof v === 'string') { return v.toUpperCase() } else { return v }
    }

    print(Duktape.enc('jsonx', val, ucStrings /*replacer*/, 4 /*space*/));
    print(Duktape.enc('jsonx', val, [ 'foo', 'bar', 'quux' ] /*replacer*/, '->' /*space*/));  // drop 'baz', use weird space

    print(Duktape.enc('jsonc', val, ucStrings /*replacer*/, 4 /*space*/));
    print(Duktape.enc('jsonc', val, [ 'foo', 'bar', 'quux' ] /*replacer*/, '->' /*space*/));  // drop 'baz', use weird space
}

print('non-default encoding options');
try {
    nonDefaultEncodingTest();
} catch (e) {
    print(e);
}

/*===
non-default decoding options
foo,bar,quux
BAR
foo,bar,quux
BAR
===*/

/* Test that non-default decoding options still work with the custom
 * formats.
 */

function nonDefaultDecodingTest() {
    var val = {
        foo: 'bar',
        bar: Duktape.dec('hex', 'deadbeef'),
        quux: [
            123,
            0 / 0,
            1 / 0,
            -1 / 0
        ],
        baz: function() {}
    };
    var enc, dec;

    function revive(k,v) {
        if (k === 'baz') {
            return undefined;  // delete 'baz'
        }
        if (typeof v === 'string') {
            return v.toUpperCase();  // uppercase strings
        }
        return v;
    }

    enc = encJsonx(val);
    dec = Duktape.dec('jsonx', enc, revive);
    print(Object.getOwnPropertyNames(dec));
    print(dec.foo);

    enc = encJsonc(val);
    dec = Duktape.dec('jsonc', enc, revive);
    print(Object.getOwnPropertyNames(dec));
    print(dec.foo);
}

print('non-default decoding options');
try {
    nonDefaultDecodingTest();
} catch (e) {
    print(e);
}

/*===
invalid xutf-8
e188
json  22c3a1c28822
jsonx "\xe1\x88"
jsonc "\u00e1\u0088"
ff41
json  22c3bf4122
jsonx "\xffA"
jsonc "\u00ffA"
c080
json  220022
jsonx "\x00"
jsonc "\u0000"
===*/

/* Test invalid XUTF-8 handling in all modes, including standard JSON.  This
 * behavior is always outside the scope of Ecmascript because all valid
 * Ecmascript strings are valid CESU-8.
 *
 * Because the XUTF-8 decoding is now lenient (it does not, for instance,
 * check continuation bytes at all), this test is now focused on testing
 * invalid initial characters or end-of-buffer conditions.
 */

function invalidXutf8Test() {
    var values = [
        'e188',     // last byte missing from U+1234 encoding (e188b4)
        'ff41',     // first byte is an invalid initial byte
        'c080',     // non-shortest encoding for U+0000
    ];

    // Because standard JSON does not escape non-ASCII codepoints, hex
    // encode its output
    values.forEach(function (v) {
        var t = String(Duktape.dec('hex', v));
        print(v);
        print('json ', Duktape.enc('hex', JSON.stringify(t)));
        print('jsonx', encJsonx(t));
        print('jsonc', encJsonc(t));
    });
}

print('invalid xutf-8');
try {
    invalidXutf8Test();
} catch (e) {
    print(e);
}

