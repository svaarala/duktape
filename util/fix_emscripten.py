#!/usr/bin/env python2
#
#  Fix a few errors from Emscripten output (stopgap until emscripten mainline
#  is updated).
#
#  NOTE: For Duktape 1.5.0 no fixups are needed anymore, at least at the time of
#  writing (the situation may of course change at a later time).

import os
import sys

fix_count = 0

replacements = {
    # RegExp fixes for non-compliant regexps (typically literal brace
    # without a backslash escape).  These fixes are no longer needed
    # with Duktape 1.5.0 which adds support for parsing non-standard
    # regexp curly braces.
    #r"""if (/<?{ ?[^}]* ?}>?/.test(type)) return true""":
    #    r"""if (/<?\{ ?[^}]* ?\}>?/.test(type)) return true""",
    #r"""var sourceRegex = /^function\s\(([^)]*)\)\s*{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?}$/;""":
    #    r"""var sourceRegex = /^function\s\(([^)]*)\)\s*\{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?\}$/;""",
    #r"""var sourceRegex = /^function\s*\(([^)]*)\)\s*{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?}$/;""":
    #    r"""var sourceRegex = /^function\s*\(([^)]*)\)\s*\{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?\}$/;""",
    #r"""/^function\s*\(([^)]*)\)\s*{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?}$/""":
    #    r"""/^function\s*\(([^)]*)\)\s*\{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?\}$/""",

    # GH-11: Attempt to parse a function's toString() output with a RegExp.
    # The RegExp assumes more of toString() output format than what is
    # guaranteed by the specification, and won't parse Duktape 1.4.0 (and
    # before) function toString() output ("function empty() {/* source code*/)}").
    # No longer needed in Duktape 1.5.0 which changed the .toString() format.
    #r"""var parsed = jsfunc.toString().match(sourceRegex).slice(1);""":
    #    r"""var parsed = (jsfunc.toString().match(sourceRegex) || []).slice(1);""",
    #r"""jsfunc.toString().match(sourceRegex).slice(1);""":
    #    r"""(jsfunc.toString().match(sourceRegex) || []).slice(1);""",
}

repl_keys = replacements.keys()
repl_keys.sort()

for line in sys.stdin:
    if len(line) > 1 and line[-1] == '\n':
        line = line[:-1]

        for k in repl_keys:
            line_fix = line.replace(k, replacements[k])
            if line_fix != line:
                fix_count += 1
            line = line_fix

        print(line)

if fix_count > 0:
    sys.stderr.write('Emscripten fixes needed (fix_emscripten.py): fix_count=%d\n' % fix_count)
    sys.stderr.flush()
