#!/usr/bin/python
#
#  Generate equivalent year table needed by duk_bi_date.c.  Based on:
#
#    http://code.google.com/p/v8/source/browse/trunk/src/date.h#146
#

import datetime
import pytz

def isleapyear(year):
	if (year % 4) != 0:
		return False
	if (year % 100) != 0:
		return True
	if (year % 400) != 0:
		return False
	return True

def eqyear(weekday, isleap):
	# weekday: 0=Sunday, 1=Monday, ...

	if isleap:
		recent_year = 1956
	else:
		recent_year = 1967
	recent_year += (weekday * 12) % 28
	year = 2008 + (recent_year + 3 * 28 - 2008) % 28

	# some assertions
	#
	# Note that Ecmascript internal weekday (0=Sunday) matches neither
	# Python weekday() (0=Monday) nor isoweekday() (1=Monday, 7=Sunday).
	# Python isoweekday() % 7 matches the Ecmascript weekday.
	# https://docs.python.org/2/library/datetime.html#datetime.date.isoweekday

	dt = datetime.datetime(year, 1, 1, 0, 0, 0, 0, pytz.UTC)  # Jan 1 00:00:00.000 UTC
	#print(weekday, isleap, year, dt.isoweekday(), isleapyear(year))
	#print(repr(dt))
	#print(dt.isoformat())

	if isleap != isleapyear(year):
		raise Exception('internal error: equivalent year does not have same leap-year-ness')
		pass

	if weekday != dt.isoweekday() % 7:
		raise Exception('internal error: equivalent year does not begin with the same weekday')
		pass

	return year

def main():
	for i in xrange(14):
		print(eqyear(i % 7, i >= 7))

if __name__ == '__main__':
	main()
