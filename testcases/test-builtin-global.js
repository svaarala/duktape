/*
 *  Builtin global (E5 Sections 15.1, B.2.1, B.2.2)
 */

/*---
{
    "skip": true
}
---*/

var indirectEval = eval;
var global = indirectEval('this');

/*===
[object global]
===*/

/* [[Class]] implementation defined, but we expect 'global' */

print(global);

/*===
TypeError
TypeError
===*/

/* Not callable or constructable */

try {
    global();
} catch (e) {
    print(e.name);
}

try {
    new global();
} catch (e) {
    print(e.name);
}

/* FIXME */

