Release Checklist
=================

Code Tests
----------

- add tests to improve coverage if necessary
- run tests:

  - full_test.sh
  - all github actions,

- run the 3 fuzzers for a while
- upgrade in rc3, and compile_everything.sh
- make sure compilation works without precompiled headers
- test scripts

Documentation
-------------

- update VERSION with the new version number
- update doc/source/history.rst
- build and upload documentation

New release on github
---------------------

- Commit and push to github
- Go to https://github.com/Remi-Coulom/joedb/releases/new
- Tag version: "vA.B.C"
- Title: "Version A.B.C"
- Add binary packages
- Then click "publish release"
