/*===
{"foo":1,"bar":"bar/toJSON"}
{"foo":1,"bar":"bar/toJSON","quux":{"toJSON":123},"quuux":{"toJSON":{"foo":"not callable"}},"quuuux":{"toJSON":["not callable"]}}
Error quuux/toJSON error
{"foo":1,"bar":"bar/toJSON","quux":{"toJSON":123},"quuux":"2012-01-02T03:04:05.006Z"}
{"foo":1,"bar":"bar/toJSON","quux":{"toJSON":123},"quuux":"inherited toJSON"}
===*/

/* Any Object values with a callable toJSON() property will get called,
 * and the return value of toJSON() replaces the value to be serialized.
 *
 * If toJSON property exists but is not callable, it is ignored.
 * If toJSON() throws an error, serialization stops with the error.
 */

function toJsonPropertyTest1() {
    var obj = {
        foo: 1,
        bar: { toJSON: function() { return 'bar/toJSON'; } }
    };

    // toJSON is called
    print(JSON.stringify(obj));
}

function toJsonPropertyTest2() {
    var obj = {
        foo: 1,
        bar: { toJSON: function() { return 'bar/toJSON'; } },
        quux: { toJSON: 123 },
        quuux: { toJSON: { foo: 'not callable' } },
        quuuux: { toJSON: [ 'not callable' ] },
    };

    // toJSON properties are not callable, and are ignored silently
    print(JSON.stringify(obj));
}

function toJsonPropertyTest3() {
    var obj = {
        foo: 1,
        bar: { toJSON: function() { return 'bar/toJSON'; } },
        quux: { toJSON: 123 },
        quuux: { toJSON: function() { throw new Error('quuux/toJSON error'); } }
    };

    try {
        print(JSON.stringify(obj));
    } catch (e) {
        // message is assumed here to be intact
        print(e.name, e.message);
    }
}

function toJsonPropertyTest4() {
    var obj = {
        foo: 1,
        bar: { toJSON: function() { return 'bar/toJSON'; } },
        quux: { toJSON: 123 },
        quuux: new Date(Date.parse('2012-01-02T03:04:05.006Z'))
    };

    // Date.prototype.toJSON() ultimately calls Date.prototype.toISOString()
    // which has an exact output format
    print(JSON.stringify(obj));
}

function toJsonPropertyTest5() {
    function F() {
    }
    F.prototype = { toJSON: function() { return 'inherited toJSON'; } };

    var obj = {
        foo: 1,
        bar: { toJSON: function() { return 'bar/toJSON'; } },
        quux: { toJSON: 123 },
        quuux: new F()
    };

    // toJSON() can be inherited
    print(JSON.stringify(obj));
}

try {
    toJsonPropertyTest1();
    toJsonPropertyTest2();
    toJsonPropertyTest3();
    toJsonPropertyTest4();
    toJsonPropertyTest5();
} catch (e) {
    print(e.name);
}
