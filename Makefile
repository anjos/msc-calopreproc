# $Id: Makefile,v 1.20 2000/09/11 23:08:09 andre Exp $

# This makefile builds the datafile reading/writing library
# such library can be used to read ASCII data files as specified
# in $(DISTRIBUTION-HOME)/src/data.spec

# def'ing important macros
# ========================

#############################################
# These are the variables you can configure #
#############################################

# The base name where you decompressed the main archive
WORKBASEDIR = $(HOME)/work

# The name of the distribution root directory
MODULE = ufrj


###########################################################################
# From now on you shouldn't change the Makefile, unless you know what you #
# are doing.								  #
###########################################################################

# 1) for generation the specification files
WORKDIR = $(WORKBASEDIR)/$(MODULE)
GEN = ./dist/spec/src/genspec # the .c/cc .h file generator (invokes parser)
DATASPEC = data # the filename to generate the .h and .c/cc for
LANG = c # the language that was chosen for the output files
SED = /bin/sed

### DEPENDENCY MACROS

MKDEP = makedepend
MKDEPFILE = .depend
MKDEPFLAGS = -f $(MKDEPFILE)

# 2) for proper C compilation and linking
CC = gcc 
CFLAGS =  -ansi -pedantic -Wall -g $(INCLUDE)
#CPPFLAGS = -DTRACE_DEBUG
CPPFLAGS = -D_GNU_SOURCE
INCLUDE = -I./include -I./dist/spec/include -I./dist/calo/include
LDFLAGS=-L./dist/spec/src -L./dist/calo -lcalo -lspec -lm #-lefence
SRCDIR = ./src
SRCFILES = util.c main.c parameter.c
SRCS = $(SRCFILES:%=$(SRCDIR)/%)
OBJS = $(SRCS:%.c=%.o)

# Configuring the data file sources
# =================================

all: preproc docs

depend: version spec $(DATASPEC:%=./src/%.c)
	@echo -------------------------
	@echo Creating dependencies ...
	@echo -------------------------
	@echo " "
	@cd dist/calo; $(MAKE) depend
	@cd dist/spec/src; $(MAKE) depend
	@if [ ! -e $(MKDEPFILE) ]; then touch $(MKDEPFILE); fi 
	$(MKDEP) $(MKDEPFLAGS) $(INCLUDE) $(SRCS) ./src/data.c

preproc: version spec $(DATASPEC:%=./src/%.o) $(OBJS) calo
	@echo -----------------------
	@echo Building executables...
	@echo -----------------------
	@echo " "
	$(CC) $(DATASPEC:%=./src/%.o) $(OBJS) $(LDFLAGS) -o $@
	@echo DONE\!
	@echo " "
	@echo In case of doubts\, don\'t push it too far\, e-mail me\:
	@echo Andre Rabello dos Anjos \<Andre\.Rabello\@ufrj\.br\>

calo: $(DATASPEC:%=./src/%.c)
	@echo DONE\!
	@echo " "
	@echo -----------------------------
	@echo Compiling the calo library...
	@echo -----------------------------
	@cd ./dist/calo; $(MAKE)
	@echo DONE\!
	@echo " "

spec:
	@echo -----------------------------
	@echo Compiling the spec library...
	@echo -----------------------------
	@echo " "
	@cd $(WORKDIR)/dist/spec; $(MAKE)
	@cd $(WORKDIR)/dist/spec/src; chmod 755 parser
	@echo DONE\!
	@echo " "

# Doing:
# ./src/data.o: ./src/data.c
$(DATASPEC:%=./src/%.o): $(DATASPEC:%=./src/%.c)
	@echo --------------------------------
	@echo Compiling specification files...
	@echo --------------------------------
	@echo " "
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	@echo DONE\!
	@echo " "

# Doing:
# ./src/data.c: ./src/data.spec
$(DATASPEC:%=./src/%.c): $(DATASPEC:%=./src/%.spec)
	@echo -------------------------------
	@echo Building specification files...
	@echo -------------------------------
	@echo " "
	@echo -n Checking genspec permissions...
	@if [ ! -x $(WORKDIR)/dist/spec/src/genspec ]; then \
          chmod 755 $(WORKDIR)/dist/spec/src/genspec; \
	 fi
	@echo done.
	$(GEN) $< $(DATASPEC) $(LANG) $(WORKDIR)/dist/spec/src/parser
	@mv $(DATASPEC:%=%.h) ./include
	@mv $(DATASPEC:%=%.c) ./src
	@echo DONE\!
	@echo " "

docs:
	@echo \* Building documentation files...
	@cd ./doc; $(MAKE) $(MAKEFLAGS) all

# The phony targets
# =================

.PHONY: clean cleanlib cleandoc version dist sdist

version:
	@echo \*
	@echo \* This file guides make\(1\) in building this package. 
	@echo \* -- current version is '$$Revision: 1.20 $$' of '$$Date: 2000/09/11 23:08:09 $$'
	@echo \* " "
	@echo \* Andre Rabello dos Anjos \<Andre\.dos\.Anjos\@cern\.ch\>
	@echo \* " "
	@echo -n \* Note: Do \*not\* forget to set the WORKDIR variable or
	@echo " things" may not go smooth.
	@echo " "

cleandoc:
	@cd ./doc; $(MAKE) clean

cleanlib:
	cd ./dist/calo; $(MAKE) clean
	cd ./dist/spec; $(MAKE) clean

clean: cleanlib cleandoc
	rm -f ./src/*~ ./src/*.o $(DATASPEC:%=./src/%.[co])
	rm -f ./include/*~ $(DATASPEC:%=./include/%.h) ./*~
	rm -f $(MKDEPFILE) $(MKDEPFILE:%=%.bak)
	rm -f preproc TAGS

dist: clean
	@echo \* Creating distribution. Current date will be written on DATE...
	@echo \* Today is `date +%A,\ %d\ of\ %B\ of\ %Y`
	@echo `date` > DATE
	@cd ..;
	 tar cvf - $(MODULE) | gzip > $(MODULE)-`cat $(WORKDIR)/VERSION`.tar.gz

shot: clean
	@echo \* Creating a snapshot of today\'s source...
	@echo \* The date will be written on DATE.
	@echo \* Today is `date +%A,\ %d\ of\ %B\ of\ %Y`
	@echo `date` > DATE
	@cd ..; tar cvf - $(MODULE) | gzip > $(MODULE)-`date +%Y.%m.%d`.tar.gz

tags:
	@echo \* Creating TAGS file...
	@find . -name '*.[ch]' | etags -

# The dependencies (not obligatory)
# =================================

sinclude $(MKDEPFILE)

