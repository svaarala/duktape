#!/usr/bin/env python2
#
#  Extract rules for Unicode case conversion, specifically the behavior
#  required by Ecmascript E5 in Sections 15.5.4.16 to 15.5.4.19.  The
#  bitstream encoded rules are used for the slow path at run time, so
#  compactness is favored over speed.
#
#  There is no support for context or locale sensitive rules, as they
#  are handled directly in C code before consulting tables generated
#  here.  Ecmascript requires case conversion both with and without
#  locale/language specific rules (e.g. String.prototype.toLowerCase()
#  and String.prototype.toLocaleLowerCase()), so they are best handled
#  in C anyway.
#
#  Case conversion rules for ASCII are also excluded as they are handled
#  by the C fast path.  Rules for non-BMP characters (codepoints above
#  U+FFFF) are omitted as they're not required for standard Ecmascript.
#

import os
import sys
import math
import optparse

import dukutil

class UnicodeData:
    "Read UnicodeData.txt into an internal representation."

    def __init__(self, filename):
        self.data = self.read_unicode_data(filename)
        print 'read %d unicode data entries' % len(self.data)

    def read_unicode_data(self, filename):
        res = []
        f = open(filename, 'rb')
        for line in f:
            if line.startswith('#'):
                continue
            line = line.strip()
            if line == '':
                continue
            parts = line.split(';')
            if len(parts) != 15:
                raise Exception('invalid unicode data line')
            res.append(parts)
        f.close()

        # Sort based on Unicode codepoint
        def mycmp(a,b):
            return cmp(long(a[0], 16), long(b[0], 16))

        res.sort(cmp=mycmp)
        return res

class SpecialCasing:
    "Read SpecialCasing.txt into an internal representation."

    def __init__(self, filename):
        self.data = self.read_special_casing_data(filename)
        print 'read %d special casing entries' % len(self.data)

    def read_special_casing_data(self, filename):
        res = []
        f = open(filename, 'rb')
        for line in f:
            try:
                idx = line.index('#')
                line = line[:idx]
            except ValueError:
                pass
            line = line.strip()
            if line == '':
                continue
            parts = line.split(';')
            parts = [i.strip() for i in parts]
            while len(parts) < 6:
                parts.append('')
            res.append(parts)
        f.close()
        return res

def parse_unicode_sequence(x):
    res = ''
    for i in x.split(' '):
        i = i.strip()
        if i == '':
            continue
        res += unichr(long(i, 16))
    return res

def get_base_conversion_maps(unicode_data):
    "Create case conversion tables without handling special casing yet."

    uc = {}        # codepoint (number) -> string
    lc = {}
    tc = {}         # titlecase

    for x in unicode_data.data:
        c1 = long(x[0], 16)

        # just 16-bit support needed
        if c1 >= 0x10000:
            continue

        if x[12] != '':
            # field 12: simple uppercase mapping
            c2 = parse_unicode_sequence(x[12])
            uc[c1] = c2
            tc[c1] = c2    # titlecase default == uppercase, overridden below if necessary
        if x[13] != '':
            # field 13: simple lowercase mapping
            c2 = parse_unicode_sequence(x[13])
            lc[c1] = c2
        if x[14] != '':
            # field 14: simple titlecase mapping
            c2 = parse_unicode_sequence(x[14])
            tc[c1] = c2

    return uc, lc, tc

def update_special_casings(uc, lc, tc, special_casing):
    "Update case conversion tables with special case conversion rules."

    for x in special_casing.data:
        c1 = long(x[0], 16)

        if x[4] != '':
            # conditions
            continue

        lower = parse_unicode_sequence(x[1])
        title = parse_unicode_sequence(x[2])
        upper = parse_unicode_sequence(x[3])

        if len(lower) > 1:
            lc[c1] = lower
        if len(upper) > 1:
            uc[c1] = upper
        if len(title) > 1:
            tc[c1] = title

        print 'special case: %d %d %d' % (len(lower), len(upper), len(title))

def remove_ascii_part(convmap):
    "Remove ASCII case conversion parts (handled by C fast path)."

    for i in xrange(128):
        if convmap.has_key(i):
            del convmap[i]

def scan_range_with_skip(convmap, start_idx, skip):
    "Scan for a range of continuous case conversion with a certain 'skip'."

    conv_i = start_idx
    if not convmap.has_key(conv_i):
        return None, None, None
    elif len(convmap[conv_i]) > 1:
        return None, None, None
    else:
        conv_o = ord(convmap[conv_i])

    start_i = conv_i
    start_o = conv_o

    while True:
        new_i = conv_i + skip
        new_o = conv_o + skip

        if not convmap.has_key(new_i):
            break
        if len(convmap[new_i]) > 1:
            break
        if ord(convmap[new_i]) != new_o:
            break

        conv_i = new_i
        conv_o = new_o

    # [start_i,conv_i] maps to [start_o,conv_o], ignore ranges of 1 char
    count = (conv_i - start_i) / skip + 1
    if count <= 1:
        return None, None, None

    # we have an acceptable range, remove them from the convmap here
    for i in xrange(start_i, conv_i + skip, skip):
        del convmap[i]

    return start_i, start_o, count

def find_first_range_with_skip(convmap, skip):
    "Find first range with a certain 'skip' value."

    for i in xrange(65536):
        start_i, start_o, count = scan_range_with_skip(convmap, i, skip)
        if start_i is None:
            continue
        return start_i, start_o, count

    return None, None, None

def generate_tables(convmap):
    "Generate bit-packed case conversion table for a given conversion map."

    # The bitstream encoding is based on manual inspection for whatever
    # regularity the Unicode case conversion rules have.
    #
    # Start with a full description of case conversions which does not
    # cover all codepoints; unmapped codepoints convert to themselves.
    # Scan for range-to-range mappings with a range of skips starting from 1.
    # Whenever a valid range is found, remove it from the map.  Finally,
    # output the remaining case conversions (1:1 and 1:n) on a per codepoint
    # basis.
    #
    # This is very slow because we always scan from scratch, but its the
    # most reliable and simple way to scan

    ranges = []        # range mappings (2 or more consecutive mappings with a certain skip)
    singles = []       # 1:1 character mappings
    multis = []        # 1:n character mappings

    # Ranges with skips

    for skip in xrange(1,6+1):    # skips 1...6 are useful
        while True:
            start_i, start_o, count = find_first_range_with_skip(convmap, skip)
            if start_i is None:
                break
            print 'skip %d: %d %d %d' % (skip, start_i, start_o, count)
            ranges.append([start_i, start_o, count, skip])

    # 1:1 conversions

    k = convmap.keys()
    k.sort()
    for i in k:
        if len(convmap[i]) > 1:
            continue
        singles.append([i, ord(convmap[i])])    # codepoint, codepoint
        del convmap[i]

    # There are many mappings to 2-char sequences with latter char being U+0399.
    # These could be handled as a special case, but we don't do that right now.
    #
    # [8064L, u'\u1f08\u0399']
    # [8065L, u'\u1f09\u0399']
    # [8066L, u'\u1f0a\u0399']
    # [8067L, u'\u1f0b\u0399']
    # [8068L, u'\u1f0c\u0399']
    # [8069L, u'\u1f0d\u0399']
    # [8070L, u'\u1f0e\u0399']
    # [8071L, u'\u1f0f\u0399']
    # ...
    #
    # tmp = {}
    # k = convmap.keys()
    # k.sort()
    # for i in k:
    #    if len(convmap[i]) == 2 and convmap[i][1] == u'\u0399':
    #        tmp[i] = convmap[i][0]
    #        del convmap[i]
    # print repr(tmp)
    #
    # skip = 1
    # while True:
    #    start_i, start_o, count = find_first_range_with_skip(tmp, skip)
    #    if start_i is None:
    #        break
    #    print 'special399, skip %d: %d %d %d' % (skip, start_i, start_o, count)
    # print len(tmp.keys())
    # print repr(tmp)
    # XXX: need to put 12 remaining mappings back to convmap

    # 1:n conversions

    k = convmap.keys()
    k.sort()
    for i in k:
        multis.append([i, convmap[i]])        # codepoint, string
        del convmap[i]

    for t in singles:
        print repr(t)

    for t in multis:
        print repr(t)

    print 'range mappings: %d' % len(ranges)
    print 'single character mappings: %d' % len(singles)
    print 'complex mappings (1:n): %d' % len(multis)
    print 'remaining (should be zero): %d' % len(convmap.keys())

    # XXX: opportunities for diff encoding skip=3 ranges?
    prev = None
    for t in ranges:
        # range: [start_i, start_o, count, skip]
        if t[3] != 3:
            continue
        if prev is not None:
            print '%d %d' % (t[0] - prev[0], t[1] - prev[1])
        else:
            print 'start: %d %d' % (t[0], t[1])
        prev = t

    # bit packed encoding

    be = dukutil.BitEncoder()

    for curr_skip in xrange(1, 7):    # 1...6
        count = 0
        for r in ranges:
            start_i, start_o, r_count, skip = r[0], r[1], r[2], r[3]
            if skip != curr_skip:
                continue
            count += 1
        be.bits(count, 6)
        print 'encode: skip=%d, count=%d' % (curr_skip, count)

        for r in ranges:
            start_i, start_o, r_count, skip = r[0], r[1], r[2], r[3]
            if skip != curr_skip:
                continue
            be.bits(start_i, 16)
            be.bits(start_o, 16)
            be.bits(r_count, 7)
    be.bits(0x3f, 6)    # maximum count value = end of skips

    count = len(singles)
    be.bits(count, 7)
    for t in singles:
        cp_i, cp_o = t[0], t[1]
        be.bits(cp_i, 16)
        be.bits(cp_o, 16)

    count = len(multis)
    be.bits(count, 7)
    for t in multis:
        cp_i, str_o = t[0], t[1]
        be.bits(cp_i, 16)
        be.bits(len(str_o), 2)
        for i in xrange(len(str_o)):
            be.bits(ord(str_o[i]), 16)

    return be.getBytes(), be.getNumBits()

def generate_regexp_canonicalize_lookup(convmap):
    res = []

    highest_nonid = -1

    for cp in xrange(65536):
        res_cp = cp  # default to as is
        if convmap.has_key(cp):
            tmp = convmap[cp]
            if len(tmp) == 1:
                # Multiple codepoints from input, ignore
                res_cp = ord(tmp[0])
        if cp >= 0x80 and res_cp < 0x80:
            res_cp = cp  # non-ASCII mapped to ASCII, ignore

        if cp != res_cp:
            highest_nonid = cp

        res.append(res_cp)

    # At the moment this is 65370, which means there's very little
    # gain in assuming 1:1 mapping above a certain BMP codepoint.
    print('HIGHEST NON-ID MAPPING: %d' % highest_nonid)
    return res

def clonedict(x):
    "Shallow clone of input dict."
    res = {}
    for k in x.keys():
        res[k] = x[k]
    return res

def main():
    parser = optparse.OptionParser()
    parser.add_option('--command', dest='command', default='caseconv_bitpacked')
    parser.add_option('--unicode-data', dest='unicode_data')
    parser.add_option('--special-casing', dest='special_casing')
    parser.add_option('--out-source', dest='out_source')
    parser.add_option('--out-header', dest='out_header')
    parser.add_option('--table-name-lc', dest='table_name_lc', default='caseconv_lc')
    parser.add_option('--table-name-uc', dest='table_name_uc', default='caseconv_uc')
    parser.add_option('--table-name-re-canon-lookup', dest='table_name_re_canon_lookup', default='caseconv_re_canon_lookup')
    (opts, args) = parser.parse_args()

    unicode_data = UnicodeData(opts.unicode_data)
    special_casing = SpecialCasing(opts.special_casing)

    uc, lc, tc = get_base_conversion_maps(unicode_data)
    update_special_casings(uc, lc, tc, special_casing)

    if opts.command == 'caseconv_bitpacked':
        # XXX: ASCII and non-BMP filtering could be an option but is now hardcoded

        # ascii is handled with 'fast path' so not needed here
        t = clonedict(uc)
        remove_ascii_part(t)
        uc_bytes, uc_nbits = generate_tables(t)

        t = clonedict(lc)
        remove_ascii_part(t)
        lc_bytes, lc_nbits = generate_tables(t)

        # Generate C source and header files
        genc = dukutil.GenerateC()
        genc.emitHeader('extract_caseconv.py')
        genc.emitArray(uc_bytes, opts.table_name_uc, size=len(uc_bytes), typename='duk_uint8_t', intvalues=True, const=True)
        genc.emitArray(lc_bytes, opts.table_name_lc, size=len(lc_bytes), typename='duk_uint8_t', intvalues=True, const=True)
        f = open(opts.out_source, 'wb')
        f.write(genc.getString())
        f.close()

        genc = dukutil.GenerateC()
        genc.emitHeader('extract_caseconv.py')
        genc.emitLine('extern const duk_uint8_t %s[%d];' % (opts.table_name_uc, len(uc_bytes)))
        genc.emitLine('extern const duk_uint8_t %s[%d];' % (opts.table_name_lc, len(lc_bytes)))
        f = open(opts.out_header, 'wb')
        f.write(genc.getString())
        f.close()
    elif opts.command == 're_canon_lookup':
        # direct canonicalization lookup for case insensitive regexps, includes ascii part
        t = clonedict(uc)
        re_canon_lookup = generate_regexp_canonicalize_lookup(t)

        genc = dukutil.GenerateC()
        genc.emitHeader('extract_caseconv.py')
        genc.emitArray(re_canon_lookup, opts.table_name_re_canon_lookup, size=len(re_canon_lookup), typename='duk_uint16_t', intvalues=True, const=True)
        f = open(opts.out_source, 'wb')
        f.write(genc.getString())
        f.close()

        genc = dukutil.GenerateC()
        genc.emitHeader('extract_caseconv.py')
        genc.emitLine('extern const duk_uint16_t %s[%d];' % (opts.table_name_re_canon_lookup, len(re_canon_lookup)))
        f = open(opts.out_header, 'wb')
        f.write(genc.getString())
        f.close()
    else:
        raise Exception('invalid command: %r' % opts.command)

if __name__ == '__main__':
    main()
