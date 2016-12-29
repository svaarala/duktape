/*
 *  Reflect.apply()
 *  Reflect.construct()
 */

/*===
func() called with 2 args
Casper the friendly ghost
last arg: ghost
2
func() called with 3 args
maggie the eaty pig
*munch*
last arg: *munch*
3
func() called with 0 args
Darkling the undefined
last arg: undefined
0
func() called with 9001 args
Vegeta the Prince of All Saiyans
IT'S OVER NINE THOUSAAAAANNNDDDD!!!!!!
last arg: Frieza sux
9001
===*/

function applyTest() {
    var func = function(x, y, z) {
        print("func() called with", arguments.length, "args");
        if (arguments.length >= 2) {
            print(this.name, "the", x, y);
        } else {
            print(this.name, "the", x);
        }
        if (arguments.length >= 3) {
            print(z);
        }
        print("last arg:", arguments[arguments.length - 1]);
        return arguments.length;
    }

    var casper = { name: "Casper" };
    var maggie = { name: "maggie" };
    var darkling = { name: "Darkling" };
    var vegeta = { name: "Vegeta" };
    print(Reflect.apply(func, casper, [ "friendly", "ghost" ]));
    print(Reflect.apply(func, maggie, [ "eaty", "pig", "*munch*" ]));
    print(Reflect.apply(func, darkling, []));

    // So I may have been having a bit too much fun writing this test... :)
    //   ~ Bruce
    var argList = [ "Prince of All", "Saiyans", "IT'S OVER NINE THOUSAAAAANNNDDDD!!!!!!" ];
    while (argList.length < 9001) {
        argList[argList.length] = "Frieza sux";
    }
    print(Reflect.apply(func, vegeta, argList));
}

try {
    applyTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Person() called with 3 args
this instanceof Person? true
last arg: ghost
true
Hi, my name is Casper the friendly ghost
Person() called with 4 args
this instanceof Person? true
last arg: *MUNCH*
true
Hi, my name is maggie the eaty pig
maggie says: *MUNCH*
Person() called with 1 args
this instanceof Person? true
last arg: Darkling
true
Hi, my name is Darkling the undefined undefined
Person() called with 812 args
this instanceof Person? true
last arg: kittiez r food 4 cowz
true
Hi, my name is Kittycow the kitty-eating cow
Kittycow says: MOOOOOOoooooooooooooo...
===*/

function constructTest() {
    function Person(name, desc, type, remark) {
        print("Person() called with", arguments.length, "args");
        print("this instanceof Person?", this instanceof Person)
        this.name = name;
        this.desc = desc;
        this.type = type;
        this.haveRemark = false;
        if (arguments.length >= 4) {
            this.haveRemark = true;
            this.remark = remark;
        }
        print("last arg:", arguments[arguments.length - 1]);
    }
    Person.prototype = {
        talk: function() {
            print("Hi, my name is", this.name, "the", this.desc, this.type);
            if (this.haveRemark) {
                print(this.name, "says:", this.remark);
            }
        }
    };
    var ghost = Reflect.construct(Person, [ "Casper", "friendly", "ghost" ]);
    print(ghost instanceof Person);
    ghost.talk();
    var maggie = Reflect.construct(Person, [ "maggie", "eaty", "pig", "*MUNCH*" ]);
    print(maggie instanceof Person);
    maggie.talk();
    var darkling = Reflect.construct(Person, [ "Darkling" ]);
    print(darkling instanceof Person);
    darkling.talk();
    var argList = [ "Kittycow", "kitty-eating", "cow", "MOOOOOOoooooooooooooo..." ];
    while (argList.length < 812) {
        argList[argList.length] = "kittiez r food 4 cowz";
    }
    var cow = Reflect.construct(Person, argList);
    print(cow instanceof Person);
    cow.talk();
}

try {
    constructTest();
} catch (e) {
    print(e.stack || e);
}
