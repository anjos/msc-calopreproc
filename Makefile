# $Id: Makefile,v 1.7 2000/05/26 18:22:55 rabello Exp $

# This makefile builds the datafile reading/writing library
# such library can be used to read ASCII data files as specified
# in $(DISTRIBUTION-HOME)/src/data.spec
# This library is composed of utilities and tests for those

# def'ing important macros
# ========================

# 1) for generation the specification files
WORKDIR = $(HOME)/work/ufrj
GEN = ./dist/spec/src/genspec # the .c/cc .h file generator (invokes parser)
DATASPEC = data # the filename to generate the .h and .c/cc for
LANG = c # the language that was chosen for the output files
SED = /bin/sed

# 2) for proper C compilation and linking
CC = gcc
CFLAGS = -g -I./include -I./dist/spec/include -I./dist/calo/include
LDFLAGS = -L./lib -L./dist/spec/src -L./dist/calo -lcalo -lspec -lm
SRCDIR = ./src
SRCFILES = util.c main.c
SRCS = $(SRCFILES:%=$(SRCDIR)/%)
OBJS = $(SRCS:%.c=%.o)

# Configuring the data file sources
# =================================

all: wrdata

testfile: version $(DATASPEC:%=./src/%.o) ./src/test.o ./src/util.o calo
	@echo --------------------------
	@echo 5\) Building executables...
	@echo --------------------------
	@echo " "
	$(CC) ./src/test.o $(DATASPEC:%=./src/%.o) ./src/util.o $(LDFLAGS) -o $@
	@echo DONE\!
	@echo " "
	@echo In case of doubts\, don\'t push it too far\, e-mail me\:
	@echo Andre Rabello dos Anjos \<Andre\.dos\.Anjos\@cern\.ch\>

wrdata: version $(DATASPEC:%=./src/%.o) $(OBJS) calo
	@echo --------------------------
	@echo 5\) Building executables...
	@echo --------------------------
	@echo " "
	$(CC) $(DATASPEC:%=./src/%.o) $(OBJS) $(LDFLAGS) -o $@
	@echo DONE\!
	@echo " "
	@echo In case of doubts\, don\'t push it too far\, e-mail me\:
	@echo Andre Rabello dos Anjos \<Andre\.dos\.Anjos\@cern\.ch\>

calo: $(DATASPEC:%=./src/%.c)
	@echo DONE\!
	@echo " "
	@echo --------------------------------
	@echo 4\) Compiling the calo library...
	@echo --------------------------------
	@cd ./dist/calo; $(MAKE)
	@echo DONE\!
	@echo " "

spec:
	@echo --------------------------------
	@echo 1\) Compiling the spec library...
	@echo --------------------------------
	@echo " "
	@cd ./dist/spec; $(MAKE)
	@cd ./dist/spec/src; chmod 755 parser
	@echo DONE\!
	@echo " "

$(DATASPEC:%=./src/%.o): $(DATASPEC:%=./src/%.c)
	@echo -----------------------------------
	@echo 3\) Compiling specification files...
	@echo -----------------------------------
	@echo " "
	$(CC) $(CFLAGS) -c $< -o $@
	@echo DONE\!
	@echo " "

$(DATASPEC:%=./src/%.c): spec $(DATASPEC:%=./src/%.spec)
	@echo ----------------------------------
	@echo 2\) Building specification files...
	@echo ----------------------------------
	@echo " "
	$(GEN) $(DATASPEC:%=./src/%.spec) $(DATASPEC) $(LANG) $(WORKDIR)/dist/spec/src/parser
	@mv $(DATASPEC:%=%.h) ./include
	@mv $(DATASPEC:%=%.c) ./src
	@echo DONE\!
	@echo " "

# The phony targets
# =================

.PHONY: clean cleanlib cleandoc cleanfig version

version:
	@echo \*
	@echo \* This file guides make\(1\) in building this package. 
	@echo \* -- current version is '$$Revision: 1.7 $$' of '$$Date: 2000/05/26 18:22:55 $$'
	@echo \* " "
	@echo \* Andre Rabello dos Anjos \<Andre\.dos\.Anjos\@cern\.ch\>
	@echo \* " "
	@echo " "

cleandoc: cleanfig
	@cd ./doc; rm -f *~

cleanfig:
	@cd ./doc/fig; rm -f *.ps *.eps *.bak *~

cleanlib:
	cd ./dist/calo; $(MAKE) clean
	cd ./dist/spec; $(MAKE) clean

clean: cleanlib
	rm -f ./src/*~ ./src/*.o $(DATASPEC:%=./src/%.[co])
	rm -f ./include/*~ $(DATASPEC:%=./include/%.h) ./*~
	rm -f wrdata testfile

dist: clean
	@echo \* Creating distribution...
	@cd ..; tar cvf - ufrj | gzip > CaloASCII-tester-`cat ufrj/VERSION`.tar.gz

sdist: clean
	@echo \* Creating small name distribution...
	@cd ..; tar cvf - ufrj | gzip > ufrj.tar.gz


