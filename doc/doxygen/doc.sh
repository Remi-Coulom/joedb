#!/bin/bash
doxygen Doxyfile
mkdir -p html/doc/source/images
cp ../../doc/source/images/joedb.svg html/doc/source/images
