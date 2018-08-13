#!/usr/bin/env python2
#
#  Utility to dump bytecode into a human readable form.
#

import struct
import optparse

ops = [{'args': ['A_R', 'BC_R'], 'name': 'LDREG'},
       {'args': ['A_R', 'BC_R'], 'name': 'STREG'},
       {'args': ['ABC_JUMP'], 'name': 'JUMP'},
       {'args': ['A_R', 'BC_C'], 'name': 'LDCONST'},
       {'args': ['A_R', 'BC_LDINT'], 'name': 'LDINT'},
       {'args': ['A_R', 'BC_LDINTX'], 'name': 'LDINTX'},
       {'args': ['BC_R'], 'name': 'LDTHIS'},
       {'args': ['BC_R'], 'name': 'LDUNDEF'},
       {'args': ['BC_R'], 'name': 'LDNULL'},
       {'args': ['BC_R'], 'name': 'LDTRUE'},
       {'args': ['BC_R'], 'name': 'LDFALSE'},
       {'args': ['A_R', 'BC_C'], 'name': 'GETVAR'},
       {'args': ['A_R', 'BC_R'], 'name': 'BNOT'},
       {'args': ['A_R', 'BC_R'], 'name': 'LNOT'},
       {'args': ['A_R', 'BC_R'], 'name': 'UNM'},
       {'args': ['A_R', 'BC_R'], 'name': 'UNP'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'EQ_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'EQ_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'EQ_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'EQ_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'NEQ_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'NEQ_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'NEQ_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'NEQ_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'SEQ_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'SEQ_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'SEQ_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'SEQ_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'SNEQ_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'SNEQ_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'SNEQ_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'SNEQ_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'GT_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'GT_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'GT_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'GT_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'GE_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'GE_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'GE_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'GE_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'LT_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'LT_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'LT_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'LT_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'LE_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'LE_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'LE_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'LE_CC'},
       {'args': ['BC_R'], 'name': 'IFTRUE_R'},
       {'args': ['BC_C'], 'name': 'IFTRUE_C'},
       {'args': ['BC_R'], 'name': 'IFFALSE_R'},
       {'args': ['BC_C'], 'name': 'IFFALSE_C'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'ADD_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'ADD_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'ADD_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'ADD_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'SUB_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'SUB_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'SUB_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'SUB_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'MUL_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'MUL_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'MUL_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'MUL_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'DIV_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'DIV_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'DIV_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'DIV_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'MOD_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'MOD_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'MOD_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'MOD_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'EXP_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'EXP_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'EXP_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'EXP_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'BAND_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'BAND_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'BAND_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'BAND_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'BOR_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'BOR_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'BOR_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'BOR_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'BXOR_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'BXOR_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'BXOR_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'BXOR_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'BASL_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'BASL_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'BASL_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'BASL_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'BLSR_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'BLSR_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'BLSR_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'BLSR_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'BASR_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'BASR_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'BASR_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'BASR_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'INSTOF_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'INSTOF_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'INSTOF_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'INSTOF_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'IN_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'IN_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'IN_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'IN_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'GETPROP_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'GETPROP_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'GETPROP_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'GETPROP_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'PUTPROP_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'PUTPROP_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'PUTPROP_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'PUTPROP_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'DELPROP_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'DELPROP_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'DELPROP_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'DELPROP_CC'},
       {'args': ['A_R', 'BC_R'], 'name': 'PREINCR'},
       {'args': ['A_R', 'BC_R'], 'name': 'PREDECR'},
       {'args': ['A_R', 'BC_R'], 'name': 'POSTINCR'},
       {'args': ['A_R', 'BC_R'], 'name': 'POSTDECR'},
       {'args': ['A_R', 'BC_C'], 'name': 'PREINCV'},
       {'args': ['A_R', 'BC_C'], 'name': 'PREDECV'},
       {'args': ['A_R', 'BC_C'], 'name': 'POSTINCV'},
       {'args': ['A_R', 'BC_C'], 'name': 'POSTDECV'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'PREINCP_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'PREINCP_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'PREINCP_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'PREINCP_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'PREDECP_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'PREDECP_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'PREDECP_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'PREDECP_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'POSTINCP_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'POSTINCP_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'POSTINCP_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'POSTINCP_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'POSTDECP_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'POSTDECP_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'POSTDECP_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'POSTDECP_CC'},
       {'args': ['A_H', 'B_R', 'C_R'],
        'flags': [{'mask': 256, 'name': 'writable'},
                  {'mask': 512, 'name': 'enumerable'},
                  {'mask': 1024, 'name': 'configurable'},
                  {'mask': 2048, 'name': 'accessor'},
                  {'mask': 4096, 'name': 'func_decl'}],
        'name': 'DECLVAR_RR'},
       {'args': ['A_H', 'B_C', 'C_R'],
        'flags': [{'mask': 256, 'name': 'writable'},
                  {'mask': 512, 'name': 'enumerable'},
                  {'mask': 1024, 'name': 'configurable'},
                  {'mask': 2048, 'name': 'accessor'},
                  {'mask': 4096, 'name': 'func_decl'}],
        'name': 'DECLVAR_CR'},
       {'args': ['A_H', 'B_R', 'C_C'],
        'flags': [{'mask': 256, 'name': 'writable'},
                  {'mask': 512, 'name': 'enumerable'},
                  {'mask': 1024, 'name': 'configurable'},
                  {'mask': 2048, 'name': 'accessor'},
                  {'mask': 4096, 'name': 'func_decl'}],
        'name': 'DECLVAR_RC'},
       {'args': ['A_H', 'B_C', 'C_C'],
        'flags': [{'mask': 256, 'name': 'writable'},
                  {'mask': 512, 'name': 'enumerable'},
                  {'mask': 1024, 'name': 'configurable'},
                  {'mask': 2048, 'name': 'accessor'},
                  {'mask': 4096, 'name': 'func_decl'}],
        'name': 'DECLVAR_CC'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'REGEXP_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'REGEXP_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'REGEXP_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'REGEXP_CC'},
       {'args': ['A_R', 'BC_I'], 'name': 'CLOSURE'},
       {'args': ['A_R', 'BC_R'], 'name': 'TYPEOF'},
       {'args': ['A_R', 'BC_C'], 'name': 'TYPEOFID'},
       {'args': ['A_R', 'BC_C'], 'name': 'PUTVAR'},
       {'args': ['A_R', 'BC_C'], 'name': 'DELVAR'},
       {'args': ['BC_R'], 'name': 'RETREG'},
       {'name': 'RETUNDEF'},
       {'args': ['BC_C'], 'name': 'RETCONST'},
       {'args': ['BC_C'], 'name': 'RETCONSTN'},
       {'args': ['BC_I'], 'name': 'LABEL'},
       {'args': ['BC_I'], 'name': 'ENDLABEL'},
       {'args': ['BC_I'], 'name': 'BREAK'},
       {'args': ['BC_I'], 'name': 'CONTINUE'},
       {'args': ['A_H', 'BC_R'],
        'flags': [{'mask': 64, 'name': 'have_catch'},
                  {'mask': 128, 'name': 'have_finally'},
                  {'mask': 256, 'name': 'catch_binding'},
                  {'mask': 512, 'name': 'with_binding'}],
        'name': 'TRYCATCH'},
       {'name': 'ENDTRY'},
       {'name': 'ENDCATCH'},
       {'args': ['ABC_R'], 'name': 'ENDFIN'},
       {'args': ['BC_R'], 'name': 'THROW'},
       {'name': 'INVLHS'},
       {'args': ['A_R', 'BC_R'], 'name': 'CSREG'},
       {'args': ['A_R', 'B_R'], 'name': 'CSVAR_RR'},
       {'args': ['A_R', 'B_C'], 'name': 'CSVAR_CR'},
       {'args': ['A_R', 'B_R'], 'name': 'CSVAR_RC'},
       {'args': ['A_R', 'B_C'], 'name': 'CSVAR_CC'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL0'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL1'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL2'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL3'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL4'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL5'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL6'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL7'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL8'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL9'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL10'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL11'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL12'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL13'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL14'},
       {'args': ['A_I', 'BC_R'], 'name': 'CALL15'},
       {'args': ['A_I', 'BC_R'], 'name': 'NEWOBJ'},
       {'args': ['A_I', 'BC_R'], 'name': 'NEWARR'},
       {'args': ['A_R', 'B_R', 'C_I'], 'name': 'MPUTOBJ'},
       {'args': ['A_R', 'B_RI', 'C_I'], 'name': 'MPUTOBJI'},
       {'args': ['A_R', 'BC_R'], 'name': 'INITSET'},
       {'args': ['A_R', 'BC_R'], 'name': 'INITGET'},
       {'args': ['A_R', 'B_R', 'C_I'], 'name': 'MPUTARR'},
       {'args': ['A_R', 'B_RI', 'C_I'], 'name': 'MPUTARRI'},
       {'args': ['B_R', 'C_R'], 'name': 'SETALEN'},
       {'args': ['B_R', 'C_R'], 'name': 'INITENUM'},
       {'args': ['B_R', 'C_R'], 'name': 'NEXTENUM'},
       {'args': ['BC_R'], 'name': 'NEWTARGET'},
       {'name': 'DEBUGGER'},
       {'args': ['ABC_I'], 'name': 'NOP'},
       {'args': ['ABC_I'], 'name': 'INVALID'},
       {'name': 'UNUSED207'},
       {'args': ['A_R', 'B_R', 'C_R'], 'name': 'GETPROPC_RR'},
       {'args': ['A_R', 'B_C', 'C_R'], 'name': 'GETPROPC_CR'},
       {'args': ['A_R', 'B_R', 'C_C'], 'name': 'GETPROPC_RC'},
       {'args': ['A_R', 'B_C', 'C_C'], 'name': 'GETPROPC_CC'},
       {'name': 'UNUSED212'},
       {'name': 'UNUSED213'},
       {'name': 'UNUSED214'},
       {'name': 'UNUSED215'},
       {'name': 'UNUSED216'},
       {'name': 'UNUSED217'},
       {'name': 'UNUSED218'},
       {'name': 'UNUSED219'},
       {'name': 'UNUSED220'},
       {'name': 'UNUSED221'},
       {'name': 'UNUSED222'},
       {'name': 'UNUSED223'},
       {'name': 'UNUSED224'},
       {'name': 'UNUSED225'},
       {'name': 'UNUSED226'},
       {'name': 'UNUSED227'},
       {'name': 'UNUSED228'},
       {'name': 'UNUSED229'},
       {'name': 'UNUSED230'},
       {'name': 'UNUSED231'},
       {'name': 'UNUSED232'},
       {'name': 'UNUSED233'},
       {'name': 'UNUSED234'},
       {'name': 'UNUSED235'},
       {'name': 'UNUSED236'},
       {'name': 'UNUSED237'},
       {'name': 'UNUSED238'},
       {'name': 'UNUSED239'},
       {'name': 'UNUSED240'},
       {'name': 'UNUSED241'},
       {'name': 'UNUSED242'},
       {'name': 'UNUSED243'},
       {'name': 'UNUSED244'},
       {'name': 'UNUSED245'},
       {'name': 'UNUSED246'},
       {'name': 'UNUSED247'},
       {'name': 'UNUSED248'},
       {'name': 'UNUSED249'},
       {'name': 'UNUSED250'},
       {'name': 'UNUSED251'},
       {'name': 'UNUSED252'},
       {'name': 'UNUSED253'},
       {'name': 'UNUSED254'},
       {'name': 'UNUSED255'}]


def decode_string(buf, off):
    strlen, = struct.unpack('>L', buf[off:off + 4])
    off += 4
    strdata = buf[off:off + strlen]
    off += strlen

    return off, strdata


def sanitize_string(val):
    # Don't try to UTF-8 decode, just escape non-printable ASCII.
    def f(c):
        if ord(c) < 0x20 or ord(c) > 0x7e or c in '\'"':
            return '\\x%02x' % ord(c)
        else:
            return c

    return "'" + ''.join(map(f, val)) + "'"


def decode_sanitize_string(buf, off):
    off, val = decode_string(buf, off)
    return off, sanitize_string(val)


def dump_ins(ins, x):
    global ops
    pc = x / 4
    args = []

    op = ops[ins & 0xff]
    comments = []

    if 'args' in op:
        for j in xrange(len(op['args'])):
            A = (ins >> 8) & 0xff
            B = (ins >> 16) & 0xff
            C = (ins >> 24) & 0xff
            BC = (ins >> 16) & 0xffff
            ABC = (ins >> 8) & 0xffffff

            Bconst = ins & 0x1
            Cconst = ins & 0x2

            if op['args'][j] == 'A_R':
                args.append('r' + str(A))

            elif op['args'][j] == 'A_RI':
                args.append('r' + str(A) + '(indirect)')

            elif op['args'][j] == 'A_C':
                args.append('c' + str(A))

            elif op['args'][j] == 'A_H':
                args.append(hex(A))

            elif op['args'][j] == 'A_I':
                args.append(str(A))

            elif op['args'][j] == 'A_B':
                args.append('true' if A else 'false')

            elif op['args'][j] == 'B_RC':
                args.append('c' if Bconst else 'r' + str(B))

            elif op['args'][j] == 'B_R':
                args.append('r' + str(B))

            elif op['args'][j] == 'B_RI':
                args.append('r' + str(B) + '(indirect)')

            elif op['args'][j] == 'B_C':
                args.append('c' + str(B))

            elif op['args'][j] == 'B_H':
                args.append(hex(B))

            elif op['args'][j] == 'B_I':
                args.append(str(B))

            elif op['args'][j] == 'C_RC':
                args.append('c' if Cconst else 'r' + str(C))

            elif op['args'][j] == 'C_R':
                args.append('r' + str(C))

            elif op['args'][j] == 'C_RI':
                args.append('r' + str(C) + '(indirect)')

            elif op['args'][j] == 'C_C':
                args.append('c' + str(C))

            elif op['args'][j] == 'C_H':
                args.append(hex(C))

            elif op['args'][j] == 'C_I':
                args.append(str(C))

            elif op['args'][j] == 'BC_R':
                args.append('r' + str(BC))

            elif op['args'][j] == 'BC_C':
                args.append('c' + str(BC))

            elif op['args'][j] == 'BC_H':
                args.append(hex(BC))

            elif op['args'][j] == 'BC_I':
                args.append(str(BC))

            elif op['args'][j] == 'ABC_H':
                args.append(hex(ABC))

            elif op['args'][j] == 'ABC_I':
                args.append(str(ABC))

            elif op['args'][j] == 'BC_LDINT':
                args.append(hex(BC - (1 << 15)))

            elif op['args'][j] == 'BC_LDINTX':
                args.append(hex(BC))
            elif op['args'][j] == 'ABC_JUMP':
                pc_add = ABC - (1 << 23) + 1
                pc_dst = pc + pc_add
                args.append(str(pc_dst) + ' (' + ('+' if pc_add >= 0 else '') + str(pc_add) + ')')
            else:
                args.append('?')

    if 'flags' in op:
        for f in op['flags']:
            if ins & f['mask']:
                comments.append(f['name'])

    if len(args):
        res = '{} {}'.format(op['name'], ', '.join(args))
    else:
        res = op['name']

    if len(comments):
        res = '{} //{}'.format(res, ', '.join(comments))

    return res


def dump_function(buf, off, ind):
    count_inst, count_const, count_funcs = struct.unpack('>LLL', buf[off:off + 12])
    off += 12
    print('%sInstructions: %d' % (ind, count_inst))
    print('%sConstants: %d' % (ind, count_const))
    print('%sInner functions: %d' % (ind, count_funcs))

    # Line numbers present, assuming debugger support; otherwise 0.
    nregs, nargs, start_line, end_line = struct.unpack('>HHLL', buf[off:off + 12])
    off += 12
    print('%sNregs: %d' % (ind, nregs))
    print('%sNargs: %d' % (ind, nargs))
    print('%sStart line number: %d' % (ind, start_line))
    print('%sEnd line number: %d' % (ind, end_line))

    compfunc_flags, = struct.unpack('>L', buf[off:off + 4])
    off += 4
    print('%sduk_hcompiledfunction flags: 0x%08x' % (ind, compfunc_flags))

    for i in xrange(count_inst):
        ins, = struct.unpack('>L', buf[off:off + 4])
        off += 4
        code = dump_ins(ins, i)  # dump instrument
        print('%s  %06d: %08lx %s' % (ind, i, ins, code))

    print('%sConstants:' % ind)
    for i in xrange(count_const):
        const_type, = struct.unpack('B', buf[off:off + 1])
        off += 1

        if const_type == 0x00:
            off, strdata = decode_sanitize_string(buf, off)
            print('%s  %06d: %s' % (ind, i, strdata))
        elif const_type == 0x01:
            num, = struct.unpack('>d', buf[off:off + 8])
            off += 8
            print('%s  %06d: %f' % (ind, i, num))
        else:
            raise Exception('invalid constant type: %d' % const_type)

    for i in xrange(count_funcs):
        print('%sInner function %d:' % (ind, i))
        off = dump_function(buf, off, ind + '  ')

    val, = struct.unpack('>L', buf[off:off + 4])
    off += 4
    print('%s.length: %d' % (ind, val))
    off, val = decode_sanitize_string(buf, off)
    print('%s.name: %s' % (ind, val))
    off, val = decode_sanitize_string(buf, off)
    print('%s.fileName: %s' % (ind, val))
    off, val = decode_string(buf, off)  # actually a buffer
    print('%s._Pc2line: %s' % (ind, val.encode('hex')))

    while True:
        off, name = decode_string(buf, off)
        if name == '':
            break
        name = sanitize_string(name)
        val, = struct.unpack('>L', buf[off:off + 4])
        off += 4
        print('%s_Varmap[%s] = %d' % (ind, name, val))

    num_formals, = struct.unpack('>L', buf[off:off + 4])
    off += 4
    if num_formals != 0xffffffff:
        print('%s_Formals: %d formal arguments' % (ind, num_formals))
        for idx in xrange(num_formals):
            off, name = decode_string(buf, off)
            name = sanitize_string(name)
            print('%s_Formals[%d] = %s' % (ind, idx, name))
    else:
        print('%s_Formals: absent' % ind)

    return off


def dump_bytecode(buf, off, ind):
    sig, = struct.unpack('B', buf[off:off + 1])
    print('%sSignature byte: 0x%02x' % (ind, sig))
    off += 1
    if sig == 0xff:
        raise Exception('pre-Duktape 2.2 0xFF signature byte (signature byte is 0xBF since Duktape 2.2)')
    if sig != 0xbf:
        raise Exception('invalid signature byte: %d' % sig)

    off = dump_function(buf, off, ind + '  ')

    return off


def main():
    parser = optparse.OptionParser()
    parser.add_option('--hex-decode', dest='hex_decode', default=False, action='store_true',
                      help='Input file is ASCII hex encoded, decode before dump')
    (opts, args) = parser.parse_args()

    with open(args[0], 'rb') as f:
        d = f.read()
        if opts.hex_decode:
            d = d.strip()
            d = d.decode('hex')
    dump_bytecode(d, 0, '')


if __name__ == '__main__':
    main()
