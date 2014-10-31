#COMPILER:=g++ -Wall -O2 -Iout
COMPILER:=clang++ -Wall -std=c++11 -O0 -Iout

out/sample: ../src/sample/sample.cpp out/sample_db.h
	$(COMPILER) -o $@ $<

out/compiler: ../src/compiler/compiler.cpp
	$(COMPILER) -o $@ $<

out/sample_db.h: ../src/sample/sample_db.xml out/compiler
	out/compiler <$< >$@

.PHONY: validate clean

clean:
	~/rc2/programs/_general/clean.sh

validate:
	xmllint --noout --dtdvalid ../src/compiler/crazydb.dtd ../src/sample/sample_db.xml
