/*===
basic reviver
reviver called
name: 
val: foo
holder: [object Object]
{"":"foo"}
===*/

/* Basic reviver test. */

print('basic reviver');

function basicReviverTest() {
    var v;

    function reviver(name, val) {
        var holder = this;
        print('reviver called');
        print('name:', name);
        print('val:', val);
        print('holder:', holder);
        print(JSON.stringify(holder));
    }

    v = JSON.parse('"foo"', reviver);
}

try {
    basicReviverTest();
} catch (e) {
    print(e.name);
}

/*===
complex reviver test
{"k_null":null, "k_true":true, "k_false":false, "k_num":123, "k_str":"str", "k_arr":[1,2], "k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_null", json(val)=null, json(holder)={"k_null":null,"k_true":true,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_true", json(val)=true, json(holder)={"k_null":null,"k_true":true,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_false", json(val)=false, json(holder)={"k_null":null,"k_true":true,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_num", json(val)=123, json(holder)={"k_null":null,"k_true":true,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_str", json(val)="str", json(holder)={"k_null":null,"k_true":true,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="0", json(val)=1, json(holder)=[1,2]
reviver: json(name)="1", json(val)=2, json(holder)=[1,2]
reviver: json(name)="k_arr", json(val)=[1,2], json(holder)={"k_null":null,"k_true":true,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="foo", json(val)=1, json(holder)={"foo":1,"bar":2}
reviver: json(name)="bar", json(val)=2, json(holder)={"foo":1,"bar":2}
reviver: json(name)="k_obj", json(val)={"foo":1,"bar":2}, json(holder)={"k_null":null,"k_true":true,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="", json(val)={"k_null":null,"k_true":true,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}, json(holder)={"":{"k_null":null,"k_true":true,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}}
{"k_null":null,"k_true":true,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
===*/

/* A more complex reviver test; no changes are made, just check the
 * call order and arguments.
 */

print('complex reviver test');

function complexReviverTest() {
    var txt;
    var v;

    function reviver(name, val) {
        var holder = this;
        print('reviver: json(name)=' + JSON.stringify(name) +
              ', json(val)=' + JSON.stringify(val) +
	      ', json(holder)=' + JSON.stringify(holder));
        return val;
    }

    txt = '{"k_null":null, "k_true":true, "k_false":false, ' +
          '"k_num":123, "k_str":"str", "k_arr":[1,2], ' +
          '"k_obj":{"foo":1,"bar":2}}';
    print(txt);

    v = JSON.parse(txt, reviver);
    print(JSON.stringify(v));
}

try {
    complexReviverTest();
} catch (e) {
    print(e.name, e);
}

/*===
reviver deletion test
{"k_null":null, "k_true":true, "k_false":false, "k_num":123, "k_str":"str", "k_arr":[1,2], "k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_null", json(val)=null, json(holder)={"k_null":null,"k_true":true,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_true", json(val)=true, json(holder)={"k_null":null,"k_true":true,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_false", json(val)=false, json(holder)={"k_null":null,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_num", json(val)=123, json(holder)={"k_null":null,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_str", json(val)="str", json(holder)={"k_null":null,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="0", json(val)=1, json(holder)=[1,2]
reviver: json(name)="1", json(val)=2, json(holder)=[1,2]
reviver: json(name)="k_arr", json(val)=[1,2], json(holder)={"k_null":null,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="foo", json(val)=1, json(holder)={"foo":1,"bar":2}
reviver: json(name)="bar", json(val)=2, json(holder)={"foo":1,"bar":2}
reviver: json(name)="k_obj", json(val)={"foo":1,"bar":2}, json(holder)={"k_null":null,"k_false":false,"k_num":123,"k_str":"str","k_obj":{"foo":1,"bar":2}}
reviver: json(name)="", json(val)={"k_null":null,"k_false":false,"k_num":123,"k_str":"str","k_obj":{"foo":1,"bar":2}}, json(holder)={"":{"k_null":null,"k_false":false,"k_num":123,"k_str":"str","k_obj":{"foo":1,"bar":2}}}
{"k_null":null,"k_false":false,"k_num":123,"k_str":"str","k_obj":{"foo":1,"bar":2}}
===*/

/* Reviver deletion test. */

print('reviver deletion test');

function reviverDeletionTest() {
    var txt;
    var v;

    function reviver(name, val) {
        var holder = this;

        print('reviver: json(name)=' + JSON.stringify(name) +
              ', json(val)=' + JSON.stringify(val) +
	      ', json(holder)=' + JSON.stringify(holder));

        // delete k_arr and k_true
        if (name === 'k_arr' || name === 'k_true') {
            return;
        }
        return val;
    }

    txt = '{"k_null":null, "k_true":true, "k_false":false, ' +
          '"k_num":123, "k_str":"str", "k_arr":[1,2], ' +
          '"k_obj":{"foo":1,"bar":2}}';
    print(txt);

    v = JSON.parse(txt, reviver);
    print(JSON.stringify(v));
}

try {
    reviverDeletionTest();
} catch (e) {
    print(e.name);
}

/*===
reviver replacement test
{"k_null":null, "k_true":true, "k_false":false, "k_num":123, "k_str":"str", "k_arr":[1,2], "k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_null", json(val)=null, json(holder)={"k_null":null,"k_true":true,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_true", json(val)=true, json(holder)={"k_null":null,"k_true":true,"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_false", json(val)=false, json(holder)={"k_null":null,"k_true":[3,2,1],"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_num", json(val)=123, json(holder)={"k_null":null,"k_true":[3,2,1],"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="k_str", json(val)="str", json(holder)={"k_null":null,"k_true":[3,2,1],"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="0", json(val)=1, json(holder)=[1,2]
reviver: json(name)="1", json(val)=2, json(holder)=[1,2]
reviver: json(name)="k_arr", json(val)=[1,2], json(holder)={"k_null":null,"k_true":[3,2,1],"k_false":false,"k_num":123,"k_str":"str","k_arr":[1,2],"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="foo", json(val)=1, json(holder)={"foo":1,"bar":2}
reviver: json(name)="bar", json(val)=2, json(holder)={"foo":1,"bar":2}
reviver: json(name)="k_obj", json(val)={"foo":1,"bar":2}, json(holder)={"k_null":null,"k_true":[3,2,1],"k_false":false,"k_num":123,"k_str":"str","k_arr":{"foo":"foo","bar":"bar"},"k_obj":{"foo":1,"bar":2}}
reviver: json(name)="", json(val)={"k_null":null,"k_true":[3,2,1],"k_false":false,"k_num":123,"k_str":"str","k_arr":{"foo":"foo","bar":"bar"},"k_obj":{"foo":1,"bar":2}}, json(holder)={"":{"k_null":null,"k_true":[3,2,1],"k_false":false,"k_num":123,"k_str":"str","k_arr":{"foo":"foo","bar":"bar"},"k_obj":{"foo":1,"bar":2}}}
{"k_null":null,"k_true":[3,2,1],"k_false":false,"k_num":123,"k_str":"str","k_arr":{"foo":"foo","bar":"bar"},"k_obj":{"foo":1,"bar":2}}
===*/

/* Reviver replacement test.
 *
 * In particular, test whether or not reviver will walk through the returned;
 * it should not.
 */

print('reviver replacement test');

function reviverReplacementTest() {
    var txt;
    var v;

    function reviver(name, val) {
        var holder = this;

        print('reviver: json(name)=' + JSON.stringify(name) +
              ', json(val)=' + JSON.stringify(val) +
	      ', json(holder)=' + JSON.stringify(holder));

        // replace k_true with [3,2,1] and k_arr with {foo:'foo',bar:'bar}
        if (name === 'k_arr') {
            return { foo: 'foo', bar: 'bar' };
        } else if (name === 'k_true') {
            return [ 3, 2, 1 ];
        } else {
            return val;
        }
    }

    txt = '{"k_null":null, "k_true":true, "k_false":false, ' +
          '"k_num":123, "k_str":"str", "k_arr":[1,2], ' +
          '"k_obj":{"foo":1,"bar":2}}';
    print(txt);

    v = JSON.parse(txt, reviver);
    print(JSON.stringify(v));
}

try {
    reviverReplacementTest();
} catch (e) {
    print(e.name);
}

/*===
reviver object identity
{"foo":1,"bar":2}
reviver: json(name)="foo", json(val)=1, json(holder)={"foo":1,"bar":2}
reviver: json(name)="bar", json(val)=2, json(holder)={"foo":{},"bar":2}
reviver: json(name)="", json(val)={"foo":{},"bar":{}}, json(holder)={"":{"foo":{},"bar":{}}}
{"foo":{},"bar":{}}
true
true
true
===*/

/* Check that replacement values are used as is.  In particular,
 * returning the same object retains object identity in the result.
 */

print('reviver object identity');

function reviverObjectIdentityTest() {
    var txt;
    var v;
    var repl_value = {};  // test this identity

    function reviver(name, val) {
        var holder = this;

        print('reviver: json(name)=' + JSON.stringify(name) +
              ', json(val)=' + JSON.stringify(val) +
	      ', json(holder)=' + JSON.stringify(holder));

        if (name === 'foo' || name === 'bar') {
            return repl_value;
        } else {
            return val;
        }
    }

    txt = '{"foo":1,"bar":2}';
    print(txt);

    v = JSON.parse(txt, reviver);
    print(JSON.stringify(v));

    print(v.foo === repl_value);
    print(v.bar === repl_value);
    print(v.foo === v.bar);
}

try {
    reviverObjectIdentityTest();
} catch (e) {
    print(e.name);
}

/*===
reviver name type test
{"foo":1,"bar":[2,3]}
object string number
object string number
object string number
object string object
object string object
{"foo":1,"bar":[2,3]}
===*/

/* Reviver 'name' argument is a string key of an object, or a string
 * coercion (ToString(arr_index)) of an array index.
 *
 * (Rhino uses a numeric array index key.)
 */

print('reviver name type test');

function reviverNameTypeTest() {
    function reviver(name, val) {
        print(typeof this, typeof name, typeof val);
        return val;
    }

    txt = '{"foo":1,"bar":[2,3]}';
    print(txt);

    v = JSON.parse(txt, reviver);
    print(JSON.stringify(v));
}

try {
    reviverNameTypeTest();
} catch (e) {
    print(e.name);
}

/*===
invalid revivers
{"foo":1,"bar":2}
{"foo":1,"bar":2}
{"foo":1,"bar":2}
{"foo":1,"bar":2}
{"foo":1,"bar":2}
{"foo":1,"bar":2}
{"foo":1,"bar":2}
{"foo":1,"bar":2}
{"foo":1,"bar":2}
===*/

/* Reviver must callable; other values are ignored silently. */

print('invalid revivers');

function testInvalidRevivers() {
    var v;

    // base case
    v = JSON.parse('{"foo":1,"bar":2}');
    print(JSON.stringify(v));

    v = JSON.parse('{"foo":1,"bar":2}', undefined);
    print(JSON.stringify(v));
    v = JSON.parse('{"foo":1,"bar":2}', null);
    print(JSON.stringify(v));
    v = JSON.parse('{"foo":1,"bar":2}', true);
    print(JSON.stringify(v));
    v = JSON.parse('{"foo":1,"bar":2}', false);
    print(JSON.stringify(v));
    v = JSON.parse('{"foo":1,"bar":2}', 1.23);
    print(JSON.stringify(v));
    v = JSON.parse('{"foo":1,"bar":2}', 'foo');
    print(JSON.stringify(v));
    v = JSON.parse('{"foo":1,"bar":2}', []);
    print(JSON.stringify(v));
    v = JSON.parse('{"foo":1,"bar":2}', {});
    print(JSON.stringify(v));
}

try {
    testInvalidRevivers();
} catch (e) {
    print(e.name);
}
