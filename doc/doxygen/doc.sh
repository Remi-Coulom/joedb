#!/bin/bash
export JOEDB_VERSION=$(tr -d '"' <../../VERSION)
export JOEDB_MANUAL=https://www.remi-coulom.fr/joedb
doxygen Doxyfile
mkdir -p html/doc/source/images
cp ../../doc/source/images/joedb.svg html/doc/source/images
