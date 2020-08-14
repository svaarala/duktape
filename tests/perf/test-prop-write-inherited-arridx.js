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
        // All of these will be misses, propagating up to root, then establishing a property.
        obj[1000] = 1; obj[1001] = 1; obj[1002] = 1; obj[1003] = 1; obj[1004] = 1; obj[1005] = 1; obj[1006] = 1; obj[1007] = 1; obj[1008] = 1; obj[1009] = 1;
        obj[1010] = 1; obj[1011] = 1; obj[1012] = 1; obj[1013] = 1; obj[1014] = 1; obj[1015] = 1; obj[1016] = 1; obj[1017] = 1; obj[1018] = 1; obj[1019] = 1;
        obj[1020] = 1; obj[1021] = 1; obj[1022] = 1; obj[1023] = 1; obj[1024] = 1; obj[1025] = 1; obj[1026] = 1; obj[1027] = 1; obj[1028] = 1; obj[1029] = 1;
        obj[1030] = 1; obj[1031] = 1; obj[1032] = 1; obj[1033] = 1; obj[1034] = 1; obj[1035] = 1; obj[1036] = 1; obj[1037] = 1; obj[1038] = 1; obj[1039] = 1;
        obj[1040] = 1; obj[1041] = 1; obj[1042] = 1; obj[1043] = 1; obj[1044] = 1; obj[1045] = 1; obj[1046] = 1; obj[1047] = 1; obj[1048] = 1; obj[1049] = 1;
        obj[1050] = 1; obj[1051] = 1; obj[1052] = 1; obj[1053] = 1; obj[1054] = 1; obj[1055] = 1; obj[1056] = 1; obj[1057] = 1; obj[1058] = 1; obj[1059] = 1;
        obj[1060] = 1; obj[1061] = 1; obj[1062] = 1; obj[1063] = 1; obj[1064] = 1; obj[1065] = 1; obj[1066] = 1; obj[1067] = 1; obj[1068] = 1; obj[1069] = 1;
        obj[1070] = 1; obj[1071] = 1; obj[1072] = 1; obj[1073] = 1; obj[1074] = 1; obj[1075] = 1; obj[1076] = 1; obj[1077] = 1; obj[1078] = 1; obj[1079] = 1;
        obj[1080] = 1; obj[1081] = 1; obj[1082] = 1; obj[1083] = 1; obj[1084] = 1; obj[1085] = 1; obj[1086] = 1; obj[1087] = 1; obj[1088] = 1; obj[1089] = 1;
        obj[1090] = 1; obj[1091] = 1; obj[1092] = 1; obj[1093] = 1; obj[1094] = 1; obj[1095] = 1; obj[1096] = 1; obj[1097] = 1; obj[1098] = 1; obj[1099] = 1;
    }
    print(ign);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
