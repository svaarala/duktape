============================
reStructuredText conventions
============================

File extension
==============

Although ``.txt`` extension is probably technically correct for RST files,
use ``.rst`` for internal documentation for better compatibility with editors,
GitHub, etc.

Section markers
===============

Book level:

#. Over- and underlined hash marks (``#``) for book title

#. Over- and underlines stars (``*``) for book sub-title

File level:

#. Over- and underlined equals signs (``=``) once at top of file (file topic)

#. Underlined equals signs (``=``)

#. Underlined minus signs (``-``)

#. Underlined colons (``:``)

#. Underlined periods (``.``)

The book level notation is reserved for future use.  It allows the
internal documentation to be easily built into a single HTML/PDF
file for ease of browsing.

Page breaks
===========

See http://comments.gmane.org/gmane.text.docutils.user/6473.

You can use::

  .. raw:: LaTeX
  
     \newpage

Lists
=====

Numbering
---------

List numbering styles can be mixed, e.g.::

  1. Foo

    a. Bar

       1. Quux

However, e.g. GitHub will renumber the bullets and may also change the
numbering style.  This will make references to list elements confusing;
e.g. if you refer to Quux as element 1.a.1 above, the reference is quite
confusing if Quux was renumbered to 1.1.1 or 1.a.iii.  Even so, such
references are sometimes necessary so they can be used.
Start at zero indent

Bullets
-------

Recommended bullet styles by level::

  * Foo

    + Bar

      - Quux

Start at zero indent
--------------------

Start lists at zero indent, e.g.::

  * Foo

  * Bar

If you don't, GitHub will render the list as a quoted block.

Empty lines
-----------

Use empty lines between elements and sub-elements for readability and to
minimize formatting issues::

  1. Foo

    a. sub-foo

       + sub-sub-foo

    b. sub-bar

  2. Bar

  3. Quux

Nested lists and GitHub
-----------------------

While ``rst2pdf`` and friends are somewhat lenient with respect to nested
lists, GitHub RST formatter is a bit more picky.  To work with the GitHub
formatter, make sure that a nested list's bullet mark is intended to the
level of the previous level's body, e.g.::

  1. Foo

     a. sub-foo

     b. sub-bar

  2. Bar

The following is **incorrect** and GitHub will render the nested list as
a quoted block::

  1. Foo

    a. sub-foo

    b. sub-bar

  2. Bar

Note that the required level depends on the length of the parent bullet.
This matters for numbered lists::

  9. Foo

     a. sub-foo

     b. sub-bar

  10. Bar

      a. sub-foo (with one more indent than above)

      b. sub-bar
