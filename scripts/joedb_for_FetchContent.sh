#!/bin/bash
set -e
cd ..
git archive -9 --prefix=joedb/ --format=tar.gz HEAD . ":(exclude)doc" ":(exclude)test" ":(exclude)syntax" ":(exclude).git*" ":(exclude)benchmark" >joedb_$(tr -d '"' <VERSION).tar.gz
