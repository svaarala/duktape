===============
Git conventions
===============

Repositories
============

The main development repository is hosted on Github:

* https://github.com/svaarala/duktape

The repo includes Ditz issues which are used for low level task tracking.
Because Ditz is no longer well supported (and doesn't work with newer Ruby
versions), Ditz issues will be migrated to some other file-based tracker.

The duktape.org website is also part of the repository.  Up to Duktape 0.12.0
also the release binaries were stored in the Duktape repo.  For Duktape 1.0.0
and after the convention is changed to use external binaries, see releases
section below.  The upside of keeping the website in the same repo is that
old documentation matching the current checkout is always available.

There's a separate repo for release binaries, so that binaries are reliably
available but don't bloat the main repository:

* https://github.com/svaarala/duktape-releases

This repo also provides unpacked release files as tags for convenience.

Releases
========

Release versioning follows semantic versioning, for details, see:

* http://duktape.org/guide.html#versioning

Release artifacts:

* A tag is created for the release (e.g. ``v1.0.4``) in the main repo.

* A GitHub release is also created for convenience with the end user
  tar.xz attached to the release:

  - https://github.com/blog/1547-release-your-software

  The release title should be the same as the release description in the tag.

* The release tar.xz is added to the duktape-releases repo:

  - https://github.com/svaarala/duktape-releases

* The unpacked tar.xz is also added as a tag (on an independent branch) on
  the duktape-releases repo for convenience.  The tag is named ``vN.N.N``.
  The independent branch used to create the tag is not kept.
  See ``release-checklist.rst`` for detailed commands.

  - http://stackoverflow.com/questions/15034390/how-to-create-a-new-and-empty-root-branch

  - http://stackoverflow.com/questions/9034540/how-to-create-a-git-branch-that-is-independent-of-the-master-branch

* The releases are also available from http://duktape.org/.

Branch and tag naming
=====================

Development branches:

* ``master``: Churn branch with active development, kept close to release
  quality at all times, unstable features are developed in feature branches.

* ``frob-xyz-tweaks``, ``add-missing-docs``, etc: Relatively short lived
  branches for developing a particular feature, may be rebased, commits may
  be squashed, etc.  Merged into ``master`` when code works, documentation
  has been updated, etc and then deleted.  There is no fixed branch naming
  but avoid ``fix-`` and ``bug-`` prefixes.

* ``fix-xxx``: Short lived bug fix branch, otherwise similar to a feature
  branch.  The branch name should begin with ``fix-`` to differentiate it
  from feature development.

Maintenance branches:

* ``vN.N-maintenance``: Maintenance branch for a release, which is used to
  backport fixes.  For example, ``v1.0-maintenance`` would be used to release
  all ``v1.0.N`` releases.  A maintenance branch is branched off master just
  before an initial zero patch level release.  Release prepping should be done
  in master so that there's no need to backport release notes and such.

Release tags:

* ``vN.N.N``: Release tags.  All releases are created from a maintenance
  branch, even the zero patch level version.

Other conventions:

* Rejected branches which may be needed later are tagged so that they don't
  clutter up the branch list.  Use an annotated tag::

    $ git tag -a -m "archive rejected xyz" -s archive/rejected-xyz xyz-feature
    $ git branch -D xyz-feature

Merging
=======

All features and fixes should be developed in separate branches and merged
to master.  The only exceptions at the moment are:

* Ditz issue commits

Before merging:

* Ensure test cases pass and broken test cases are fixed to match possible
  new output.

* Ensure documentation is up-to-date, including both internal and external
  documentation.

Branches should be merged with ``--no-ff`` to avoid fast forward merges::

  $ git checkout -b frob-xyz-tweaks
  # develop...
  $ git checkout master
  $ git merge --no-ff frob-xyz-tweaks
  $ git branch -d frob-xyz-tweaks

Making fixes to maintenance branches
====================================

* Make fix to master first through a fix branch.  This includes code changes,
  testcase changes, release note update.

* Check out maintenance branch (e.g. ``v1.0-maintenance``), and git cherry pick
  fix commits from master.  Cherry pick code changes and testcase changes where
  appropriate (to allow the fix to be tested).  **Don't** update release note
  in the branch: release notes are only kept up-to-date in master.

  If a lot of commits need to be cherry picked, create a branch and merge to
  maintenance branch.

* Git cherry picking:

  - http://sleeplessgeek.blogspot.fi/2011/03/using-git-cherry-pick.html

* Basically::

    $ git cherry-pick <commit>

Commit messages
===============

Merges to master branch must have clean commit messages.  Merge commit
should retain the default merge heading which should be followed by a
descriptive paragraph similar to what the release note updates are.
This makes the merge commits useful for getting an overview what changes
have been made and why.

Commit messages should follow these guidelines:

* Capitalized title line at most 50 characters long, no trailing period.
  This works best with GitHub and is also a common convention.

* Beneath that use normal sentence structure, bullet lists etc are OK.
  No particular format for this part now.

* GitHub compatible messages are nice:

  - https://github.com/blog/926-shiny-new-commit-styles
  - http://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html

Ditz
====

Bugs, features, and tasks are tracked with Ditz, which keeps all issues
as individual YAML files in the ``bugs/`` directory.

Each Ditz issue has a UUID identifier.  The Ditz command line tool also
provides short identifiers for referring to issues on the command line
(e.g. ``ditz close duk-123``).  Note, however, that these identifiers are
**not stable**; Ditz numbers them on-the-fly.  The numbering may change
if you e.g. delete an issue manually or change an issue's component.

Conventions:

* Don't refer to issues with their short identifiers (``duk-123``) in
  documentation or code: these identifiers are not stable.  Issues can
  be referred to with their long identifiers.  Refer to issues using their
  full hash.
