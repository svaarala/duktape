#!/usr/bin/env python
"""
Fix trailing whitespace and line endings (to Unix) in a file.
Usage: python fix_whitespace.py foo.py
"""

import os
import sys
import optparse

def main():
	parser = optparse.OptionParser()
	parser.add_option('-d',  default=[], action='append', dest='file_path', type='string', help='Specify Folder to Test Files')
	parser.add_option('--ext',  default=[], action='append', dest='file_extensions', type='string', help='Specify File Extensions')
	(opts, args) = parser.parse_args()
	if(0 != len(opts.file_path) and 0 != len(opts.file_extensions) ):
		totalProblems = 0;
		for filepath in opts.file_path:
			mFilePath = os.path.join('', filepath)
			for root, dirs, files in os.walk(filepath):
				for file in files:
					filename = os.path.join(root, file)
					for Ext in opts.file_extensions:
						if file.endswith(Ext):
							fix_whitespace(filename)


def fix_whitespace(fname):
    """ Fix whitespace in a file """
    with open(fname, "rb") as fo:
        original_contents = fo.read()
    # "rU" Universal line endings to Unix
    with open(fname, "rU") as fo:
        contents = fo.read()
    lines = contents.split("\n")
    fixed = 0
    for k, line in enumerate(lines):
        new_line = line.rstrip()
        if len(line) != len(new_line):
            lines[k] = new_line
            fixed += 1
    with open(fname, "wb") as fo:
        fo.write("\n".join(lines))
    if fixed or contents != original_contents:
        print("************* %s" % os.path.basename(fname))
    if fixed:
        slines = "lines" if fixed > 1 else "line"
        print("Fixed trailing whitespace on %d %s" \
              % (fixed, slines))
    if contents != original_contents:
        print("Fixed line endings to Unix (\\n)")


if __name__ == "__main__":
    main()