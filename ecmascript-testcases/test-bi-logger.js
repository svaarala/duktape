/*
 *  Duktape logger basic tests.
 *
 *  The raw() function is replaced here: the default one writes to stderr
 *  which we cannot test, and also we need to normalize the timestamp from
 *  log lines to write expect strings.
 */

function raw_replacement(msg) {
    // Timestamp is non-predictable
    msg = String(msg);  // arg is a buffer
    msg = msg.replace(/^\S+/, 'TIMESTAMP');
    print(msg);
}

Duktape.Logger.prototype.raw = raw_replacement;

/*===
logger name
TIMESTAMP INF myLogger: logger created
TIMESTAMP INF anon: logger created
TIMESTAMP INF anon: undefined: inherit anon
TIMESTAMP INF anon: logger created
TIMESTAMP INF anon: null: inherit anon
TIMESTAMP INF input: logger created
TIMESTAMP INF myLogger: logger created
TIMESTAMP INF newName: logger renamed
TIMESTAMP INF anon: logger name deleted
TIMESTAMP INF inherited: inherited name changed
===*/

function loggerNameTest() {
    var logger;

    // Logger name can be set manually

    logger = new Duktape.Logger('myLogger');
    logger.info('logger created');

    // Logger name given as null/undefined (any non-string actually) causes
    // the logger to not have a 'n' property and does *not* trigger automatic
    // name assignment.

    logger = new Duktape.Logger(undefined);
    logger.info('logger created');
    logger.info('undefined: inherit anon');

    logger = new Duktape.Logger(null);
    logger.info('logger created');
    logger.info('null: inherit anon');

    // Logger name defaults to caller fileName if present.  We don't know
    // the full path of the logger and fileName is not writable, so we use
    // eval to force caller fileName to "input".

    logger = eval('new Duktape.Logger()');  // caller fileName is 'input'
    logger.info('logger created');

    // If fileName is not present, logger gets no 'n' property and
    // inherits 'n' from Logger prototype

    /* XXX: cannot easily test because cannot create a function with no
     * fileName, and fileName is not configurable so can't delete it.
     */

    // Logger name can be modified runtime, and even removed in which
    // case it is inherited from the Logger prototype

    logger = new Duktape.Logger('myLogger');
    logger.info('logger created');
    logger.n = 'newName';
    logger.info('logger renamed');
    delete logger.n;
    logger.info('logger name deleted');
    Duktape.Logger.prototype.n = 'inherited';
    logger.info('inherited name changed');
    Duktape.Logger.prototype.n = 'anon';
}

print('logger name');

try {
    loggerNameTest();
} catch (e) {
    print(e);
}

/*===
log level
TIMESTAMP INF myLogger: info 1
TIMESTAMP WRN myLogger: warn 1
TIMESTAMP ERR myLogger: error 1
TIMESTAMP FTL myLogger: fatal 1
TIMESTAMP TRC myLogger: trace 2
TIMESTAMP DBG myLogger: debug 2
TIMESTAMP INF myLogger: info 2
TIMESTAMP WRN myLogger: warn 2
TIMESTAMP ERR myLogger: error 2
TIMESTAMP FTL myLogger: fatal 2
TIMESTAMP WRN myLogger: warn 3
TIMESTAMP ERR myLogger: error 3
TIMESTAMP FTL myLogger: fatal 3
===*/

function logLevelTest() {
    var logger;

    logger = new Duktape.Logger('myLogger');

    // Default log level is 'info', so trace/debug is omitted.

    logger.trace('trace 1');
    logger.debug('debug 1');
    logger.info('info 1');
    logger.warn('warn 1');
    logger.error('error 1');
    logger.fatal('fatal 1');

    // Update global log level from prototype

    Duktape.Logger.prototype.l = 0;

    logger.trace('trace 2');
    logger.debug('debug 2');
    logger.info('info 2');
    logger.warn('warn 2');
    logger.error('error 2');
    logger.fatal('fatal 2');

    Duktape.Logger.prototype.l = 2;  // back to info

    // Update logger log level from the instance

    logger.l = 3;

    logger.trace('trace 3');
    logger.debug('debug 3');
    logger.info('info 3');
    logger.warn('warn 3');
    logger.error('error 3');
    logger.fatal('fatal 3');

    logger.l = 2;
}

print('log level');

try {
    logLevelTest();
} catch (e) {
    print(e);
}

/*===
formatting
types
TIMESTAMP INF myLogger: type: undefined
TIMESTAMP INF myLogger: type: null
TIMESTAMP INF myLogger: type: true
TIMESTAMP INF myLogger: type: false
TIMESTAMP INF myLogger: type: 123.4
TIMESTAMP INF myLogger: type: str
TIMESTAMP INF myLogger: type: foo,bar,quux
TIMESTAMP INF myLogger: type: [object Object]
TIMESTAMP INF myLogger: error: ERROR:my error
toLogString() called
toLogString() called
TIMESTAMP INF myLogger: lazy visible {foo:1,bar:2} {quux:3,baz:4}
TIMESTAMP INF myLogger: lazy visible {foo:1,bar:2} {quux:3,baz:4}
TIMESTAMP INF myLogger: lazy visible {foo:1,bar:2} {quux:3,baz:4}
TIMESTAMP INF myLogger: instance overrides fmt: OBJECT
TIMESTAMP INF myLogger2: other loggers still use default fmt: [object Object]
TIMESTAMP INF myLogger: logger now using default fmt again: [object Object]
TIMESTAMP INF myLogger: now using jx formatting: {foo:1,bar:2}
TIMESTAMP INF myLogger: string 0 string 1 string 2 string 3 string 4 string 5 string 6 string 7 string 8 string 9 string 10 string 11 string 12 string 13 string 14 string 15 string 16 string 17 string 18 string 19 string 20 string 21 string 22 string 23 string 24 string 25 string 26 string 27 string 28 string 29 string 30 string 31 string 32 string 33 string 34 string 35 string 36 string 37 string 38 string 39 string 40 string 41 string 42 string 43 string 44 string 45 string 46 string 47 string 48 string 49 string 50 string 51 string 52 string 53 string 54 string 55 string 56 string 57 string 58 string 59 string 60 string 61 string 62 string 63 string 64 string 65 string 66 string 67 string 68 string 69 string 70 string 71 string 72 string 73 string 74 string 75 string 76 string 77 string 78 string 79 string 80 string 81 string 82 string 83 string 84 string 85 string 86 string 87 string 88 string 89 string 90 string 91 string 92 string 93 string 94 string 95 string 96 string 97 string 98 string 99 string 100 string 101 string 102 string 103 string 104 string 105 string 106 string 107 string 108 string 109 string 110 string 111 string 112 string 113 string 114 string 115 string 116 string 117 string 118 string 119 string 120 string 121 string 122 string 123 string 124 string 125 string 126 string 127 string 128 string 129 string 130 string 131 string 132 string 133 string 134 string 135 string 136 string 137 string 138 string 139 string 140 string 141 string 142 string 143 string 144 string 145 string 146 string 147 string 148 string 149 string 150 string 151 string 152 string 153 string 154 string 155 string 156 string 157 string 158 string 159 string 160 string 161 string 162 string 163 string 164 string 165 string 166 string 167 string 168 string 169 string 170 string 171 string 172 string 173 string 174 string 175 string 176 string 177 string 178 string 179 string 180 string 181 string 182 string 183 string 184 string 185 string 186 string 187 string 188 string 189 string 190 string 191 string 192 string 193 string 194 string 195 string 196 string 197 string 198 string 199 string 200 string 201 string 202 string 203 string 204 string 205 string 206 string 207 string 208 string 209 string 210 string 211 string 212 string 213 string 214 string 215 string 216 string 217 string 218 string 219 string 220 string 221 string 222 string 223 string 224 string 225 string 226 string 227 string 228 string 229 string 230 string 231 string 232 string 233 string 234 string 235 string 236 string 237 string 238 string 239 string 240 string 241 string 242 string 243 string 244 string 245 string 246 string 247 string 248 string 249 string 250 string 251 string 252 string 253 string 254 string 255 string 256 string 257 string 258 string 259 string 260 string 261 string 262 string 263 string 264 string 265 string 266 string 267 string 268 string 269 string 270 string 271 string 272 string 273 string 274 string 275 string 276 string 277 string 278 string 279 string 280 string 281 string 282 string 283 string 284 string 285 string 286 string 287 string 288 string 289 string 290 string 291 string 292 string 293 string 294 string 295 string 296 string 297 string 298 string 299 string 300 string 301 string 302 string 303 string 304 string 305 string 306 string 307 string 308 string 309 string 310 string 311 string 312 string 313 string 314 string 315 string 316 string 317 string 318 string 319 string 320 string 321 string 322 string 323 string 324 string 325 string 326 string 327 string 328 string 329 string 330 string 331 string 332 string 333 string 334 string 335 string 336 string 337 string 338 string 339 string 340 string 341 string 342 string 343 string 344 string 345 string 346 string 347 string 348 string 349 string 350 string 351 string 352 string 353 string 354 string 355 string 356 string 357 string 358 string 359 string 360 string 361 string 362 string 363 string 364 string 365 string 366 string 367 string 368 string 369 string 370 string 371 string 372 string 373 string 374 string 375 string 376 string 377 string 378 string 379 string 380 string 381 string 382 string 383 string 384 string 385 string 386 string 387 string 388 string 389 string 390 string 391 string 392 string 393 string 394 string 395 string 396 string 397 string 398 string 399 string 400 string 401 string 402 string 403 string 404 string 405 string 406 string 407 string 408 string 409 string 410 string 411 string 412 string 413 string 414 string 415 string 416 string 417 string 418 string 419 string 420 string 421 string 422 string 423 string 424 string 425 string 426 string 427 string 428 string 429 string 430 string 431 string 432 string 433 string 434 string 435 string 436 string 437 string 438 string 439 string 440 string 441 string 442 string 443 string 444 string 445 string 446 string 447 string 448 string 449 string 450 string 451 string 452 string 453 string 454 string 455 string 456 string 457 string 458 string 459 string 460 string 461 string 462 string 463 string 464 string 465 string 466 string 467 string 468 string 469 string 470 string 471 string 472 string 473 string 474 string 475 string 476 string 477 string 478 string 479 string 480 string 481 string 482 string 483 string 484 string 485 string 486 string 487 string 488 string 489 string 490 string 491 string 492 string 493 string 494 string 495 string 496 string 497 string 498 string 499 string 500 string 501 string 502 string 503 string 504 string 505 string 506 string 507 string 508 string 509 string 510 string 511 string 512 string 513 string 514 string 515 string 516 string 517 string 518 string 519 string 520 string 521 string 522 string 523 string 524 string 525 string 526 string 527 string 528 string 529 string 530 string 531 string 532 string 533 string 534 string 535 string 536 string 537 string 538 string 539 string 540 string 541 string 542 string 543 string 544 string 545 string 546 string 547 string 548 string 549 string 550 string 551 string 552 string 553 string 554 string 555 string 556 string 557 string 558 string 559 string 560 string 561 string 562 string 563 string 564 string 565 string 566 string 567 string 568 string 569 string 570 string 571 string 572 string 573 string 574 string 575 string 576 string 577 string 578 string 579 string 580 string 581 string 582 string 583 string 584 string 585 string 586 string 587 string 588 string 589 string 590 string 591 string 592 string 593 string 594 string 595 string 596 string 597 string 598 string 599 string 600 string 601 string 602 string 603 string 604 string 605 string 606 string 607 string 608 string 609 string 610 string 611 string 612 string 613 string 614 string 615 string 616 string 617 string 618 string 619 string 620 string 621 string 622 string 623 string 624 string 625 string 626 string 627 string 628 string 629 string 630 string 631 string 632 string 633 string 634 string 635 string 636 string 637 string 638 string 639 string 640 string 641 string 642 string 643 string 644 string 645 string 646 string 647 string 648 string 649 string 650 string 651 string 652 string 653 string 654 string 655 string 656 string 657 string 658 string 659 string 660 string 661 string 662 string 663 string 664 string 665 string 666 string 667 string 668 string 669 string 670 string 671 string 672 string 673 string 674 string 675 string 676 string 677 string 678 string 679 string 680 string 681 string 682 string 683 string 684 string 685 string 686 string 687 string 688 string 689 string 690 string 691 string 692 string 693 string 694 string 695 string 696 string 697 string 698 string 699 string 700 string 701 string 702 string 703 string 704 string 705 string 706 string 707 string 708 string 709 string 710 string 711 string 712 string 713 string 714 string 715 string 716 string 717 string 718 string 719 string 720 string 721 string 722 string 723 string 724 string 725 string 726 string 727 string 728 string 729 string 730 string 731 string 732 string 733 string 734 string 735 string 736 string 737 string 738 string 739 string 740 string 741 string 742 string 743 string 744 string 745 string 746 string 747 string 748 string 749 string 750 string 751 string 752 string 753 string 754 string 755 string 756 string 757 string 758 string 759 string 760 string 761 string 762 string 763 string 764 string 765 string 766 string 767 string 768 string 769 string 770 string 771 string 772 string 773 string 774 string 775 string 776 string 777 string 778 string 779 string 780 string 781 string 782 string 783 string 784 string 785 string 786 string 787 string 788 string 789 string 790 string 791 string 792 string 793 string 794 string 795 string 796 string 797 string 798 string 799 string 800 string 801 string 802 string 803 string 804 string 805 string 806 string 807 string 808 string 809 string 810 string 811 string 812 string 813 string 814 string 815 string 816 string 817 string 818 string 819 string 820 string 821 string 822 string 823 string 824 string 825 string 826 string 827 string 828 string 829 string 830 string 831 string 832 string 833 string 834 string 835 string 836 string 837 string 838 string 839 string 840 string 841 string 842 string 843 string 844 string 845 string 846 string 847 string 848 string 849 string 850 string 851 string 852 string 853 string 854 string 855 string 856 string 857 string 858 string 859 string 860 string 861 string 862 string 863 string 864 string 865 string 866 string 867 string 868 string 869 string 870 string 871 string 872 string 873 string 874 string 875 string 876 string 877 string 878 string 879 string 880 string 881 string 882 string 883 string 884 string 885 string 886 string 887 string 888 string 889 string 890 string 891 string 892 string 893 string 894 string 895 string 896 string 897 string 898 string 899 string 900 string 901 string 902 string 903 string 904 string 905 string 906 string 907 string 908 string 909 string 910 string 911 string 912 string 913 string 914 string 915 string 916 string 917 string 918 string 919 string 920 string 921 string 922 string 923 string 924 string 925 string 926 string 927 string 928 string 929 string 930 string 931 string 932 string 933 string 934 string 935 string 936 string 937 string 938 string 939 string 940 string 941 string 942 string 943 string 944 string 945 string 946 string 947 string 948 string 949 string 950 string 951 string 952 string 953 string 954 string 955 string 956 string 957 string 958 string 959 string 960 string 961 string 962 string 963 string 964 string 965 string 966 string 967 string 968 string 969 string 970 string 971 string 972 string 973 string 974 string 975 string 976 string 977 string 978 string 979 string 980 string 981 string 982 string 983 string 984 string 985 string 986 string 987 string 988 string 989 string 990 string 991 string 992 string 993 string 994 string 995 string 996 string 997 string 998 string 999
TIMESTAMP INF myLogger: formatting error 123 TypeError: fake error 234
Error: can't coerce me
===*/

function formattingTest() {
    var logger, logger2;

    logger = new Duktape.Logger('myLogger');

    // Formatting test, all standard types except function (function coerces
    // a string with Ecmascript comments which interferes with the expect string)

    print('types');
    logger.info('type:', undefined);
    logger.info('type:', null);
    logger.info('type:', true);
    logger.info('type:', false);
    logger.info('type:', 123.4);
    logger.info('type:', 'str');
    logger.info('type:', [ 'foo', 'bar', 'quux' ]);
    logger.info('type:', { foo: 1, bar: 2 });

    // toLogString()

    Error.prototype.toLogString = function() { return 'ERROR:' + this.message; };
    logger.info('error:', new TypeError('my error'));
    delete Error.prototype.toLogString;

    // Lazy formatting using toLogString

    function lazyJx1(val) {
        // Simple alternative, creates a closure per value
        return {
            toLogString: function() {
                print('toLogString() called');
                return Duktape.enc('jx', val);
            }
        };
    }

    function lazyJx2(val) {
        // Alternative using bind() (does not print anything when formatting)
        return {
            toLogString: Duktape.enc.bind(null, 'jx', val)
        };
    }

    function LazyValue(val) {
        this.v = val;
    }
    LazyValue.prototype.toLogString = function () {
        return Duktape.enc('jx', this.v);
    }
    function lazyJx3(val) {
        // Alternative which avoids creating a closure per value.  This relies
        // on toLogString() being called as a method, with 'this' bound to the
        // object.
        return new LazyValue(val);
    }

    logger.info('lazy visible', lazyJx1({foo:1, bar:2}), lazyJx1({quux:3, baz:4}));
    logger.debug('lazy invisible', lazyJx1({foo:1, bar:2}), lazyJx1({quux:3, baz:4}));

    logger.info('lazy visible', lazyJx2({foo:1, bar:2}), lazyJx2({quux:3, baz:4}));
    logger.debug('lazy invisible', lazyJx2({foo:1, bar:2}), lazyJx2({quux:3, baz:4}));

    logger.info('lazy visible', lazyJx3({foo:1, bar:2}), lazyJx3({quux:3, baz:4}));
    logger.debug('lazy invisible', lazyJx3({foo:1, bar:2}), lazyJx3({quux:3, baz:4}));

    // The object formatter function fmt() can be overridden in the instance

    logger2 = new Duktape.Logger('myLogger2');
    logger.fmt = function() { return 'OBJECT'; }
    logger.info('instance overrides fmt:', {foo:1, bar:2});
    logger2.info('other loggers still use default fmt:', {foo:1, bar:2});
    delete logger.fmt;
    logger.info('logger now using default fmt again:', {foo:1, bar:2});

    // The formatter can be overridden at the global level too

    var old_fmt = Duktape.Logger.prototype.fmt;
    Duktape.Logger.prototype.fmt = function(val) { return Duktape.enc('jx', val); };
    logger.info('now using jx formatting:', {foo:1, bar:2});
    Duktape.Logger.prototype.fmt = old_fmt;

    // A lot of arguments and a long log string

    var args = [];
    for (i = 0; i < 1000; i++) {
        args.push('string ' + i);
    }
    logger.info.apply(logger, args);

    // Error during formatting causes formatted element to be replaced with
    // a ToString() coerced version of the error thrown.  Here the error is
    // triggered with toLogString()

    logger.info('formatting error',
                123,
                { toLogString: function() { throw new TypeError('fake error'); } },
                234);

    // If ToString(error) fails, that error is propagated to the caller at
    // the moment.  This may not be the best behavior, but test for the
    // current behavior.

    try {
        logger.info('formatting error, error coercion fails',
                    123,
                    {
                        toLogString: function() {
                            var err = new TypeError('fake error');
                            err.toString = function() { throw new Error("can't coerce me"); };
                            return err;
                        }
                    },
                    234);
    } catch (e) {
        print(e);
    }
}

print('formatting');

try {
    formattingTest();
} catch (e) {
    print(e);
}

/*===
multiline
TIMESTAMP INF myLogger: here
is a string
on multiple
lines
===*/

function multiLineTest() {
    var logger;

    logger = new Duktape.Logger('myLogger');

    // Multiline logging

    logger.info('here\nis a string\non multiple\nlines');
}

print('multiline');

try {
    multiLineTest();
} catch (e) {
    print(e);
}

/*===
length test
239
240
241
242
243
244
245
246
247
248
249
250
251
252
253
254
255
256
257
258
259
260
261
262
263
264
265
266
267
268
269
270
271
272
273
274
275
276
277
278
279
280
281
282
283
284
285
286
287
288
289
290
291
292
293
294
295
296
297
298
299
300
301
302
303
304
305
306
307
308
309
310
311
312
313
314
315
316
317
318
319
320
321
322
323
324
325
326
327
328
329
40
41
43
47
55
71
103
167
295
551
1063
2087
4135
8231
16423
32807
65575
131111
262183
524327
1048615
===*/

/* There are internal behavior differences when logging short and long
 * log messages.  Short messages (currently) reuse a single stashed
 * buffer, while longer messages use one-off buffers.  Exercise the
 * boundary between the two.  The limit is DUK_BI_LOGGER_SHORT_MSG_LIMIT,
 * currently 256.
 */

function lengthTest() {
    var logger;
    var msg;

    // Replace raw() with something that just prints the result length.

    function raw_printlen(msg) {
        print(String(msg).length);
    }
    Duktape.Logger.prototype.raw = raw_printlen;

    function mkStr(len) {
        var res = '';
        while (len--) {
            res = res + 'x';
        }
        return res;
    }

    // Exercise the boundary between the two buffer behaviors.  Start earlier
    // than the limit to account for the log prefix.
    logger = new Duktape.Logger('myLogger');
    msg = mkStr(200);
    while (msg.length <= 290) {
        logger.info(msg);
        msg = msg + 'x';
    }

    // Test log entries up to 1 MB.
    msg = 'y';
    while (msg.length <= 1024 * 1024) {
        logger.info(msg);
        msg = msg + msg;
    }
}

print('length test');

try {
    lengthTest();
} catch (e) {
    print(e);
}

Duktape.Logger.prototype.raw = raw_replacement;
