===============
Duktape website
===============

Overview
========

This directory contains all that is necessary to build the Duktape website
(http://duktape.org/).

The website has been implemented as a set of static files, to allow the site
to be cached and viewed off-line.  The pages do depend on external resources,
especially Google Fonts and Web Font Loader.  However, the pages are designed
to gracefully degrade:

* No network connection: pages must be readable with at least fallback fonts

* No CSS: pages must be readable with reasonable styling (default HTML styling)

* No Javascript: pages must be readable

The pages are also designed to be reasonably readable with a text browser
like w3m or elinks.

Build process
=============

A custom templating model is used to generate the HTML files, see
``buildsite.py``.  The general process is:

* An initial HTML source document is generated from HTML snippets and with
  Python functions.  For instance, API function descriptions are in the
  ``api/`` directory and they have a specific syntax that the Python functions
  can parse.  The Python code also creates the HTML source for representing
  value stack states.

* The resulting HTML document is parsed with BeautifulSoup into a parse tree.

* Transformation passes are applied to the parse tree.  For instance, C and
  ECMAScript code is colorized with ``source-highlight``.

* Finally, the parse tree is converted into an ASCII HTML document with
  BeautifulSoup.

API documentation
=================

API documentation is essential for developers, so special emphasis has been
placed on making it as useful as possible.  In particular:

* Official function-by-function API documentation is available from
  a standard place.

* API documentation can be easily searched and browsed.  Searching for
  functions is possible with browser page search.

* API documentation can be easily linked to.  In particular, it is possible
  to link to individual concepts or functions.

* API documentation can be read online, or downloaded for offline use.

The API documentation is compiled into a single HTML file, but is edited as
a set of individual source files in this directory.

Web font loading
================

Google Web Fonts are used for the CSS fonts:

* https://developers.google.com/fonts/docs/getting_started

To avoid long load delays, web fonts are loaded asynchronously instead of
using the simple "CSS include" mechanism.  Web Font Loader is used for
the asynchronous loading:

* https://github.com/typekit/webfontloader/blob/master/README.md

Because we want to avoid both (1) showing unstyled text ("FOUT") and
(2) waiting indefinitely for fonts load (very annoying with no network
ocnnection), we use a trick by Kevin Dew.  A custom ``wf-fail`` class
is added to the HTML element if font loading doesn't complete in a
reasonable time; CSS can then use this class to display the page.  This
avoids FOUT in most cases and is still reasonably responsive when font
loading fails very slowly.  See:

* http://kevindew.me/post/47052453532/a-fallback-for-when-google-web-font-loader-fails

Finally, to allow clients without Javascript to render the page, the
HTML element initially has a custom ``wf-nojavascript`` class which is
immediately removed by Javascript code.  If Javascript is disabled, the
class remains, and text can be shown based on this custom class.
Unfortunately this means that clients without Javascript support will
only see fallback fonts.

Minimizing HTML file size and transfer size
===========================================

Since large single-file documents are used, it is important to minimize
file size and file transfer size.  Some notes on that below.

Code snippets
-------------

Colorized code snippets are generated with source-highlight, which produces
very verbose output.  Using the external CSS option for source-highlight
makes the output more easily stylable and also reduces output size.

HTTP compression
----------------

Repetitive HTML code compresses to about 10-15% of uncompressed size, so
HTTP compression should definitely be used.  HTTP compression is supported
by practically all servers and browsers:

* http://www.http-compression.com/

Data URI images
---------------

Very small, non-repeated CSS images can be embedded with data URIs to
minimize load time.  Currently done manually, see ``buildimages.py``.

Misc issues
===========

Heading elements
----------------

Use the heading elements (``<h1>``, ``<h2>``, etc) only for those headings
that are logically part of the document structure and should be displayed
in a "table of contents".

The Duktape logo and the page/document title are **not** such headings.
They are represented as::

  <div class="main-title"><strong>Duktape</strong></div>

The ``div`` outside provides block formatting while ``strong`` ensures text
browsers emphasize the text properly.

Table overflows
---------------

Narrow layouts need table overflow handling.  The best solution would be to
CSS style the table element itself, but that doesn't seem to work cleanly in
all browsers.  For now, use::

  <div class="table-wrap">
  <table>...</table>
  </div>

Obsolete elements
-----------------

Instead of ``<tt>`` use ``<code>`` as a general replacement.  Validators
will complain about ``<tt>`` because it's an obsolete "representational"
element.

Favicon
-------

The site should provide a 16x16 favicon.

Apple touch icon
----------------

The site should provide an Apple touch icon.  It is also used by Android and
Nokia N9 (and probably a lot of other devices) to automagic an icon for a
bookmarked web page.  If there is no such icon, there will be multiple
unnecessary requests to the server for a page load.

Without any document meta headers Nokia N9, for instance, attempts to get::

  GET /apple-touch-icon-80x80-precomposed.png
  GET /apple-touch-icon-80x80.png
  GET /apple-touch-icon-precomposed.png
  GET /apple-touch-icon.png

iOS versions will have different automatic icon searches, like::

  GET /apple-touch-icon-57x57-precomposed.png
  GET /apple-touch-icon-57x57.png
  GET /apple-touch-icon-precomposed.png
  GET /apple-touch-icon.png

Different devices prefer different icon sizes.  You can specify multiple
sizes in the document metadata; apparently devices will select first
matching.  Older devices which don't understand a "sizes" specification
will apparently use last entry::

  <link rel="apple-touch-icon" sizes="144x144" href="touch-icon-ipad-144.png" />
  <link rel="apple-touch-icon" href="touch-icon-default.png" />

Other issues:

* There's various information about the appropriate icon sizes and they've
  apparently changed with iOS 7.

* Icons can either be "precomposed" (= gloss effect already applied) or
  non-precomposed in which case the device will apply whatever gloss
  effects it desires.  The movement is perhaps away from automatic
  glossing; Apple docs say: "Safari on iOS 7 doesn’t add effects to icons.".

Resources:

* https://developer.apple.com/library/safari/documentation/AppleApplications/Reference/SafariWebContent/ConfiguringWebApplications/ConfiguringWebApplications.html
* http://stackoverflow.com/questions/2997437/what-size-should-apple-touch-icon-png-be-for-ipad-and-iphone-4

Startup image
-------------

Some devices can also support a "startup image" when a bookmark is loaded.
Specify as::

  <link rel="apple-touch-startup-image" href="/startup.png">

References:

* https://developer.apple.com/library/safari/documentation/AppleApplications/Reference/SafariWebContent/ConfiguringWebApplications/ConfiguringWebApplications.html

Multicolumn layout and text-shadow/filter
-----------------------------------------

Multicolumn layout combined with ``text-shadow`` or ``filter`` on hover
causes the multicolumn elements to reflow when hovering over elements.
There is no such effects in e.g. paragraph inline text.

Workaround is to avoid these CSS properties for multicolumn layouts.
Link hovering can be highlighted with e.g. a darker color than normal
and/or with a background color.

Browser testing
===============

Browser testing is ad hoc now.  Current browser set:

* Firefox

* Chromium

* Android browser

* Nokia N9 browser

* Konqueror

* Netscape Navigator 4.79

  - Graceful degradation test for CSS
  - http://www.ailis.de/~k/archives/75-Netscape-Navigator-4-on-Ubuntu-Linux-12.10.html

* Internet Explorer 6

  - Graceful degradation test for CSS

* W3m

* Elinks

Validation
==========

Validate pages against:

* http://validator.w3.org/

* http://html5.validator.nu/

Credits
=======

GNU source-highlight:

* http://www.gnu.org/software/src-highlite/

Google fonts:

* http://www.google.com/fonts

Web Font Loader:

* https://github.com/typekit/webfontloader

Kevin Dew's FOUT trick:

* http://kevindew.me/post/47052453532/a-fallback-for-when-google-web-font-loader-fails

CSS reset:

* http://www.cssreset.com/

GitHub ribbon:

* https://github.com/blog/273-github-ribbons

Random notes
============

* http://stackoverflow.com/questions/14323376/do-html5-script-tag-need-type-javascript

* overflow:auto is an alternative to the traditional "clearfix"

* http://ajiprabowo.wordpress.com/2011/11/11/saying-goodbye-to-the-overflow-hidden-clearing-hack/

* http://designshack.net/articles/css/whats-the-deal-with-display-inline-block/

* http://coding.smashingmagazine.com/2011/09/19/css3-flexible-box-layout-explained/

Future work
===========

Inline elements for source-highlight output
-------------------------------------------

Transform source-highlight output to change most common span elements
into standard inline elements and apply styling to them based on a
wrapper div class (e.g. ``.c-code i { ... }``).  This should reduce size
of highlighted source code considerably, with a small impact on text
browser readability.

Inline elements for value stacks
--------------------------------

Value stack HTML code size can be minimized by using standard inline
elements with minimal explicit classing.

The downside of this approach is that text browsing is impacted.  The
inline elements should be chosen to be reasonable (even meaningful) for
text browsing.

HTML inline elements:

* https://developer.mozilla.org/en-US/docs/HTML/Inline_elements

Best candidates are probably:

* b
* i
* tt
* em

