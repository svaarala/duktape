#!/usr/bin/env python
#
#  Compatibility stub which now executes JS-based tooling.
#
#  Should be Python2 and Python3 compatible.

import os
import sys
import time
import optparse
import subprocess

def detect_nodejs():
    try:
        cmd = [ 'nodejs', '-e', 'console.log("test")' ]
        res = subprocess.check_output(cmd)
        if res[:4] == 'test'.encode('utf-8'):
            return 'nodejs'
    except:
        pass

    try:
        cmd = [ 'node', '-e', 'console.log("test")' ]
        res = subprocess.check_output(cmd)
        if res[:4] == 'test'.encode('utf-8'):
            return 'node'
    except:
        pass

    return None

def parse_options():
    parser = optparse.OptionParser()
    parser.add_option('--repo-directory', dest='repo_directory', default=None, help='Duktape repo directory (default is CWD)')
    parser.add_option('--output-directory', dest='output_directory', default=None, help='Dist output directory (required)')
    parser.add_option('--git-commit', dest='git_commit', default=None, help='Force git commit hash')
    parser.add_option('--git-describe', dest='git_describe', default=None, help='Force git describe')
    parser.add_option('--git-branch', dest='git_branch', default=None, help='Force git branch name')
    parser.add_option('--create-spdx', dest='create_spdx', action='store_true', default=False, help='Create SPDX license file')
    parser.add_option('--rom-support', dest='rom_support', action='store_true', help=optparse.SUPPRESS_HELP)
    parser.add_option('--rom-auto-lightfunc', dest='rom_auto_lightfunc', action='store_true', default=False, help=optparse.SUPPRESS_HELP)
    parser.add_option('--user-builtin-metadata', dest='user_builtin_metadata', action='append', default=[], help=optparse.SUPPRESS_HELP)
    parser.add_option('--quiet', dest='quiet', action='store_true', default=False, help='Suppress info messages (show warnings)')
    parser.add_option('--verbose', dest='verbose', action='store_true', default=False, help='Show verbose debug messages')
    parser.add_option('--nodejs-command', dest='nodejs_command', default=None, help='Force Node.js command name')
    (opts, args) = parser.parse_args()

    return opts, args

def main():
    sys.stderr.write('\n')
    sys.stderr.write('****************************************************************************\n')
    sys.stderr.write('*** Duktape python tooling is obsolete, migrate to JS-based tooling!     ***\n')
    sys.stderr.write('*** This tool now internally invokes the JS-based tooling.               ***\n')
    sys.stderr.write('*** Minimum Node.js version is 14.x.                                     ***\n')
    sys.stderr.write('****************************************************************************\n')
    sys.stderr.write('\n')
    time.sleep(2)

    entry_cwd = os.getcwd()
    script_path = sys.path[0]  # http://stackoverflow.com/questions/4934806/how-can-i-find-scripts-directory-with-python

    opts, args = parse_options()

    if opts.nodejs_command is None:
        nodejs_command = detect_nodejs()
    else:
        nodejs_command = opts.nodejs_command
    if nodejs_command is None:
        raise Exception('failed to detect Node.js, override with --nodejs-command')

    for fn in [
        os.path.join(script_path, 'duktool.js'),
        os.path.join(script_path, '..', 'src-tools', 'index.js'),
        os.path.join(script_path, '..', 'src-tools', 'duktool.js')
    ]:
        if os.path.isfile(fn):
            duktool_path = fn
            break
    if duktool_path is None:
        raise Exception('could not find duktool.js or src-tools/index.js')

    cmd = [
        nodejs_command,
        duktool_path,
        'dist'
    ]
    if opts.output_directory is not None:
        cmd += [ '--output-directory', opts.output_directory ]
    if opts.repo_directory is not None:
        cmd += [ '--repo-directory', opts.repo_directory ]
    if opts.git_commit is not None:
        cmd += [ '--git-commit', opts.git_commit ]
    if opts.git_describe is not None:
        cmd += [ '--git-describe', opts.git_describe ]
    if opts.git_branch is not None:
        cmd += [ '--git-branch', opts.git_branch ]
    if opts.create_spdx:
        cmd += [ '--create-spdx' ]
    if opts.rom_support:
        print('--rom-support ignored (now always enabled)')
    if opts.rom_auto_lightfunc:
        raise Exception('--rom-auto-lightfunc no longer supported for dist (use it with configure)')
    if len(opts.user_builtin_metadata) > 0:
        raise Exception('--user-builtin-metadata no longer supported for dist (use --builtin-file with configure')
    if opts.quiet:
        print('--quiet ignored')
    if opts.verbose:
        print('--verbose ignored')
    if True:
        cmd += [ '--validate-git' ]

    sys.stderr.write('*** Executing JS-based tooling with command: ' + repr(cmd) + '\n\n')
    subprocess.check_call(cmd)

if __name__ == '__main__':
    main()
