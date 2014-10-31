.PHONY: validate

COMPILER:=g++ -std=c++0x -Wall -O2 -Iout -I/home/coulom/boost_1_56_0

out/sample: ../src/sample/sample.cpp out/SampleDB.h
	$(COMPILER) -o $@ $<

out/compiler: ../src/compiler/compiler.cpp
	$(COMPILER) -o $@ $<

out/SampleDB.h: ../src/sample/SampleDB.xml out/compiler
	out/compiler <$< >$@

validate:
	xmllint --noout --dtdvalid ../src/compiler/crazydb.dtd ../src/sample/SampleDB.xml
