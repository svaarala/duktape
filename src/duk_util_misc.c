/*
 *  Misc util stuff
 */

#include "duk_internal.h"

/*
 *  Lowercase digits for radix values 2 to 36.  Also doubles as lowercase
 *  hex nybble table.
 */

char duk_lc_digits[36] = { '0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
                           'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                           'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                           'w', 'x', 'y', 'z' };

char duk_uc_nybbles[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
