#!/usr/bin/env python2
#
#  Small helper for perftest runs.
#

import os
import sys
import json
import time
import optparse
import subprocess

class Alarm(Exception):
    pass

def alarm_handler(signum, frame):
    raise Alarm

def main():
    parser = optparse.OptionParser()
    parser.add_option('--count', type='int', dest='count', default=3, help='Number of test runs')
    parser.add_option('--mode', dest='mode', default='min', help='Mode for choosing result: min, max, avg, all')
    parser.add_option('--sleep', type='float', dest='sleep', default=0.0, help='Fixed sleep value between runs')
    parser.add_option('--sleep-factor', type='float', dest='sleep_factor', default=0.0, help='Relative sleep value between runs, e.g. 2.0 means sleep twice as long as previous test run')
    parser.add_option('--rerun-limit', type='int', dest='rerun_limit', default=30, help='Run test only once if test run time exceeds this time limit')
    parser.add_option('--kill-timeout', type='int', dest='kill_timeout', default=None, help='Timeout for SIGKILLing the test')
    parser.add_option('--kill-wait', type='int', dest='kill_wait', default=3, help='Time to wait after SIGKILLing subprocess')
    parser.add_option('--verbose', action='store_true', dest='verbose', default=False, help='Verbose output')
    parser.add_option('--output', default=None, help='Output JSON file')

    (opts, args) = parser.parse_args()

    time_min = None
    time_max = None
    time_sum = 0.0

    if opts.verbose:
        sys.stderr.write('Running:')
        sys.stderr.flush()

    doc = {
        'count': opts.count,
        'mode': opts.mode,
        'sleep': opts.sleep,
        'sleep_factor': opts.sleep_factor,
        'rerun_limit': opts.rerun_limit,
        'runs': [],
        'args': args,
        'time_list': [],
        'time_min': None,
        'time_max': None,
        'time_avg': None
    }

    if opts.output is not None and os.path.exists(opts.output):
        os.unlink(opts.output)

    for i in xrange(opts.count):
        time.sleep(opts.sleep)

        cmd = []
        cmd = cmd + args
        #print(repr(cmd))

        if opts.kill_timeout is not None:
            # https://stackoverflow.com/questions/1191374/using-module-subprocess-with-timeout
            import signal
            signal.signal(signal.SIGALRM, alarm_handler)
            signal.alarm(opts.kill_timeout)

        killed = False
        retval = -1
        time_start = time.time()
        try:
            p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            stdout, stderr = p.communicate()
            if opts.kill_timeout is not None:
                import signal
                signal.alarm(0)
            retval = p.wait()
        except Alarm:
            # XXX: Kill children? Not needed for duk executable.
            killed = True
            os.kill(p.pid, signal.SIGKILL)
            retval = p.wait()
            time.sleep(opts.kill_wait)
        time_end = time.time()

        run = {
            'cmd': cmd,
            'retval': retval,
            'failed': False,
            'killed': False,
            'sigsegv': False,
        }
        doc['runs'].append(run)

        if killed:
            run['failed'] = True
            run['killed'] = True
            doc['failed'] = True
            doc['killed'] = True
            break
        elif retval == 139:
            run['failed'] = True
            run['sigsegv'] = True
            doc['failed'] = True
            doc['sigsegv'] = True
            break
        elif retval != 0:
            run['failed'] = True
            doc['failed'] = True
            break

        time_this = time_end - time_start
        #print(i, time_this)

        if time_min is None:
            time_min = time_this
        else:
            time_min = min(time_min, time_this)
        if time_max is None:
            time_max = time_this
        else:
            time_max = max(time_max, time_this)
        time_sum += time_this

        if opts.verbose:
            sys.stderr.write(' %f' % time_this)
            sys.stderr.flush()

        doc['time_list'].append(time_this)

        # Sleep time dependent on test time is useful for thermal throttling.
        time_sleep = opts.sleep_factor * time_this + opts.sleep

        run['time'] = time_this
        run['sleep_time'] = time_sleep

        time.sleep(time_sleep)

        # If run takes too long, there's no point in trying to get an accurate
        # estimate.
        if time_this >= opts.rerun_limit:
            doc['rerun_limit_reached'] = True
            break

    if opts.verbose:
        sys.stderr.write('\n')
        sys.stderr.flush()

    # /usr/bin/time has only two digits of resolution
    if doc.get('killed', False):
        print('kill')
    elif doc.get('sigsegv', False):
        print('segv')
    elif doc.get('failed', False):
        print('n/a')
    else:
        time_avg = time_sum / float(len(doc['time_list']))

        doc['time_avg'] = time_avg
        doc['time_min'] = time_min
        doc['time_max'] = time_max

        if opts.mode == 'min':
            print('%.02f' % time_min)
        elif opts.mode == 'max':
            print('%.02f' % time_max)
        elif opts.mode == 'avg':
            print('%.02f' % time_avg)
        elif opts.mode == 'all':
            print('min=%.02f, max=%.02f, avg=%0.2f, count=%d: %r' % \
                  (time_min, time_max, time_avg, len(doc['time_list']), doc['time_list']))
        else:
            print('invalid mode: %r' % opts.mode)

    if opts.output is not None:
        with open(opts.output, 'wb') as f:
            f.write(json.dumps(doc, indent=4) + '\n')

    sys.exit(0)

if __name__ == '__main__':
    main()
