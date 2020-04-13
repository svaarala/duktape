/*
 *  Basic property write performance for an inherited property
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var root = {};
    var i;
    var ign;
    var obj;

    for (i = 0; i < 100; i++) {
        root['prop' + i] = 123;
    }
    if (typeof Duktape !== 'undefined') { Duktape.compact(root); }

    for (i = 0; i < 1e5; i++) {
        obj = Object.create(Object.create(root));  // two levels of inheritance
        obj.prop0 = 1; obj.prop1 = 1; obj.prop2 = 1; obj.prop3 = 1; obj.prop4 = 1; obj.prop5 = 1; obj.prop6 = 1; obj.prop7 = 1; obj.prop8 = 1; obj.prop9 = 1;
        obj.prop10 = 1; obj.prop11 = 1; obj.prop12 = 1; obj.prop13 = 1; obj.prop14 = 1; obj.prop15 = 1; obj.prop16 = 1; obj.prop17 = 1; obj.prop18 = 1; obj.prop19 = 1;
        obj.prop20 = 1; obj.prop21 = 1; obj.prop22 = 1; obj.prop23 = 1; obj.prop24 = 1; obj.prop25 = 1; obj.prop26 = 1; obj.prop27 = 1; obj.prop28 = 1; obj.prop29 = 1;
        obj.prop30 = 1; obj.prop31 = 1; obj.prop32 = 1; obj.prop33 = 1; obj.prop34 = 1; obj.prop35 = 1; obj.prop36 = 1; obj.prop37 = 1; obj.prop38 = 1; obj.prop39 = 1;
        obj.prop40 = 1; obj.prop41 = 1; obj.prop42 = 1; obj.prop43 = 1; obj.prop44 = 1; obj.prop45 = 1; obj.prop46 = 1; obj.prop47 = 1; obj.prop48 = 1; obj.prop49 = 1;
        obj.prop50 = 1; obj.prop51 = 1; obj.prop52 = 1; obj.prop53 = 1; obj.prop54 = 1; obj.prop55 = 1; obj.prop56 = 1; obj.prop57 = 1; obj.prop58 = 1; obj.prop59 = 1;
        obj.prop60 = 1; obj.prop61 = 1; obj.prop62 = 1; obj.prop63 = 1; obj.prop64 = 1; obj.prop65 = 1; obj.prop66 = 1; obj.prop67 = 1; obj.prop68 = 1; obj.prop69 = 1;
        obj.prop70 = 1; obj.prop71 = 1; obj.prop72 = 1; obj.prop73 = 1; obj.prop74 = 1; obj.prop75 = 1; obj.prop76 = 1; obj.prop77 = 1; obj.prop78 = 1; obj.prop79 = 1;
        obj.prop80 = 1; obj.prop81 = 1; obj.prop82 = 1; obj.prop83 = 1; obj.prop84 = 1; obj.prop85 = 1; obj.prop86 = 1; obj.prop87 = 1; obj.prop88 = 1; obj.prop89 = 1;
        obj.prop90 = 1; obj.prop91 = 1; obj.prop92 = 1; obj.prop93 = 1; obj.prop94 = 1; obj.prop95 = 1; obj.prop96 = 1; obj.prop97 = 1; obj.prop98 = 1; obj.prop99 = 1;
    }
    print(ign);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
