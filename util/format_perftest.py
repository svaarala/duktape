#!/usr/bin/env python2
#
#  Format a perftest text dump into a HTML table.
#

import os
import sys
import re

def main():
    # test-try-catch-throw.js       : duk.O2.alt0 40.70 duk.O2.alt0f 40.74 duk.O2.alt1 40.10 duk.O2.alt1a 39.91 duk.O2.alt2 40.10 duk.O2.alt3 39.77 duk.O2.master 40.01 duk.O2.130 38.08

    re_line = re.compile(r'^(\S+)\s*:\s*(.*?)$')
    re_part = re.compile(r'\S+')

    colors = True

    headings = []
    testnames = []
    results = []
    baseline = []

    # Column index (positive or negative) for baseline engine (after filtering).
    baseline_column = 4

    with open(sys.argv[1], 'rb') as f_in, open(sys.argv[2], 'wb') as f_out:
        for line in f_in:
            line = line.strip()
            m = re_line.match(line)
            if m is None:
                continue

            testname = m.group(1)
            testnames.append(testname)

            parts = re_part.findall(m.group(2))

            if len(headings) == 0:
                for idx in xrange(0, len(parts), 2):
                    headings.append(parts[idx])

            result = []
            for idx in xrange(1, len(parts), 2):
                try:
                    result.append(float(parts[idx]))
                except ValueError:
                    result.append(None)
            results.append(result)

        filter_columns = [ 3, 4, 5, 6, 7, 8, 9, 10 ]
        def do_filter(val):
            res = []
            for i in filter_columns:
                res.append(val[i])
            return res

        #headings = do_filter(headings)
        #results = [ do_filter(x) for x in results ]

        baseline = [ x[baseline_column] for x in results ]

        #print(repr(headings))
        #print(repr(results))

        f_out.write('<!DOCTYPE html>\n')
        f_out.write('<html>\n')
        f_out.write('<head>\n')
        f_out.write("""\
<style>
th, td { margin: 0; padding: 6pt; text-align: right; }
tr:nth-child(odd) { background: #eeeeee; }
</style>
""")
        f_out.write('</head>\n')
        f_out.write('<body>\n')
        f_out.write('<table>\n')

        f_out.write('<tr>')
        f_out.write('<th></th>')
        for h in headings:
            f_out.write('<th>' + h + '</th>')
        f_out.write('</tr>\n')

        for idx,result in enumerate(results):
            f_out.write('<tr>')
            f_out.write('<td>' + testnames[idx] + '</td>')
            for column,t in enumerate(result):
                wraptag = None
                wrapchars = None
                icon = None

                style = 'background-color: #ffffff'

                if baseline[idx] is not None and baseline[idx] > 0 and t is not None:
                    factor = t / baseline[idx]
                    if factor < 0.90:
                        style = 'background-color: #88ff88; font-weight: bold'
                        wraptag = 'strong'
                        icon = '&#9650;'
                    elif factor < 0.93:
                        style = 'background-color: #ddffdd'
                        wraptag = 'strong'
                        icon = '&#8657;'
                    elif factor < 0.97:
                        style = 'background-color: #eeffee'
                    elif factor > 1.10:
                        style = 'background-color: #ff8888; font-weight: bold'
                        wraptag = 'em'
                        wrapchars = '()'
                        icon = '&#9660;'
                    elif factor > 1.07:
                        style = 'background-color: #ffdddd'
                        wraptag = 'em'
                        wrapchars = '()'
                        icon = '&#8659;'
                    elif factor > 1.03:
                        style = 'background-color: #ffeeee'
                        wrapchars = '()'
                    else:
                        pass

                if column == baseline_column:
                    style = 'background-color: #eeeeee'
                #if column not in [ 3, 4 ]:
                #    style = 'background-color: #eeeeee'

                if t is None:
                    text = '-'
                else:
                    text = '%.2f' % t

                # style doesn't survive in GFM; em/strong does
                if wrapchars is not None:
                    text = '%s%s%s' % (wrapchars[0], text, wrapchars[1])
                if wraptag is not None:
                    text = '<%s>%s</%s>' % (wraptag, text, wraptag)
                if icon is not None:
                    text = '%s %s' % (text, icon)

                f_out.write('<td style="%s">%s</td>' % (style, text))
            f_out.write('</tr>\n')

        f_out.write('</table>\n')
        f_out.write('</body>\n')
        f_out.write('</html>\n')

if __name__ == '__main__':
    main()
