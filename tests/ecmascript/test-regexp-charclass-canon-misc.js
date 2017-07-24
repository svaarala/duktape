/*@include util-regexp.js@*/

/*===
F65536
T65536
T65535 F1
F1 T65535
===*/

print(RegExpUtil.getRegExpSingleCharMatches(/[]/));
print(RegExpUtil.getRegExpSingleCharMatches(/[\u0000-\uffff]/));
print(RegExpUtil.getRegExpSingleCharMatches(/[\u0000-\ufffe]/));
print(RegExpUtil.getRegExpSingleCharMatches(/[\u0001-\uffff]/));
