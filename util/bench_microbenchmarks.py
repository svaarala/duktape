#!/usr/bin/env python2

import os
import sys
import time
import json
import subprocess

TIME_MULTI='./util/time_multi.py'
COUNT=5
SLEEP=0.5
SLEEP_FACTOR=2.5
RERUN_LIMIT=20
KILL_TIMEOUT=600
KILL_WAIT=20
TMP_BENCH_ONE='/tmp/bench-one.json'
BENCH_OUT='/tmp/bench.json'

#COUNT=1
#SLEEP=0.0
#SLEEP_FACTOR=0.0

# - Duktape is interpreted and uses reference counting
# - Python and Perl are interpreted and also use reference counting
# - Ruby and Lua are interpreted but don't use reference counting
# - Mujs is interpreted but doesn't use reference counting
# - Rhino compiles to Java bytecode and is ultimately JITed
# - Node.js (V8) is JITed
# - Luajit is JITed

engines = [
    { 'cmd': './duk-perf-pgo.O2.220', 'ext': 'js' },
    { 'cmd': './duk-pgo.O2.220', 'ext': 'js' },
    { 'cmd': './duk-perf.O2.220', 'ext': 'js' },
    { 'cmd': './duk.O2.220', 'ext': 'js' },

    { 'cmd': './duk.O2.210', 'ext': 'js' },
    { 'cmd': './duk.O2.200', 'ext': 'js' },
    { 'cmd': './duk.O2.150', 'ext': 'js' },

    { 'cmd': 'mujs', 'ext': 'js' },
    { 'cmd': 'jerry', 'ext': 'js' },
    { 'cmd': 'lua', 'ext': 'lua' },
    { 'cmd': 'python', 'ext': 'py' },
    { 'cmd': 'perl', 'ext': 'pl' },
    { 'cmd': 'ruby', 'ext': 'rb' },

    { 'cmd': 'luajit', 'ext': 'lua', 'jit': True },
    { 'cmd': 'rhino', 'ext': 'js', 'jit': True },
    { 'cmd': 'nodejs', 'ext': 'js', 'jit': True }
]

for eng in engines:
    if not eng.has_key('name'):
        eng['name'] = os.path.basename(eng['cmd'])
    if not eng.has_key('jit'):
        eng['jit'] = False

def run_one(testfile):
    #sys.stdout.write('%s:' % testfile)
    #sys.stdout.flush()

    doc = {
        'test': testfile,
        'engines': {}
    }

    for eng in engines:
        #sys.stdout.write(' %s' % eng)
        #sys.stdout.flush()
        testpath = testfile + '.' + eng['ext']
        cmd = [
            'python2',
            TIME_MULTI,
            '--count', str(COUNT),
            '--sleep', str(SLEEP),
            '--sleep-factor', str(SLEEP_FACTOR),
            '--rerun-limit', str(RERUN_LIMIT),
            '--kill-timeout', str(KILL_TIMEOUT),
            '--kill-wait', str(KILL_WAIT),
            '--mode', 'all',
            #'--verbose',
            '--output', TMP_BENCH_ONE,
            eng['cmd'],
            testpath
        ]
        #print(' '.join(cmd))

        proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        ret = proc.communicate(input='')
        #if print_stdout:
        #    sys.stdout.write(ret[0])
        #    sys.stdout.flush()
        if proc.returncode != 0:
            sys.stdout.write(ret[1])  # print stderr on error
            sys.stdout.flush()

        with open(TMP_BENCH_ONE, 'rb') as f:
            subdoc = json.loads(f.read())
        doc['engines'][eng['name']] = subdoc

    #sys.stdout.write('\n')
    #sys.stdout.flush()

    return doc

def format_one(doc):
    res = '%-40s:' % os.path.basename(doc['test'])
    for eng in engines:
        engname = eng['name']
        t = doc['engines'][engname]
        if t.get('killed', False):
            res += ' %s %5s' % (engname, 'kill')
        elif t.get('sigsegv', False):
            res += ' %s %5s' % (engname, 'segv')
        elif t.get('failed', False):
            res += ' %s %5s' % (engname, 'n/a')
        else:
            res += ' %s %5.2f' % (engname, t['time_min'])
    return res

def main():
    doc = {
        'tests': []
    }

    filenames = []
    for fn in sys.argv[1:]:
        t = os.path.splitext(fn)[0]
        if os.path.basename(t)[0:5] != 'test-':
            continue
        if t not in filenames:
            filenames.append(t)

    print('Running benchmarks for %d unique tests: %s' % (len(filenames), ' '.join([ os.path.basename(x) for x in filenames ])))
    print('')
    print('-' * 76)
    print('')

    start_time = time.time()
    for idx,fn in enumerate(filenames):
        subdoc = run_one(fn)
        doc['tests'].append(subdoc)
        now = time.time()
        rate = float(now - start_time) / float(idx + 1)
        eta = rate * (float(len(filenames)) - float(idx + 1))
        print('[ %3d/%3d, %3d%%, ETA %3d min ] %s' % \
            (idx + 1, len(filenames), \
             int(float(idx + 1) / float(len(filenames)) * 100.0), \
             int(eta / 60.0), format_one(subdoc)))

    print('')
    now = time.time()
    print('Total time: %.2f minutes' % ((float(now) - float(start_time)) / 60.0))
    print('')
    print('-' * 76)
    print('')
    for subdoc in doc['tests']:
        print(format_one(subdoc))

    print('')
    print('-' * 76)
    print('')
    with open(BENCH_OUT, 'wb') as f:
        f.write(json.dumps(doc, indent=4))
        print('Wrote output JSON to %s' % BENCH_OUT)

if __name__ == '__main__':
    main()
