/*===
111
222
333
444
555
666
777
888
999
===*/

/* With enough constants, constant numbers emitted for e.g. GETPROP will no
 * longer fit into the short 9-bit arguments and will cause problems.
 * In the worst case a constant reference becomes a register reference
 * because the reg/const indicator bit will be flipped; this further
 * causes valgrind gripes.
 *
 * This needs to be fixed in the compiler so that it will croak when it
 * attempts to emit invalid code.  Also constants above the 9-bit addressable
 * range need to be coerced into temp registers with LDCONST.
 *
 * This test case illustrates the current problem: execution fails and
 * there are valgrind errors for invalid reads.
 */

// Test source must be eval'd; if it were part of the program, it would
// fail in compilation.  Use line continuations for the source, ugh.

var test_source = "\
function test() {\
    var obj;\
\
    obj = {\
        const0: 0, const1: 1, const2: 2, const3: 3, const4: 4, const5: 5, const6: 6, const7: 7,\
        const8: 8, const9: 9, const10: 10, const11: 11, const12: 12, const13: 13, const14: 14, const15: 15,\
        const16: 16, const17: 17, const18: 18, const19: 19, const20: 20, const21: 21, const22: 22, const23: 23,\
        const24: 24, const25: 25, const26: 26, const27: 27, const28: 28, const29: 29, const30: 30, const31: 31,\
        const32: 32, const33: 33, const34: 34, const35: 35, const36: 36, const37: 37, const38: 38, const39: 39,\
        const40: 40, const41: 41, const42: 42, const43: 43, const44: 44, const45: 45, const46: 46, const47: 47,\
        const48: 48, const49: 49, const50: 50, const51: 51, const52: 52, const53: 53, const54: 54, const55: 55,\
        const56: 56, const57: 57, const58: 58, const59: 59, const60: 60, const61: 61, const62: 62, const63: 63,\
        const64: 64, const65: 65, const66: 66, const67: 67, const68: 68, const69: 69, const70: 70, const71: 71,\
        const72: 72, const73: 73, const74: 74, const75: 75, const76: 76, const77: 77, const78: 78, const79: 79,\
        const80: 80, const81: 81, const82: 82, const83: 83, const84: 84, const85: 85, const86: 86, const87: 87,\
        const88: 88, const89: 89, const90: 90, const91: 91, const92: 92, const93: 93, const94: 94, const95: 95,\
        const96: 96, const97: 97, const98: 98, const99: 99, const100: 100, const101: 101, const102: 102, const103: 103,\
        const104: 104, const105: 105, const106: 106, const107: 107, const108: 108, const109: 109, const110: 110, const111: 111,\
        const112: 112, const113: 113, const114: 114, const115: 115, const116: 116, const117: 117, const118: 118, const119: 119,\
        const120: 120, const121: 121, const122: 122, const123: 123, const124: 124, const125: 125, const126: 126, const127: 127,\
        const128: 128, const129: 129, const130: 130, const131: 131, const132: 132, const133: 133, const134: 134, const135: 135,\
        const136: 136, const137: 137, const138: 138, const139: 139, const140: 140, const141: 141, const142: 142, const143: 143,\
        const144: 144, const145: 145, const146: 146, const147: 147, const148: 148, const149: 149, const150: 150, const151: 151,\
        const152: 152, const153: 153, const154: 154, const155: 155, const156: 156, const157: 157, const158: 158, const159: 159,\
        const160: 160, const161: 161, const162: 162, const163: 163, const164: 164, const165: 165, const166: 166, const167: 167,\
        const168: 168, const169: 169, const170: 170, const171: 171, const172: 172, const173: 173, const174: 174, const175: 175,\
        const176: 176, const177: 177, const178: 178, const179: 179, const180: 180, const181: 181, const182: 182, const183: 183,\
        const184: 184, const185: 185, const186: 186, const187: 187, const188: 188, const189: 189, const190: 190, const191: 191,\
        const192: 192, const193: 193, const194: 194, const195: 195, const196: 196, const197: 197, const198: 198, const199: 199,\
        const200: 200, const201: 201, const202: 202, const203: 203, const204: 204, const205: 205, const206: 206, const207: 207,\
        const208: 208, const209: 209, const210: 210, const211: 211, const212: 212, const213: 213, const214: 214, const215: 215,\
        const216: 216, const217: 217, const218: 218, const219: 219, const220: 220, const221: 221, const222: 222, const223: 223,\
        const224: 224, const225: 225, const226: 226, const227: 227, const228: 228, const229: 229, const230: 230, const231: 231,\
        const232: 232, const233: 233, const234: 234, const235: 235, const236: 236, const237: 237, const238: 238, const239: 239,\
        const240: 240, const241: 241, const242: 242, const243: 243, const244: 244, const245: 245, const246: 246, const247: 247,\
        const248: 248, const249: 249, const250: 250, const251: 251, const252: 252, const253: 253, const254: 254, const255: 255,\
        const256: 256, const257: 257, const258: 258, const259: 259, const260: 260, const261: 261, const262: 262, const263: 263,\
        const264: 264, const265: 265, const266: 266, const267: 267, const268: 268, const269: 269, const270: 270, const271: 271,\
        const272: 272, const273: 273, const274: 274, const275: 275, const276: 276, const277: 277, const278: 278, const279: 279,\
        const280: 280, const281: 281, const282: 282, const283: 283, const284: 284, const285: 285, const286: 286, const287: 287,\
        const288: 288, const289: 289, const290: 290, const291: 291, const292: 292, const293: 293, const294: 294, const295: 295,\
        const296: 296, const297: 297, const298: 298, const299: 299, const300: 300, const301: 301, const302: 302, const303: 303,\
        const304: 304, const305: 305, const306: 306, const307: 307, const308: 308, const309: 309, const310: 310, const311: 311,\
        const312: 312, const313: 313, const314: 314, const315: 315, const316: 316, const317: 317, const318: 318, const319: 319,\
        const320: 320, const321: 321, const322: 322, const323: 323, const324: 324, const325: 325, const326: 326, const327: 327,\
        const328: 328, const329: 329, const330: 330, const331: 331, const332: 332, const333: 333, const334: 334, const335: 335,\
        const336: 336, const337: 337, const338: 338, const339: 339, const340: 340, const341: 341, const342: 342, const343: 343,\
        const344: 344, const345: 345, const346: 346, const347: 347, const348: 348, const349: 349, const350: 350, const351: 351,\
        const352: 352, const353: 353, const354: 354, const355: 355, const356: 356, const357: 357, const358: 358, const359: 359,\
        const360: 360, const361: 361, const362: 362, const363: 363, const364: 364, const365: 365, const366: 366, const367: 367,\
        const368: 368, const369: 369, const370: 370, const371: 371, const372: 372, const373: 373, const374: 374, const375: 375,\
        const376: 376, const377: 377, const378: 378, const379: 379, const380: 380, const381: 381, const382: 382, const383: 383,\
        const384: 384, const385: 385, const386: 386, const387: 387, const388: 388, const389: 389, const390: 390, const391: 391,\
        const392: 392, const393: 393, const394: 394, const395: 395, const396: 396, const397: 397, const398: 398, const399: 399,\
        const400: 400, const401: 401, const402: 402, const403: 403, const404: 404, const405: 405, const406: 406, const407: 407,\
        const408: 408, const409: 409, const410: 410, const411: 411, const412: 412, const413: 413, const414: 414, const415: 415,\
        const416: 416, const417: 417, const418: 418, const419: 419, const420: 420, const421: 421, const422: 422, const423: 423,\
        const424: 424, const425: 425, const426: 426, const427: 427, const428: 428, const429: 429, const430: 430, const431: 431,\
        const432: 432, const433: 433, const434: 434, const435: 435, const436: 436, const437: 437, const438: 438, const439: 439,\
        const440: 440, const441: 441, const442: 442, const443: 443, const444: 444, const445: 445, const446: 446, const447: 447,\
        const448: 448, const449: 449, const450: 450, const451: 451, const452: 452, const453: 453, const454: 454, const455: 455,\
        const456: 456, const457: 457, const458: 458, const459: 459, const460: 460, const461: 461, const462: 462, const463: 463,\
        const464: 464, const465: 465, const466: 466, const467: 467, const468: 468, const469: 469, const470: 470, const471: 471,\
        const472: 472, const473: 473, const474: 474, const475: 475, const476: 476, const477: 477, const478: 478, const479: 479,\
        const480: 480, const481: 481, const482: 482, const483: 483, const484: 484, const485: 485, const486: 486, const487: 487,\
        const488: 488, const489: 489, const490: 490, const491: 491, const492: 492, const493: 493, const494: 494, const495: 495,\
        const496: 496, const497: 497, const498: 498, const499: 499, const500: 500, const501: 501, const502: 502, const503: 503,\
        const504: 504, const505: 505, const506: 506, const507: 507, const508: 508, const509: 509, const510: 510, const511: 511,\
        const512: 512, const513: 513, const514: 514, const515: 515, const516: 516, const517: 517, const518: 518, const519: 519,\
        const520: 520, const521: 521, const522: 522, const523: 523, const524: 524, const525: 525, const526: 526, const527: 527,\
        const528: 528, const529: 529, const530: 530, const531: 531, const532: 532, const533: 533, const534: 534, const535: 535,\
        const536: 536, const537: 537, const538: 538, const539: 539, const540: 540, const541: 541, const542: 542, const543: 543,\
        const544: 544, const545: 545, const546: 546, const547: 547, const548: 548, const549: 549, const550: 550, const551: 551,\
        const552: 552, const553: 553, const554: 554, const555: 555, const556: 556, const557: 557, const558: 558, const559: 559,\
        const560: 560, const561: 561, const562: 562, const563: 563, const564: 564, const565: 565, const566: 566, const567: 567,\
        const568: 568, const569: 569, const570: 570, const571: 571, const572: 572, const573: 573, const574: 574, const575: 575,\
        const576: 576, const577: 577, const578: 578, const579: 579, const580: 580, const581: 581, const582: 582, const583: 583,\
        const584: 584, const585: 585, const586: 586, const587: 587, const588: 588, const589: 589, const590: 590, const591: 591,\
        const592: 592, const593: 593, const594: 594, const595: 595, const596: 596, const597: 597, const598: 598, const599: 599,\
        const600: 600, const601: 601, const602: 602, const603: 603, const604: 604, const605: 605, const606: 606, const607: 607,\
        const608: 608, const609: 609, const610: 610, const611: 611, const612: 612, const613: 613, const614: 614, const615: 615,\
        const616: 616, const617: 617, const618: 618, const619: 619, const620: 620, const621: 621, const622: 622, const623: 623,\
        const624: 624, const625: 625, const626: 626, const627: 627, const628: 628, const629: 629, const630: 630, const631: 631,\
        const632: 632, const633: 633, const634: 634, const635: 635, const636: 636, const637: 637, const638: 638, const639: 639,\
        const640: 640, const641: 641, const642: 642, const643: 643, const644: 644, const645: 645, const646: 646, const647: 647,\
        const648: 648, const649: 649, const650: 650, const651: 651, const652: 652, const653: 653, const654: 654, const655: 655,\
        const656: 656, const657: 657, const658: 658, const659: 659, const660: 660, const661: 661, const662: 662, const663: 663,\
        const664: 664, const665: 665, const666: 666, const667: 667, const668: 668, const669: 669, const670: 670, const671: 671,\
        const672: 672, const673: 673, const674: 674, const675: 675, const676: 676, const677: 677, const678: 678, const679: 679,\
        const680: 680, const681: 681, const682: 682, const683: 683, const684: 684, const685: 685, const686: 686, const687: 687,\
        const688: 688, const689: 689, const690: 690, const691: 691, const692: 692, const693: 693, const694: 694, const695: 695,\
        const696: 696, const697: 697, const698: 698, const699: 699, const700: 700, const701: 701, const702: 702, const703: 703,\
        const704: 704, const705: 705, const706: 706, const707: 707, const708: 708, const709: 709, const710: 710, const711: 711,\
        const712: 712, const713: 713, const714: 714, const715: 715, const716: 716, const717: 717, const718: 718, const719: 719,\
        const720: 720, const721: 721, const722: 722, const723: 723, const724: 724, const725: 725, const726: 726, const727: 727,\
        const728: 728, const729: 729, const730: 730, const731: 731, const732: 732, const733: 733, const734: 734, const735: 735,\
        const736: 736, const737: 737, const738: 738, const739: 739, const740: 740, const741: 741, const742: 742, const743: 743,\
        const744: 744, const745: 745, const746: 746, const747: 747, const748: 748, const749: 749, const750: 750, const751: 751,\
        const752: 752, const753: 753, const754: 754, const755: 755, const756: 756, const757: 757, const758: 758, const759: 759,\
        const760: 760, const761: 761, const762: 762, const763: 763, const764: 764, const765: 765, const766: 766, const767: 767,\
        const768: 768, const769: 769, const770: 770, const771: 771, const772: 772, const773: 773, const774: 774, const775: 775,\
        const776: 776, const777: 777, const778: 778, const779: 779, const780: 780, const781: 781, const782: 782, const783: 783,\
        const784: 784, const785: 785, const786: 786, const787: 787, const788: 788, const789: 789, const790: 790, const791: 791,\
        const792: 792, const793: 793, const794: 794, const795: 795, const796: 796, const797: 797, const798: 798, const799: 799,\
        const800: 800, const801: 801, const802: 802, const803: 803, const804: 804, const805: 805, const806: 806, const807: 807,\
        const808: 808, const809: 809, const810: 810, const811: 811, const812: 812, const813: 813, const814: 814, const815: 815,\
        const816: 816, const817: 817, const818: 818, const819: 819, const820: 820, const821: 821, const822: 822, const823: 823,\
        const824: 824, const825: 825, const826: 826, const827: 827, const828: 828, const829: 829, const830: 830, const831: 831,\
        const832: 832, const833: 833, const834: 834, const835: 835, const836: 836, const837: 837, const838: 838, const839: 839,\
        const840: 840, const841: 841, const842: 842, const843: 843, const844: 844, const845: 845, const846: 846, const847: 847,\
        const848: 848, const849: 849, const850: 850, const851: 851, const852: 852, const853: 853, const854: 854, const855: 855,\
        const856: 856, const857: 857, const858: 858, const859: 859, const860: 860, const861: 861, const862: 862, const863: 863,\
        const864: 864, const865: 865, const866: 866, const867: 867, const868: 868, const869: 869, const870: 870, const871: 871,\
        const872: 872, const873: 873, const874: 874, const875: 875, const876: 876, const877: 877, const878: 878, const879: 879,\
        const880: 880, const881: 881, const882: 882, const883: 883, const884: 884, const885: 885, const886: 886, const887: 887,\
        const888: 888, const889: 889, const890: 890, const891: 891, const892: 892, const893: 893, const894: 894, const895: 895,\
        const896: 896, const897: 897, const898: 898, const899: 899, const900: 900, const901: 901, const902: 902, const903: 903,\
        const904: 904, const905: 905, const906: 906, const907: 907, const908: 908, const909: 909, const910: 910, const911: 911,\
        const912: 912, const913: 913, const914: 914, const915: 915, const916: 916, const917: 917, const918: 918, const919: 919,\
        const920: 920, const921: 921, const922: 922, const923: 923, const924: 924, const925: 925, const926: 926, const927: 927,\
        const928: 928, const929: 929, const930: 930, const931: 931, const932: 932, const933: 933, const934: 934, const935: 935,\
        const936: 936, const937: 937, const938: 938, const939: 939, const940: 940, const941: 941, const942: 942, const943: 943,\
        const944: 944, const945: 945, const946: 946, const947: 947, const948: 948, const949: 949, const950: 950, const951: 951,\
        const952: 952, const953: 953, const954: 954, const955: 955, const956: 956, const957: 957, const958: 958, const959: 959,\
        const960: 960, const961: 961, const962: 962, const963: 963, const964: 964, const965: 965, const966: 966, const967: 967,\
        const968: 968, const969: 969, const970: 970, const971: 971, const972: 972, const973: 973, const974: 974, const975: 975,\
        const976: 976, const977: 977, const978: 978, const979: 979, const980: 980, const981: 981, const982: 982, const983: 983,\
        const984: 984, const985: 985, const986: 986, const987: 987, const988: 988, const989: 989, const990: 990, const991: 991,\
        const992: 992, const993: 993, const994: 994, const995: 995, const996: 996, const997: 997, const998: 998, const999: 999\
    };\
\
    print(obj.const111);\
    print(obj.const222);\
    print(obj.const333);\
    print(obj.const444);\
    print(obj.const555);\
    print(obj.const666);\
    print(obj.const777);\
    print(obj.const888);\
    print(obj.const999);\
}\
";

try {
    eval(test_source + "; test();");
} catch (e) {
    print(e);
}
