#!/usr/bin/env python2
#
#  Generate a size report from a Duktape library / executable.
#  Write out useful information about function sizes in a variety
#  of forms.
#

import os
import sys
import re
import subprocess

#000000000040d200 <duk_to_hstring>:
#  40d200:       55                      push   %rbp
#  40d201:       89 f5                   mov    %esi,%ebp

re_funcstart = re.compile(r'^[0-9a-fA-F]+\s<(.*?)>:$')
re_codeline = re.compile(r'^\s*([0-9a-fA-F]+):\s+((?:[0-9a-fA-F][0-9a-fA-F] )*[0-9a-fA-F][0-9a-fA-F])\s+(.*?)\s*$')

def objdump(filename):
    proc = subprocess.Popen(['objdump', '-D', filename], stdout=subprocess.PIPE)
    curr_func = None
    func_start = None
    func_end = None
    ret = {}

    def storeFunc():
        if curr_func is None or func_start is None or func_end is None:
            return
        ret[curr_func] = {
            'name': curr_func,
            'start': func_start,
            'end': func_end,  # exclusive
            'length': func_end - func_start
        }

    for line in proc.stdout:
        line = line.strip()

        m = re_funcstart.match(line)
        if m is not None:
            if curr_func is not None:
                storeFunc()
            curr_func = m.group(1)
            func_start = None
            func_end = None

        m = re_codeline.match(line)
        if m is not None:
            func_addr = long(m.group(1), 16)
            func_bytes = m.group(2)
            func_nbytes = len(func_bytes.split(' '))
            func_instr = m.group(3)
            if func_start is None:
                func_start = func_addr
            func_end = func_addr + func_nbytes

    storeFunc()

    return ret

def filterFuncs(funcs):
    todo = []  # avoid mutation while iterating

    def accept(fun):
        n = fun['name']

        if n in [ '.comment',
                  '.dynstr',
                  '.dynsym',
                  '.eh_frame_hdr',
                  '.interp',
                  '.rela.dyn',
                  '.rela.plt',
                  '_DYNAMIC',
                  '_GLOBAL_OFFSET_TABLE_',
                  '_IO_stdin_used',
                  '__CTOR_LIST__',
                  '__DTOR_LIST__',
                  '_fini',
                  '_init',
                  '_start',
                  '' ]:
            return False

        for pfx in [ '.debug', '.gnu', '.note',
                     '__FRAME_', '__' ]:
            if n.startswith(pfx):
                return False

        return True

    for k in funcs.keys():
        if not accept(funcs[k]):
            todo.append(k)

    for k in todo:
        del funcs[k]

def main():
    funcs = objdump(sys.argv[1])
    filterFuncs(funcs)

    funcs_keys = funcs.keys()
    funcs_keys.sort()
    combined_size_all = 0
    combined_size_duk = 0
    for k in funcs_keys:
        fun = funcs[k]
        combined_size_all += fun['length']
        if fun['name'].startswith('duk_'):
            combined_size_duk += fun['length']

    f = sys.stdout
    f.write('<html>')
    f.write('<head>')
    f.write('<title>Size dump for %s</title>' % sys.argv[1])
    f.write("""\
<style type="text/css">
tr:nth-child(2n) {
    background: #eeeeee;
}
tr:nth-child(2n+1) {
    background: #dddddd;
}
</style>
""")
    f.write('</head>')
    f.write('<body>')

    f.write('<h1>Summary</h1>')
    f.write('<table>')
    f.write('<tr><td>Entries</td><td>%d</td></tr>' % len(funcs_keys))
    f.write('<tr><td>Combined size (all)</td><td>%d</td></tr>' % combined_size_all)
    f.write('<tr><td>Combined size (duk_*)</td><td>%d</td></tr>' % combined_size_duk)
    f.write('</table>')

    f.write('<h1>Sorted by function name</h1>')
    f.write('<table>')
    f.write('<tr><th>Name</th><th>Bytes</th></tr>')
    funcs_keys = funcs.keys()
    funcs_keys.sort()
    for k in funcs_keys:
        fun = funcs[k]
        f.write('<tr><td>%s</td><td>%d</td></tr>' % (fun['name'], fun['length']))
    f.write('</table>')

    f.write('<h1>Sorted by size</h1>')
    f.write('<table>')
    f.write('<tr><th>Name</th><th>Bytes</th></tr>')
    funcs_keys = funcs.keys()
    def cmpSize(a,b):
        return cmp(funcs[a]['length'], funcs[b]['length'])
    funcs_keys.sort(cmp=cmpSize)
    for k in funcs_keys:
        fun = funcs[k]
        f.write('<tr><td>%s</td><td>%d</td></tr>' % (fun['name'], fun['length']))
    f.write('</table>')

    f.write('</body>')
    f.write('</html>')

if __name__ == '__main__':
    main()
