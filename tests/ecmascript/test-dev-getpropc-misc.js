/*
 *  Misc GETPROPC related tests
 */

/*===
non-shuffled test
obj.func called 1 2 3
TypeError: undefined not callable (property 'noSuch' of [object Array])
TypeError: undefined not callable (property 'noSuchComputed' of [object Array])
shuffled test
obj.func called 1 2 3
TypeError: undefined not callable (property 'noSuch' of [object Array])
TypeError: undefined not callable (property 'noSuch' of [object Array])
TypeError: undefined not callable (property 'noSuchComputed' of [object Array])
===*/

var obj = [];
obj.func = function func(a, b, c) {
    print('obj.func called', a, b, c);
};

function getObj() {
    return obj;
}

function nonShuffledTest() {
    obj.func(1, 2, 3);

    try {
        obj.noSuch(1, 2, 3);
    } catch (e) {
        print(e);
    }

    try {
        (123, getObj())['no' + 'Such' + 'Computed'](1, 2, 3);
    } catch (e) {
        print(e);
    }
}

try {
    print('non-shuffled test');
    nonShuffledTest();
} catch (e) {
    print(e.stack || e);
}

function shuffledTest() {
    var x000, x001, x002, x003, x004, x005, x006, x007, x008, x009;
    var x010, x011, x012, x013, x014, x015, x016, x017, x018, x019;
    var x020, x021, x022, x023, x024, x025, x026, x027, x028, x029;
    var x030, x031, x032, x033, x034, x035, x036, x037, x038, x039;
    var x040, x041, x042, x043, x044, x045, x046, x047, x048, x049;
    var x050, x051, x052, x053, x054, x055, x056, x057, x058, x059;
    var x060, x061, x062, x063, x064, x065, x066, x067, x068, x069;
    var x070, x071, x072, x073, x074, x075, x076, x077, x078, x079;
    var x080, x081, x082, x083, x084, x085, x086, x087, x088, x089;
    var x090, x091, x092, x093, x094, x095, x096, x097, x098, x099;
    var x100, x101, x102, x103, x104, x105, x106, x107, x108, x109;
    var x110, x111, x112, x113, x114, x115, x116, x117, x118, x119;
    var x120, x121, x122, x123, x124, x125, x126, x127, x128, x129;
    var x130, x131, x132, x133, x134, x135, x136, x137, x138, x139;
    var x140, x141, x142, x143, x144, x145, x146, x147, x148, x149;
    var x150, x151, x152, x153, x154, x155, x156, x157, x158, x159;
    var x160, x161, x162, x163, x164, x165, x166, x167, x168, x169;
    var x170, x171, x172, x173, x174, x175, x176, x177, x178, x179;
    var x180, x181, x182, x183, x184, x185, x186, x187, x188, x189;
    var x190, x191, x192, x193, x194, x195, x196, x197, x198, x199;
    var x200, x201, x202, x203, x204, x205, x206, x207, x208, x209;
    var x210, x211, x212, x213, x214, x215, x216, x217, x218, x219;
    var x220, x221, x222, x223, x224, x225, x226, x227, x228, x229;
    var x230, x231, x232, x233, x234, x235, x236, x237, x238, x239;
    var x240, x241, x242, x243, x244, x245, x246, x247, x248, x249;
    var x250, x251, x252, x253, x254, x255, x256, x257, x258, x259;
    var x260, x261, x262, x263, x264, x265, x266, x267, x268, x269;
    var x270, x271, x272, x273, x274, x275, x276, x277, x278, x279;
    var x280, x281, x282, x283, x284, x285, x286, x287, x288, x289;
    var x290, x291, x292, x293, x294, x295, x296, x297, x298, x299;

    obj.func(1, 2, 3);

    try {
        obj.noSuch(1, 2, 3);
    } catch (e) {
        print(e);
    }

    try {
        obj.noSuch('arg000', 'arg001', 'arg002', 'arg003', 'arg004',
                   'arg005', 'arg006', 'arg007', 'arg008', 'arg009',
                   'arg010', 'arg011', 'arg012', 'arg013', 'arg014',
                   'arg015', 'arg016', 'arg017', 'arg018', 'arg019',
                   'arg020', 'arg021', 'arg022', 'arg023', 'arg024',
                   'arg025', 'arg026', 'arg027', 'arg028', 'arg029',
                   'arg030', 'arg031', 'arg032', 'arg033', 'arg034',
                   'arg035', 'arg036', 'arg037', 'arg038', 'arg039',
                   'arg040', 'arg041', 'arg042', 'arg043', 'arg044',
                   'arg045', 'arg046', 'arg047', 'arg048', 'arg049',
                   'arg050', 'arg051', 'arg052', 'arg053', 'arg054',
                   'arg055', 'arg056', 'arg057', 'arg058', 'arg059',
                   'arg060', 'arg061', 'arg062', 'arg063', 'arg064',
                   'arg065', 'arg066', 'arg067', 'arg068', 'arg069',
                   'arg070', 'arg071', 'arg072', 'arg073', 'arg074',
                   'arg075', 'arg076', 'arg077', 'arg078', 'arg079',
                   'arg080', 'arg081', 'arg082', 'arg083', 'arg084',
                   'arg085', 'arg086', 'arg087', 'arg088', 'arg089',
                   'arg090', 'arg091', 'arg092', 'arg093', 'arg094',
                   'arg095', 'arg096', 'arg097', 'arg098', 'arg099',
                   'arg100', 'arg101', 'arg102', 'arg103', 'arg104',
                   'arg105', 'arg106', 'arg107', 'arg108', 'arg109',
                   'arg110', 'arg111', 'arg112', 'arg113', 'arg114',
                   'arg115', 'arg116', 'arg117', 'arg118', 'arg119',
                   'arg120', 'arg121', 'arg122', 'arg123', 'arg124',
                   'arg125', 'arg126', 'arg127', 'arg128', 'arg129',
                   'arg130', 'arg131', 'arg132', 'arg133', 'arg134',
                   'arg135', 'arg136', 'arg137', 'arg138', 'arg139',
                   'arg140', 'arg141', 'arg142', 'arg143', 'arg144',
                   'arg145', 'arg146', 'arg147', 'arg148', 'arg149',
                   'arg150', 'arg151', 'arg152', 'arg153', 'arg154',
                   'arg155', 'arg156', 'arg157', 'arg158', 'arg159',
                   'arg160', 'arg161', 'arg162', 'arg163', 'arg164',
                   'arg165', 'arg166', 'arg167', 'arg168', 'arg169',
                   'arg170', 'arg171', 'arg172', 'arg173', 'arg174',
                   'arg175', 'arg176', 'arg177', 'arg178', 'arg179',
                   'arg180', 'arg181', 'arg182', 'arg183', 'arg184',
                   'arg185', 'arg186', 'arg187', 'arg188', 'arg189',
                   'arg190', 'arg191', 'arg192', 'arg193', 'arg194',
                   'arg195', 'arg196', 'arg197', 'arg198', 'arg199',
                   'arg200', 'arg201', 'arg202', 'arg203', 'arg204',
                   'arg205', 'arg206', 'arg207', 'arg208', 'arg209',
                   'arg210', 'arg211', 'arg212', 'arg213', 'arg214',
                   'arg215', 'arg216', 'arg217', 'arg218', 'arg219',
                   'arg220', 'arg221', 'arg222', 'arg223', 'arg224',
                   'arg225', 'arg226', 'arg227', 'arg228', 'arg229',
                   'arg230', 'arg231', 'arg232', 'arg233', 'arg234',
                   'arg235', 'arg236', 'arg237', 'arg238', 'arg239',
                   'arg240', 'arg241', 'arg242', 'arg243', 'arg244',
                   'arg245', 'arg246', 'arg247', 'arg248', 'arg249',
                   'arg250', 'arg251', 'arg252', 'arg253', 'arg254',
                   'arg255', 'arg256', 'arg257', 'arg258', 'arg259',
                   'arg260', 'arg261', 'arg262', 'arg263', 'arg264',
                   'arg265', 'arg266', 'arg267', 'arg268', 'arg269',
                   'arg270', 'arg271', 'arg272', 'arg273', 'arg274',
                   'arg275', 'arg276', 'arg277', 'arg278', 'arg279',
                   'arg280', 'arg281', 'arg282', 'arg283', 'arg284',
                   'arg285', 'arg286', 'arg287', 'arg288', 'arg289',
                   'arg290', 'arg291', 'arg292', 'arg293', 'arg294',
                   'arg295', 'arg296', 'arg297', 'arg298', 'arg299');
    } catch (e) {
        print(e);
    }

    try {
        (123, getObj())['no' + 'Such' + 'Computed'](1, 2, 3);
    } catch (e) {
        print(e);
    }
}

try {
    print('shuffled test');
    shuffledTest();
} catch (e) {
    print(e.stack || e);
}
