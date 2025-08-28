#!/bin/bash
cmake --install . --component bin
if [ "$(uname)" = "Darwin" ]; then
 hash -r
else
 ldconfig
fi
