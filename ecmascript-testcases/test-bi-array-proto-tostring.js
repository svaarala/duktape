/*===
TypeError
TypeError
string [object Boolean]
string [object Boolean]
string [object Number]
string [object String]
string 1,2
string [object Object]
string [object Object]
string JOIN
string BOOLJOIN
proto.join
string foo;bar;quux
string [object Object]
MyJoinError
===*/

function basicTest() {
    var proto, obj;

    function test(this_value) {
        var t;

        try {
            t = Array.prototype.toString.call(this_value);
            print(typeof t, t);
        } catch (e) {
            print(e.name);
        }
    }

    // join() is called if the ToObject() coercion has it; for these basic tests
    // only the array ([1,2]) has a join().  Others fall back to the standard
    // Object.prototype.toString()

    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('foo');
    test([1,2]);
    test({ foo: 1, bar:1 });

    // explicit join() on an object

    test({ join: 123 });  // not callable, still fall back to Object.prototype.toString()
    test({ join: function() { return 'JOIN' } });

    // join() inherited

    Boolean.prototype.join = function() { return 'BOOLJOIN'; };
    test(true);

    // join() inherited, with joinable members

    proto = Object.create(Object.prototype);
    proto.join = function() {
        var i;
        var tmp = [];
        print('proto.join');
        for (i = 0; i < this.length; i++) {
            tmp.push(String(this[i]));
        }
        return tmp.join(';');
    };
    obj = Object.create(proto);
    obj.length = 3;
    obj['0'] = 'foo';
    obj['1'] = 'bar';
    obj['2'] = 'quux';
    test(obj);

    // the join() fallback is the standard Object.prototype.toString() function,
    // i.e. override should have no effect (must return '[object Object]' and
    // not 'OBJ-TOSTRING').

    Object.prototype.toString = function() { return 'OBJ-TOSTRING'; };
    test({ foo: 1, bar: 2 });

    // a join() error should propagate out
    test({
        join: function() {
            var e = new Error('join error');
            e.name = 'MyJoinError';
            throw e;
        }
    });
}

try {
    basicTest();
} catch (e) {
    print(e);
}
