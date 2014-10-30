.PHONY: validate

out/sample: ../src/sample/sample.cpp out/SampleDB.h
	g++ -Wall -O2 -I out -o $@ $<

out/SampleDB.h: ../src/sample/SampleDB.xml ../src/compiler/header.xsl
	saxonb-xslt $? >$@

validate:
	xmllint --noout --dtdvalid ../src/compiler/crazydb.dtd ../src/sample/SampleDB.xml
