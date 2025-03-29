#!/bin/bash
export JOEDB_VERSION=$(tr -d '"' <../../VERSION)
mkdir -p ../build/html
doxygen Doxyfile
