Contributing to Duktape
=======================

Copyrights and licensing
------------------------

Duktape copyrights are held by its contributors.  Contributors agree to
license their contribution(s) under Duktape `LICENSE.txt`.  See `AUTHORS.rst`
for details.

To make a code contribution to Duktape
--------------------------------------

* Fork the Duktape GitHub repository and make your changes:

  - Use a well named topic branch for the changes, with lowercase and dashes,
    e.g. `xyz-compiler-fixes`.

  - Fork off the `master` branch.  Avoid forking from Duktape repository work
    branches as they are frequently rebased.

  - Your branch should have a consistent logical scope.  If the branch does
    several independent things (like adding a feature and fixing some unrelated
    repo scripts), use separate branches.

* Test your changes as thoroughly as possible.  At the very minimum:

  - CI test runs must pass, which covers both a code policy check, and
    runs basic API and ECMAScript test case set.

  - If some test cases are invalidated by the changes, fix the test cases as
    part of the branch.  If you add new functionality, you should add test
    case(s) to illustrate the changes and desired behavior.

  - Fix any code policy violations or let me know if the policy check is
    broken.  Consistent code is easier to read.

* Ensure your code follows the style guidelines in `code-issues.rst`.
  Not everything is spelled out explicitly, so try to follow the general
  style in the code base.

* Clean up your commits so that they are logical and well scoped:

  - Rebase your pull request if necessary so that commits are logical and
    clean.  A smaller number of larger, logical commits are preferred over
    small commits and "fixups".

  - Keep commits to source separate from commits to documentation.

  - If the branch includes a fix that might be cherry picked into a
    maintenance release, ensure that fix is in a separate commit.

* Add yourself to the end of the author list in `AUTHORS.rst` if you're
  not already on the list.  By doing this you confirm that:

  - You own the rights to the contribution, or have the legal right to
    license the contribution under Duktape `LICENSE.txt` on behalf of
    the copyright owner(s).

  - You, or the copyright owner(s), agree to irrevocably license your
    contribution under Duktape `LICENSE.txt`.

  - Please include an e-mail address, a link to your GitHub profile, or
    something similar to allow your contribution to be identified accurately.

* Create a pull request in GitHub.  For now, the "base branch" should be
  "master", i.e. the pull requests are merged directly to the master branch.
  In the description:

  - Summarize the change and the motivation for the change.

  - If test case status changes (tests are broken / fixed, test cases
    themselves needed fixing, test cases were added, etc), mention that.

  - A pull request can be created before you think your changes are finished.
    It's OK to work on a feature in the pull request: this facilitates
    discussion in the pull request comments.

* When the changes are finished, comment on the pull that you're no longer
  making any changes and would like the branch to be merged.  If there is
  feedback needing fixes, comment when you're done.  This ensures merges
  are not done too early.

To report bugs or request features
----------------------------------

Use GitHub issues to report bugs or request features:

* Please include a compilation or execution log to help diagnosis.

* For portability related compilation errors, such as endianness detection,
  please include a list of the preprocessor defines provided by your compiler
  (if possible).  For GCC and Clang there are helpful Makefile targets,
  `gccpredefs` and `clangpredefs`.

If you don't think your request needs a wide audience, you can also
send e-mail to <sami.vaarala@iki.fi>.
