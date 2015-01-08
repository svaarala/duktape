=================
Release checklist
=================

Checklist for ordinary releases
===============================

* Git branch naming note

  - ``vN.N.N-release-prep``: use this naming for bumping version number, etc.
    Merge to master before tagging release.

  - ``vN.N.N-release-post``: use this naming for bumping version number after
    release, checklist fixes after release, etc.

* Git maintenance

  - ensure git commits are up-to-date

  - ensure branches are merged, unused branches deleted (also remotely)

  - ensure branches are rebased where applicable

  - git fsck --full

  - git gc --aggressive

* Ditz maintenance

  - Ensure Ditz issues for the new release are all closed

* Finalize DUK_VERSION

  - Change previous development version (with patch level 99) to release
    version

  - Verify by running Duktape cmdline and evaluating ``Duktape.version``

* Check dist-files/README.rst

* Ensure LICENSE.txt is up-to-date

  - Check year range

* Ensure RELEASES.rst is up-to-date (must be done before candidate tar.xz
  build because dist package contains RELEASES.rst)

  - New release is in place

  - Release log entries match ditz issues

  - Release date is in place

* Compilation tests:

  - Clean compile for command line tool with (a) no options and (b) common
    debug options (DUK_OPT_DEBUG, DUK_OPT_DPRINT, DUK_OPT_SELF_TESTS,
    DUK_OPT_ASSERTIONS)

  - Compile both from ``src`` and ``src-separate``.

  - Run ``mandel.js`` to test the the command line tool works.

  - Platform / compiler combinations (incomplete, should be automated):

    + Linux x86-64 gcc

    + Linux x86-64 gcc + -m32

    + Linux x86-64 clang

    + Linux x86-64 clang + -m32

    + FreeBSD clang

    + FreeBSD clang + -m32

    + Windows MinGW

    + Windows MinGW-w64

    + Windows MSVC (cl) x86

    + Windows MSVC (cl) x64

    + Windows Cygwin 32-bit

    + Windows Cygwin 64-bit

    + Linux MIPS gcc

    + Linux ARMEL gcc (little endian)

    + Linux gcc on some mixed endian ARM platform

    + Linux SH4 gcc

* Compile command line tool as a Windows DLL, checks Windows symbol visibility
  macros::

    > cd dist
    > cl /O2 /DDUK_OPT_DLL_BUILD /Isrc /LD src\duktape.c
    > cl /O2 /DDUK_OPT_DLL_BUILD /Isrc examples\cmdline\duk_cmdline.c duktape.lib
    > duk_cmdline.exe

* Ecmascript testcases

  - **FIXME: semiautomate test running for various configurations**

  - On x86-64 (exercise 16-byte duk_tval):

    - make qecmatest   # quick test

    - VALGRIND_WRAP=1 make qecmatest  # valgrind test

  - On x86-32 (exercise 8-byte duk_tval)

    - make qecmatest   # quick test

  - Run testcases on all endianness targets

  - Run with assertions enabled at least on x86-64

* Memory usage testing

  - Leaks are mostly detected by Valgrind, but bugs in valstack or object
    resize algorithms (or similar) can lead to unbounded or suboptimal
    memory usage

  - XXX: establish some baseline test

* API testcases

  - On x86-64:

    - make apitest

* Compile option matrix test

  - Run 1000 iterations of ``util/matrix_compile.py`` which compiles and runs
    random combinations of feature options and compilers (gcc, g++, clang) on
    x86, x64, and x32

* Regfuzz

  - On x86-64, with DUK_OPT_ASSERTIONS

    - make regfuzztest

* test262

  - on x86-64

    - make test262test

  - Run with assertions enabled at least on x86-64

* emscripten (run emscripten-generated code with Duktape)

  - on x86-64

    - make emscriptentest

* emscripten (compile Duktape with emscripten, run with Node)

  - on x86-64

    - make emscriptenduktest

* JS-Interpreter

  - on x86-64

    - make jsinterpretertest

* lua.js

  - on x86-64

    - make luajstest

* Git release and tag

  - Tagging should be done before creating the candidate tar files so that
    "git describe" output will have a nice tag name.

  - This will be a preliminary tag which can be moved if necessary.  Don't
    push it to the public repo until the tag is certain not to move anymore.

  - There can be commits to the repo after tagging but nothing that will
    affect "make dist" output.

  - ``git tag -l -n1`` to list current tags

  - ``git tag -s -m "<one line release description>" vN.N.N`` to set tag

  - ``git tag -f -s -m "<one line release description>" vN.N.N`` to forcibly
    reset tag if it needs to be moved

* If release is a stable major/minor release (e.g. 1.1.0), create a maintenance
  branch ``vN.N-maintenance`` off the release tag.

* Build candidate tar.xz files

  - These should remain the same after this point so that their hash
    values are known.

  - Check git describe output from dist ``README.rst``, ``src/duktape.h``,
    and ``src/duktape.c``.  It should show the release tag.

* Check source dist contents

  - Check file list

  - Grep for FIXME and XXX

  - Trivial compile test for combined source

  - Trivial compile test for separate sources (important because
    it's easy to forget to add files in make_dist.sh)

* Store binaries to duktape-releases repo

  - Add the tar.xz to the master branch

  - Create an independent branched named ``unpacked-vN.N.N`` with unpacked
    tar.xz contents

    + http://stackoverflow.com/questions/15034390/how-to-create-a-new-and-empty-root-branch

    + http://stackoverflow.com/questions/9034540/how-to-create-a-git-branch-that-is-independent-of-the-master-branch

  - Tag the final branch with ``vN.N.N``, push the tag, and delete the branch.
    The branch is not pushed to the server.

  - The concrete commands are packaged into ``add-unpacked.sh`` in
    duktape-releases repo.

* Update website downloads page

  - Release date

  - Link

  - Date

  - "latest" class

  - Release notes (layout and contents) for release

* Build website

  - Readthrough

  - Test that the Duktape REPL (Dukweb) works

  - Check duk command line version number in Guide "Getting started"

* Ditz release

  - ``ditz release vN.N``

  - git add and commit ditz issues

* Upload website and test

* Final Git stuff

  - Ensure ``master`` is pushed and unnecessary branches are cleaned up

  - Push the release tag

  - Push the maintenance branch if created

* Make GitHub release

  - Release description should match tag description but be capitalized

  - Attach the end user distributable to the GitHub release

* Bump Duktape version for next release and testing

  - Set patch level to 99, e.g. after 0.10.0 stable release, set DUK_VERSION
    from 1000 to 1099.  This ensures that any forks off the trunk will have a
    version number easy to distinguish as an unofficial release.

  - ``src/duk_api_public.h.in``

* Update ``DITZ_RELEASE`` in ``Makefile``

  - It should point to the next expected release so that ``make issuecount``
    and ``make issues`` provide useful output

Checklist for maintenance releases
==================================

* Make fixes to master and cherry pick fixes to maintenance branch (either
  directly or through a fix branch).  Test fixes in maintenance branch too.

* Update release notes and website in master.  **Don't** update these in
  the maintenance branch.

* Bump DUK_VERSION in maintenance branch.

* Review diff between previous release and new patch release.

* Tag release, description "maintenance release" should be good enough for
  most patch releases.

* Build release, push it to ``duktape-releases`` in binary and unpacked form.

* Build website from master.  Deploy only ``download.html``.

  This is rather hacky: we need the release notes so the build must be made
  from master, but master may also contain website changes for the next
  release.
