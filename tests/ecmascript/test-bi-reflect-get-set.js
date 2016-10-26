/*
 *  Reflect.get()
 *  Reflect.set()
 */

/*===
Reflect.get()
own
inherited
get
magic
undefined
===*/

function getTest() {
    var proto = { inherited: "inherited" };
    var obj = Object.create(proto, {
        own: { value: "own" },
        magic: {
            get: function() { print("get"); return "magic"; }
        }
    });

    // Reflect.get() looks at own as well as inherited properties.
    print(Reflect.get(obj, 'own'));
    print(Reflect.get(obj, 'inherited'));

    // It also invokes getters.
    print(Reflect.get(obj, 'magic'));

    // Nonexistent properties return 'undefined' as you'd expect.
    print(Reflect.get(obj, 'nonexistent'));
}

try {
    print("Reflect.get()");
    getTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Reflect.set()
own,_magic,magic
true
true
own,_magic,magic,new
evul hackerz wuz here :)
the new thing
own,_magic,magic,new
inherited
true
manual override inherited
own,_magic,magic,new,inherited
set
true
get
witchcraft
false
undefined
false
evul hackerz wuz here :)
set
TypeError
get
witchcraft
===*/

function setTest() {
    'use strict';

    var proto = { inherited: "inherited" };
    var obj = Object.create(proto, {
        own: { value: "own", writable: true },
        _magic: { value: "magic", writable: true },
        magic: {
            get: function() { print("get"); return this._magic; },
            set: function(value) { print("set"); this._magic = value; }
        }
    });

    // Reflect.set() modifies existing properties as well as creates new ones:
    print(Reflect.ownKeys(obj));
    print(Reflect.set(obj, 'own', "evul hackerz wuz here :)"));
    print(Reflect.set(obj, 'new', "the new thing"));
    print(Reflect.ownKeys(obj));
    print(obj.own);
    print(obj.new);

    // Calling Reflect.set() for an inherited property will add it as an own
    // property of the target object.  The prototype property will not be
    // modified.
    print(Reflect.ownKeys(obj));
    print(obj.inherited);
    print(Reflect.set(obj, 'inherited', "manual override"));
    print(obj.inherited, Reflect.getPrototypeOf(obj).inherited);
    print(Reflect.ownKeys(obj));

    // It will call setters:
    print(Reflect.set(obj, 'magic', "witchcraft"));
    print(obj.magic);

    // However, it will fail to add properties to a non-extensible object or
    // modify non-writable properties (NB: no error is thrown even though the
    // caller is strict):
    Object.freeze(obj);
    print(Reflect.set(obj, 'foo', "bar"));
    print(obj.foo);
    print(Reflect.set(obj, 'own', "die stupid hacker!"));
    print(obj.own);

    // Setters will still be called even for a completely frozen object.  In
    // practice that will usually fail anyway because the underlying object is
    // immutable, but this is hardly specific to Reflect.set(), of course.
    try {
        Reflect.set(obj, 'magic', "voodoo");
        print("never here");
    } catch (e) {
        print(e.name);
    }
    print(obj.magic);
}

try {
    print("Reflect.set()");
    setTest();
} catch (e) {
    print(e.stack || e);
}
