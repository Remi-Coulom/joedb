#!/bin/bash
#https://github.com/OfekShilon/optview2
~/repos/optview2/opt-viewer.py -j4 --output-dir clang_release/optview2 --source-dir ./clang_release ./clang_release/CMakeFiles
echo xdg-open clang_release/optview2/index.html
