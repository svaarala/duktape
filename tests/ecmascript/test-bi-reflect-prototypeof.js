/*
 *  Reflect.getPrototypeOf()
 *  Reflect.setPrototypeOf()
 */

/*===
true
true
true
true
Megaman Protoman
true
Megaman undefined
null
true
Megaman Protoman
false
undefined
false
undefined
false
undefined
===*/

function test() {
    var proto = { proto: "Protoman" };
    var mega = Object.create(proto, { name: { value: "Megaman" } });
    var noExts = Object.preventExtensions({ prevent: "forest fires" });
    var sealed = Object.seal({ seal: "Andre" });
    var frozen = Object.freeze({ snowman: "Olaf" });

    // note: Object literals inherit from Object.prototype.
    print(Reflect.getPrototypeOf(proto) === Object.prototype);
    print(Reflect.getPrototypeOf(mega) === proto);
    print(Reflect.getPrototypeOf(function(){}) === Function.prototype);
    print(Reflect.getPrototypeOf([1,2,3]) === Array.prototype);

    // note: null prototype => inherit nothing.
    print(mega.name, mega.proto);
    print(Reflect.setPrototypeOf(mega, null));
    print(mega.name, mega.proto);
    print(Reflect.getPrototypeOf(mega));
    print(Reflect.setPrototypeOf(mega, proto));
    print(mega.name, mega.proto);

    // Setting prototype will fail for any object which has had any of these
    // done to it: preventExtensions(), freeze(), seal().
    print(Reflect.setPrototypeOf(noExts, mega));  // false
    print(noExts.name);
    print(Reflect.setPrototypeOf(sealed, mega));  // ditto
    print(sealed.name);
    print(Reflect.setPrototypeOf(frozen, mega));  // ditto
    print(frozen.name);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
