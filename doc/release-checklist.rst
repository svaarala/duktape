=================
Release checklist
=================

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

* Check dist-files/README.txt

* Ensure LICENSE.txt is up-to-date

  - Check year range

* Ensure RELEASES.txt is up-to-date (must be done before candidate tar.xz
  build because dist package contains RELEASES.txt)

  - New release is in place

  - Release log entries match ditz issues

  - Release date is in place

* Compilation tests: clean compile with common debug options
  (DUK_OPT_DEBUG, DUK_OPT_DPRINT, DUK_OPT_SELF_TESTS, DUK_OPT_ASSERTIONS)
  and with no debug options:

  - **FIXME: incomplete list, automate compilation tests**

  - Linux x86-64 gcc

  - Linux x86-64 gcc + -m32

  - Linux x86-64 clang

  - Linux x86-64 clang + -m32

  - FreeBSD clang

  - FreeBSD clang + -m32

  - Windows MinGW

  - Windows MinGW-w64

  - Windows MSVC (cl) x86

  - Windows MSVC (cl) x64

  - Linux MIPS gcc

* Ecmascript testcases

  - **FIXME: semiautomate test running for various configurations**

  - On x86-64:

    - make qecmatest   # quick test

    - make vgecmatest  # valgrind test

  - Run testcases on all endianness targets

  - Run with assertions enabled at least on x86-64

* API testcases

  - On x86-64:

    - make apitest

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

* Build candidate tar.xz files

  - These should remain the same after this point so that their hash
    values are known

  - NOTE: because dist-files/README.txt also contains a "git describe" of
    the current commit, the describe string will refer to the previous
    release tag (not the current release tag)

* Check source dist contents

  - Check file list

  - Grep for FIXME and XXX

  - Trivial compile test for combined source

  - Trivial compile test for separate sources (important because
    it's easy to forget to add files in make_dist.sh)

* Store binaries and update website downloads page

  - Release date

  - Link

  - Date

  - "latest" class

* Build website

  - Readthrough

  - Test that the Duktape REPL (Dukweb) works

* Ditz release

  - ``ditz release vN.N``

  - git add and commit ditz issues

* Git release and tag

  - ``git tag -l -n1`` to list current tags

  - ``git tag -s -m "<one line release description>" vN.N.N``

* Upload and test

* Bump Duktape version for next release and testing

  - Set patch level to 99, e.g. after 0.10.0 stable release, set DUK_VERSION
    from 1000 to 1099.  This ensures that any forks off the trunk will have a
    version number easy to distinguish as an unofficial release.

  - ``src/duk_api_public.h.in``
