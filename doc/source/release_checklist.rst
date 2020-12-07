Release Checklist
=================

Code Tests
----------

- add tests to improve coverage if necessary
- run tests in Windows, Linux, and PowerPC virtual machine
- run the 3 fuzzers for a while

Documentation
-------------

- update VERSION with the new version number
- update doc/source/history.rst

Packaging
---------

- Linux: cpack -G DEB
- Windows: zip of renamed installed directory to joedb-VERSION-Windows-x64
- test deb package installation (on newly installed machine)

New release on github
---------------------

- Commit and push to github
- Go to https://github.com/Remi-Coulom/joedb/releases/new
- Tag version: "vA.B.C"
- Title: "Version A.B.C"
- Add binary packages
- Then click "publish release"
