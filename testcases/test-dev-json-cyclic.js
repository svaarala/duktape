/*===
TypeError
TypeError
TypeError
===*/

/* TypeError is required for cyclic structures. */

obj1 = { 'foo': 1 };
obj1.cycle = obj1;

obj2a = { 'foo': 1 };
obj2b = { 'bar': 2 };
obj2a.cycle = obj2b;
obj2b.cycle = obj2a;

obj3a = { 'foo': 1 };
obj3b = { 'bar': 2 };
obj3c = { 'quux': 3 };
obj3a.cycle = obj3b;
obj3b.cycle = obj3c;
obj3c.cycle = obj3a;

try {
    print(JSON.stringify(obj1));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.stringify(obj2a));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.stringify(obj3a));
} catch (e) {
    print(e.name);
}

