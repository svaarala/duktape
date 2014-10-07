var t;

/*===
undefined undefined
{}
{"foo":1}
[1,2,null,4]
===*/

try {
    // result is undefined
    t = JSON.stringify(undefined);
    print(typeof t, t);

    // undefined values are not serialized in objects
    print(JSON.stringify({bar:undefined}));  // empty case perhaps worth extra test
    print(JSON.stringify({foo:1,bar:undefined}));

    // undefined values are serialized as 'null' in arrays
    print(JSON.stringify([1,2,undefined,4]));
} catch (e) {
    print(e.name);
}

/*===
string null
{"foo":1,"bar":null}
[1,2,null,4]
===*/

try {
    t = JSON.stringify(null);
    print(typeof t, t);
    print(JSON.stringify({foo:1,bar:null}));
    print(JSON.stringify([1,2,null,4]));
} catch (e) {
    print(e.name);
}

/*===
string true
{"foo":1,"bar":true}
[1,2,true,4]
string false
{"foo":1,"bar":false}
[1,2,false,4]
===*/

try {
    t = JSON.stringify(true);
    print(typeof t, t);
    print(JSON.stringify({foo:1,bar:true}));
    print(JSON.stringify([1,2,true,4]));

    t = JSON.stringify(false);
    print(typeof t, t);
    print(JSON.stringify({foo:1,bar:false}));
    print(JSON.stringify([1,2,false,4]));
} catch (e) {
    print(e.name);
}

/*===
string -123
{"foo":1,"bar":-123}
[1,2,-123,4]
string 0
{"foo":1,"bar":0}
[1,2,0,4]
string 0
{"foo":1,"bar":0}
[1,2,0,4]
string 123
{"foo":1,"bar":123}
[1,2,123,4]
string null
{"foo":1,"bar":null}
[1,2,null,4]
string null
{"foo":1,"bar":null}
[1,2,null,4]
string null
{"foo":1,"bar":null}
[1,2,null,4]
===*/

/* Basic cases for numbers.  Special cases:
 *
 *   +0/-0       serializes as 0 (the sign of zero is always omitted)
 *   -Infinity   serializes a null
 *   Infinity    - "" -
 *   NaN         - "" -
 */

function numbersTest() {
    var numbers = [ -123, -0, +0, 123,
                    Number.NEGATIVE_INFINITY, Number.POSITIVE_INFINITY, Number.NaN ];
    var i;

    for (i = 0; i < numbers.length; i++) {
        x = numbers[i];

        t = JSON.stringify(x);
        print(typeof t, t);
        print(JSON.stringify({foo:1,bar:x}));
        print(JSON.stringify([1,2,x,4]));
    }
}

try {
    numbersTest();
} catch (e) {
    print(e.name);
}

/*===
===*/

/* XXX: Fraction handling for numbers. */

/*===
string "foo"
{"foo":1,"bar":"foo"}
[1,2,"foo",4]
string "foo \n\"bar"
{"foo":1,"bar":"foo \n\"bar"}
[1,2,"foo \n\"bar",4]
===*/

/* Basic cases for strings. */

function stringsTest() {
    var strings = [ "foo", "foo \n\"bar" ];
    var i;

    for (i = 0; i < strings.length; i++) {
        x = strings[i];

        t = JSON.stringify(x);
        print(typeof t, t);
        print(JSON.stringify({foo:1,bar:x}));
        print(JSON.stringify([1,2,x,4]));
    }
}

try {
    stringsTest();
} catch (e) {
    print(e.name);
}

/*===
"\""
"\\"
"/"
"\u0000"
"\u0001"
"\u0002"
"\u0003"
"\u0004"
"\u0005"
"\u0006"
"\u0007"
"\b"
"\t"
"\n"
"\u000b"
"\f"
"\r"
"\u000e"
"\u000f"
"\u0010"
"\u0011"
"\u0012"
"\u0013"
"\u0014"
"\u0015"
"\u0016"
"\u0017"
"\u0018"
"\u0019"
"\u001a"
"\u001b"
"\u001c"
"\u001d"
"\u001e"
"\u001f"
" "
34 128 34
34 129 34
34 130 34
34 131 34
34 132 34
34 133 34
34 134 34
34 135 34
34 136 34
34 137 34
34 138 34
34 139 34
34 140 34
34 141 34
34 142 34
34 143 34
34 144 34
34 145 34
34 146 34
34 147 34
34 148 34
34 149 34
34 150 34
34 151 34
34 152 34
34 153 34
34 154 34
34 155 34
34 156 34
34 157 34
34 158 34
34 159 34
===*/

/* Escape testing.
 *
 * Note that JSON.stringify() is required to NOT escape any control
 * characters which are < U+0020.  For instance, U+0080 is encoded
 * as the three characters '"', '\u0080' (unescaped), '"'.  We test
 * for these by looking at the code points.
 *
 * Also note that although '/' can be escaped in JSON, it MUST NOT
 * be escaped by JSON.stringify().
 */

function escapeTest() {
    var strings = [ '"', '\\', '/' ];
    var i;

    for (i = 0; i < strings.length; i++) {
        t = JSON.stringify(strings[i]);
        print(t);
    }

    for (i = 0; i <= 32; i++) {
        t = JSON.stringify(String.fromCharCode(i));
        print(t);
    }

    for (i = 0x80; i < 0xa0; i++) {
        t = JSON.stringify(String.fromCharCode(i));
        print(t.charCodeAt(0), t.charCodeAt(1), t.charCodeAt(2));
    }
}

try {
    escapeTest();
} catch (e) {
    print(e.name);
}

/*===
{"foo":1,"bar":"text","object":{"baz":2},"array":[1,2,3]}
===*/

/* Very basic object/array serialization test. */
try {
    t = JSON.stringify({foo:1,bar:'text',object:{baz:2},array:[1,2,3]});
    print(t);
} catch (e) {
    print(e.name);
}

/*===
[1,2,null,4]
[1,2,null,4]
[1,2,null,4]
===*/

/* Functions (anything callable) serialize as 'null' */

try {
    // anonymous Ecmascript function
    t = JSON.stringify([1,2,function(){},4]);
    print(t);

    // native function
    t = JSON.stringify([1,2,Object.prototype.toString,4]);
    print(t);

    // bound function
    t = JSON.stringify([1,2,print.bind('foo'),4]);
    print(t);
} catch (e) {
    print(e.name);
}
