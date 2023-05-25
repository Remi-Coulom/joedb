Release Checklist
=================

Code Tests
----------

- add tests to improve coverage if necessary
- run tests:

  - all github actions,
  - cygwin

- run the 3 fuzzers for a while

Documentation
-------------

- update VERSION with the new version number
- update doc/source/history.rst

New release on github
---------------------

- Commit and push to github
- Go to https://github.com/Remi-Coulom/joedb/releases/new
- Tag version: "vA.B.C"
- Title: "Version A.B.C"
- Add binary packages
- Then click "publish release"
