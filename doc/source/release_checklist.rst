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
- compile with most recent clang, gcc, msvc
- also try newer version of C++ standard: C++20, C++23

Documentation
-------------

- update VERSION with the new version number
- run joedb_for_FetchContent.sh, update doc/source/FetchContent_example, and
  test it.
- update doc/source/history.rst
- build and upload documentation

New Release on github
---------------------

- Commit and push to github
- Go to https://github.com/Remi-Coulom/joedb/releases/new
- Tag version: "vA.B.C"
- Title: "Version A.B.C"
- Add binary packages
- Then click "publish release"
