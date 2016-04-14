================
Kraken benchmark
================

Download a Kraken snapshot and unpack it, for example:

* http://hg.mozilla.org/projects/kraken/file/e119421cb325

Then use ``kraken_duk.py`` as the test driver; for now::

    $ cd kraken-e119421cb325
    $ cp ../kraken_duk.py ../kraken_harness.js .   # some path assumptions
    $ cp /path/to/my/duk duk    # hardcoded assumption in kraken_duk.py now
    $ ./sunspider --shell ./kraken_duk.py --suite kraken-1.0 --runs 10
