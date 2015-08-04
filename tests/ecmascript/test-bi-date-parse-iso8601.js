/*===
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
-23215049510877 -23215049511000 -23215049520000 -23215075200000 -23215507200000 -23225875200000
-23215049510877 -23215049511000 -23215049520000 -23215075200000 -23215507200000 -23225875200000
1262401445006 1325473445006 1325432525006 1325514365006
791942400000 791942400000
NaN -62198755200000 -62167219200000 -62135596800000 0 1260835200000 NaN
275760 NaN -271821 NaN
-3217862321754994 6249223278245006
NaN
NaN
===*/

/* ISO 8601 form parsing tests. */

function iso8601ParseTest() {
    // Note: the following are not valid abbreviations but since an
    // implementation is allowed to parse them anyway, they should
    // not be negative tests here.
    //
    // Date.parse('1970-01-01T00Z')
    // Date.parse('1970-01-01T00')
    // Date.parse('1234-05-06T07')
    //
    // Similarly, out-of-range values are not allowed in the E5.1
    // ISO 8601 subset, but an implementation can parse them anyway
    // (like we do).
    //
    // Date.parse('2001-02-03T04:05:06.007Z')
    // Date.parse('2001-02-03T04:05:60.007Z')
    // Date.parse('2001-02-03T04:60:06.007Z')
    // Date.parse('2001-02-03T25:05:06.007Z')
    // Date.parse('2001-02-29T04:05:06.007Z')
    // Date.parse('2001-13-03T04:05:06.007Z')
    //
    // Same format issue with milliseconds: we parse non-standard
    // millisecond counts too.
    //
    // Date.parse('2001-02-03T04:05:06Z')
    // Date.parse('2001-02-03T04:05:06.123Z')
    // Date.parse('2001-02-03T04:05:06.12Z')
    // Date.parse('2001-02-03T04:05:06.1Z')
    // Date.parse('2001-02-03T04:05:06.1234Z')
    //
    // Same format issue for year 0; extended format must have plus sign
    // but we parse either.
    //
    // Date.parse('+000000-01-02T03:04:05.006Z')
    // Date.parse('-000000-01-02T03:04:05.006Z')


    // 1970-01-01, some combinations
    print(Date.parse('1970-01-01T00:00:00.000Z'),
          Date.parse('1970-01-01T00:00:00.000'),
          Date.parse('1970-01-01T00:00:00Z'),
          Date.parse('1970-01-01T00:00:00'),
          Date.parse('1970-01-01T00:00Z'),
          Date.parse('1970-01-01T00:00'),
          Date.parse('1970-01-01Z'),
          Date.parse('1970-01-01'),
          Date.parse('1970-01Z'),
          Date.parse('1970-01'),
          Date.parse('1970Z'),
          Date.parse('1970'),
          Date.parse('1970T00:00Z'),
          Date.parse('1970T00:00'),
          Date.parse('1970T00:00:00Z'),
          Date.parse('1970T00:00:00'),
          Date.parse('1970T00:00:00.000Z'),
          Date.parse('1970T00:00:00.000'),
          Date.parse('1970-01T00:00Z'),
          Date.parse('1970-01T00:00'),
          Date.parse('1970-01T00:00:00Z'),
          Date.parse('1970-01T00:00:00'),
          Date.parse('1970-01T00:00:00.000Z'),
          Date.parse('1970-01T00:00:00.000'));

    // 1234-05-06T07:08:09.123Z; check component defaulting
    print(Date.parse('1234-05-06T07:08:09.123Z'),
          Date.parse('1234-05-06T07:08:09'),
          Date.parse('1234-05-06T07:08Z'),
          Date.parse('1234-05-06'),
          Date.parse('1234-05'),
          Date.parse('1234'));
    print(Date.parse('1234-05-06T07:08:09.123Z'),  // should default to these
          Date.parse('1234-05-06T07:08:09.000Z'),
          Date.parse('1234-05-06T07:08:00.000Z'),
          Date.parse('1234-05-06T00:00:00.000Z'),
          Date.parse('1234-05-01T00:00:00.000Z'),
          Date.parse('1234-01-01T00:00:00.000Z'));

    // Timezone part may be missing (interpreted as UTC), 'Z' (UTC), or
    // given as a +/- HH:mm offset.
    print(Date.parse('2010-01-02T03:04:05.006'),
          Date.parse('2012-01-02T03:04:05.006Z'),
          Date.parse('2012-01-02T03:04:05.006+11:22'),
          Date.parse('2012-01-02T03:04:05.006-11:22'));

    // E5.1 Section 15.9.1.15 NOTE 1
    print(Date.parse('1995-02-04T24:00'), Date.parse('1995-02-05T00:00'));

    // Extended years (from E5.1 spec)
    print(Date.parse('-283457-03-21T15:00:00.008Z'),  // out of valid E5 range
          Date.parse('-000001-01-01T00:00:00Z'),
          Date.parse('+000000-01-01T00:00:00Z'),
          Date.parse('+000001-01-01T00:00:00Z'),
          Date.parse('+001970-01-01T00:00:00Z'),
          Date.parse('+002009-12-15T00:00:00Z'),
          Date.parse('+287396-10-12T08:59:00.992Z'));  // out of valid E5 range

    // Note that the first and last extended year examples are out of
    // Ecmascript valid range:
    print(new Date(0 + 100e6*24*3600*1000).getUTCFullYear(),
          new Date(0 + 100e6*24*3600*1000 + 1).getUTCFullYear(),
          new Date(0 - 100e6*24*3600*1000).getUTCFullYear(),
          new Date(0 - 100e6*24*3600*1000 - 1).getUTCFullYear());

    // So, test some 6-digit years that are actually valid
    print(Date.parse('-100000-01-02T03:04:05.006Z'),
          Date.parse('+200000-01-02T03:04:05.006Z'));

    // If Date.parse() argument does not conform to the ISO 8601 subset
    // in the E5.1 specification, the implementation may fall back to
    // custom parsing.  However, test obviously broken cases.
    print(Date.parse(''));
    print(Date.parse('foo'));
}

try {
    iso8601ParseTest();
} catch (e) {
    print(e.name);
}
