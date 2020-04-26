======================
Date and time handling
======================

This document describes the Duktape ``Date`` built-in implementation.

Date/time handling is a major portability concern.  This document describes
how to implement an external Date provider and configure Duktape to use it
through the ``duk_config.h`` header.  An external Date provider allows Duktape
to be compiled on exotic platforms without change to Duktape internals.

Overview of ECMAScript date/time concepts
=========================================

ECMAScript time value
---------------------

An ECMAScript time value is essentially a UNIX/Posix (UTC) time value
measured in milliseconds without fractions:

* http://www.ecma-international.org/ecma-262/5.1/#sec-15.9.1.1
* http://en.wikipedia.org/wiki/Unix_time

A time value has a simple arithmetic relationship with UTC datetime (calendar)
values; leap years are taken into account but leap seconds are not (as a side
effect, when a leap second is inserted the ECMAScript time value conceptually
jumps backwards by one second).  This simple relationship allows easy, platform
independent conversion between the two representations.

The definition for a valid ECMAScript time value is very strict, and an
implementation is required to treat anything outside that range as an invalid
time value (NaN / "Invalid Date").  The valid time range is 100 million days
backwards and forwards from Jan 1, 1970.  The minimum and maximum values are::

  > new Date(-100e6 * 24 * 60 * 60 * 1000).toISOString()
  '-271821-04-20T00:00:00.000Z'
  > new Date(+100e6 * 24 * 60 * 60 * 1000).toISOString()
  '+275760-09-13T00:00:00.000Z'

No fractions may be present in the internal millisecond value.  Even if an
implementation maintained sub-millisecond time values, all return values
go through the internal ``TimeClip()`` algorithm which coerces the value
with ``ToInteger()``, so there is no standard way to access sub-millisecond
values.

Broken down datetime
--------------------

A datetime broken down into year, month, day-of-month, hour, minute, second,
millisecond, and weekday.  Components can be read or written with setter and
getter API calls.  Broken down datetime values can be UTC time or local time,
depending on the API call.

String representation: parsing and formatting
---------------------------------------------

The specification provides an ISO 8601 subset to provide a platform neutral
format for expressing date/time values as strings, and parsing them back
from strings.  Platform specific issues come into play when converting
between UTC time and local time, and when formatting or parsing date values
in additional platform specific formats.

To simplify, the parsing/formatting requirements are:

* The implementation is required to parse the ECMAScript ISO 8601 subset but
  may parse any other formats as well, including a larger ISO 8601 subset.

* The implementation is allowed to serialize time values into arbitrary
  strings, as long as it can parse them back into matching time values.
  (This is required only if milliseconds amount is zero; a reasonable
  implementation will still, of course, guarantee that other components
  are parsed back correctly.)

* ``toISOString()`` is required to use the ECMAScript ISO 8601 subset exactly,
  and the resulting string must parse back to the same time value (again, only
  technically required if milliseconds is zero).  This is the platform neutral
  string format which is guaranteed to work even across implementations.

External Date providers
=======================

External date provider vs. direct support
-----------------------------------------

Duktape provides built-in support for Unix/Windows date/time APIs and
should work out-of-the-box on these common platforms.  On more exotic
platforms you may need to either:

a. Implement an external "Date provider" and configure Duktape to use
   it through ``duk_config.h``.  You don't need to modify Duktape source.

b. Modify Duktape internals to support the new platform directly, which
   requires changes to Duktape source/headers.

Using an external Date provider is a good default to start with.  If the
target platform might interest other users, you can contribute both an
external Date provider (it can be packaged into Duktape examples/ directory)
and Duktape changes to support the target directly.

Implementing an external Date provider
--------------------------------------

To implement an external Date provider you must use the ``duk_config.h``
configuration model and provide the following config options:

* ``DUK_USE_DATE_GET_NOW``: mandatory, provides current ECMAScript time.

* ``DUK_USE_DATE_GET_LOCAL_TZOFFSET``: mandatory, provides offset between
  UTC and local time for a given timestamp.  Can always map to zero, i.e.
  pretend that local time matches UTC time.

* ``DUK_USE_DATE_PARSE_STRING``: optional, parse a platform specific string
  into ECMAScript time.

* ``DUK_USE_DATE_FORMAT_STRING``: optional, format ECMAScript time into a
  platform specific string.

You also need to make sure ``duk_config.h`` won't use any of the Duktape
built-in Date provider functions:

* Ensure ``DUK_USE_DATE_{NOW,TZO,PRS,FMT}_*`` defines are unset.

There's an example dummy provider in:

* ``examples/dummy-date-provider/``

You can also look into the Unix/Windows Date providers built into Duktape:

* ``src-input/duk_bi_date_unix.c``

* ``src-input/duk_bi_date_windows.c``

Platform dependencies
=====================

Porting requirements
--------------------

The minimum requirement for porting the Date implementation to a new
platform is:

* A function to get the current (UTC) time as an ECMAScript time value,
  preferably with a millisecond precision.

  - In many cases the current time can be obtained directly, as is the
    case with ``gettimeofday()`` for instance.

  - An implementation can also get a broken down datetime for the current
    UTC instant, and then use the ECMAScript timevalue conversion functions
    to convert it to an ECMAScript time value.  The conversion is entirely
    platform neutral, because the ECMAScript time model enforces a simple
    relationship between time values and calendar dates.

Without additional porting effort, string formatting and parsing will be
somewhat limited (but compliant), and the local time will always be UTC.
The following is thus very nice:

* A function to get the time offset between local time and UTC on a certain
  UTC instant.  The E5.1 specification has separate concepts for the local
  time zone adjustment (LocalTZA) and daylight saving time adjustment
  (DaylightSavingTA(t)).  The ECMAScript conversion semantics, especially
  with respect to handling of daylight savings, must be followed.

Finally, these are nice-to-have to provide support for Date string formats
other than ISO 8601 (which is always supported):

* A function to format a Date value in a platform dependent manner.

* A function to parse a Date value from a string in a platform dependent
  manner.

Platform specific formatting and parsing
----------------------------------------

The current approach to using platform specific formatting/parsing APIs is
as follows:

* The primary requirement is to provide a portable base implementation which
  is as platform neutral as possible.  Timestamps can be formatted in a ISO
  8601-like manner, and local time can be assumed to be UTC if no timezone
  and/or DST information is available.

* Platform specific local time and locale mechanisms can be used, as long as
  they don't restrict the ECMAScript time range.  For instance, if the valid
  platform datetime range is smaller than ECMAScript's, the implementation
  must either fall back to default handling if the range is exceeded, or
  extrapolate in a reasonable manner.

The ECMAScript valid datetime range is huge, and may be larger than what the
underlying platform supports.  This poses challenges to detect e.g. daylight
savings time reliably.  For instance, if the platform has a Y2038 limit, how
does one query for daylight savings time for the year 200000?

The E5.1 specification provides explicit guidance for this; Section 15.9.1.8:

  If the host environment provides functionality for determining daylight
  saving time, the implementation of ECMAScript is free to map the year in
  question to an equivalent year (same leap-year-ness and same starting week
  day for the year) for which the host environment provides daylight saving
  time information. The only restriction is that all equivalent years should
  produce the same result.

However, the equivalent year mapping approach is not necessarily preferred
in the long term see e.g. the following discussion:

* https://bugzilla.mozilla.org/show_bug.cgi?id=351066

Note that using a platform specific API to get timezone offset and DST
information makes programs behave slightly differently across platforms, even
when they are running with the same locale.  There's no way around this
unless the locale information needed by Duktape is provided by a portable
or pluggable provider (e.g. user callback for tzoffset/DST information).

Linux
-----

Current implementation uses:

* ``gettimeofday()``
* ``strptime()``
* ``strftime()``

APIs available for formatting datetime values:

* ``ctime_r()``
* ``asctime_r()``
* ``strftime()``

APIs available for parsing datetime values:

* ``strptime()``: quite portable, but requires an explicit format string
* ``getdate_r()``: GNU specific, more generic, but requires ``DATEMSK`` to be set

See also:

* http://www.gnu.org/software/libc/manual/html_node/Date-and-Time.html#Date-and-Time

macOS / Darwin
--------------

Current implementation uses the same functions as on Linux.

Windows
-------

Current implementation uses time functions documented in:

* http://msdn.microsoft.com/en-us/library/windows/desktop/ms725473(v=vs.85).aspx

The same implementation works for WIN32 and WIN64.

See also:

* http://www.suacommunity.com/dictionary/gettimeofday-entry.php

Parsing the E5 ISO 8601 subset
==============================

E5.1 Section 15.9.1.15 describes the subset, with the following
possible parts::

     YYYY         T       HH:mm           empty
     YYYY-MM              HH:mm:ss        Z
     YYYY-MM-DD           HH:mm:ss.sss    +HH:mm
  +YYYYYY                                 -HH:mm
  +YYYYYY-MM
  +YYYYYY-MM-DD
  -YYYYYY
  -YYYYYY-MM                                ^
  -YYYYYY-MM-DD                             |
                                            |
        |   may skip time part              |
        `-----------------------------------'

A valid date time string may contain only a date part or both a
date and a time part, followed by an optional timezone part.  A
missing timezone is interpreted the same as a 'Z'.

An implementation is allowed to parse a wider set of strings, so
an implementation can actually be made simpler by checking the input
format less rigidly.  Some reasonable relaxations:

* Allow an arbitrary number of digits for any date part, including leading
  zeroes.  Millisecond digits after the third one can be ignored (which is
  the same as truncation towards zero).

* Allow year to be signed regardless of the number of year digits.

* Allow date/time separator to be a space in addition to 'T'.

* Allow a timezone offset to be specified without colon (e.g. ``+1234``
  in addition to ``+12:34``).

* Allow unnormalized components.  In fact, the specification actually
  requires accepting these two as equivalent: ``1995-02-04T24:00`` and
  ``1995-02-05T00:00``.  Other unnormalized cases could be accepted too,
  like ``1995-02-123T11:2345:99``.

* Allow whitespace in additional places; in particular, before and after
  the string.

V8 seems to relax the rules if the date/time separator is a space but will
be strict if the separator is 'T'::

  > new Date('+0001979-0001-0000002T00003:0004:00005.006123123Z').toISOString()
  RangeError: Invalid time value

  > new Date('+0001979-0001-0000002 00003:0004:00005.006123123Z').toISOString()
  '1979-01-02T03:04:05.006Z'

  > new Date('  +0001979-0001-0000002 00003:0004:00005.006123123 +01:00  ').toISOString()
  '1979-01-02T02:04:05.006Z'

Some options for implementation a compact parser:

* Use an internal regexp to match the parts, then convert them to integers
  (accepting leading zeroes).

* Use a set of partial ``sscanf()`` calls.

* Use a custom char-by-char parser.

With a relaxed format a custom char-by-char parser is relatively simple and
is the current implementation approach:

1. Strip the input string (remove leading and trailing whitespace).
   (Currently not done.)

2. Initialize a broken down timestamp with default values.  Initialize
   part_index to 0.  Check first character to handle year sign.

3. Parse a decimal number of 1...n digits.  When it is finished, write it
   to part_index.

4. Check the next character to determine what to do next: update part_index
   (either by one or skip directly to "hour" part) and parse next part,
   or accept/reject.  The separator for timezone offset may be '+' or '-',
   which needs to be recorded.

5. If accepted, subtract timezone hours and minutes from the hours and
   minutes part (to convert to UTC), and then convert the (possibly
   unnormalized) components into an ECMAScript time value.

The parser will produce the following "parts":

* Year, default: 1970 (actually arbitrary, because a year is always required)
* Month, default: 1
* Day-of-month, default: 1
* Hour, default: 0
* Minute, default: 0
* Second, default: 0
* Millisecond, default: 0
* Timezone hours, default: 0
* Timezone seconds, default: 0

The current implementation is a rule-driven parser based on this basic model.

Misc notes
==========

* Almost all API calls require a Date instance as the 'this' binding
  (a TypeError is thrown otherwise).  Exceptions are noted in the
  specification; concretely, ``toJSON()``.

* The internal time value always exists for a Date instance, and is
  always a number.  The number value is either NaN, or a finite number
  in the valid E5 range, with no millisecond fractions.  The internal
  component representation uses zero-based day and month, while ECMAScript
  API uses one-based day and zero-based month.

* When the internal time value is broken into components, each
  component will be normalized, and will fit into a 32-bit signed
  integer.  When using setter calls, one or more components are replaced
  with unnormalized values, which will not necessarily fit into a 32-bit
  signed integer, before converting back to an internal time value.  The
  setter values may be huge (even out of 64-bit range) without resulting
  in an invalid result date, if multiple cancelling values are given
  (e.g. 1e100 seconds and -1e103 milliseconds, cancelling to zero).

* Setters and getters are optimized for size, to use a single helper with a
  set of flags and arguments to keep each getter and setter itself very small.
  This makes them a bit cryptic; see e.g. handling of setters with optional
  parameters.
