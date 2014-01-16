
/*---
{
    "skip": true
}
---*/

/*===
FIXME
===*/

print('object constructor as function');

function constructorAsFunctionTest() {
}

try {
    constructorAsFunctionTest();
} catch (e) {
    print(e.name);
}

/*===
FIXME
===*/

print('object constructor as constructor');

function constructorTest() {
}

try {
    constructorTest();
} catch (e) {
    print(e.name);
}


