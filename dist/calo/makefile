## hello emacs, this is a -*- makefile -*- 
## Andre Rabello dos Anjos <Andre.dos.Anjos@cern.ch>

## $Id: makefile,v 1.11 2000/10/23 02:25:51 andre Exp $

### Calorimeter Library compilation

### COMPILATION MACROS

AR = ar
CC = gcc
RANLIB = ranlib
ARFLAGS = -rvu
CDEFS = -D_BSD_SOURCE -D_GNU_SOURCE
###CFLAGS = -O2 -ansi -pedantic -Wall $(CDEFS)
CFLAGS = -g -ansi -pedantic -Wall $(CDEFS)
LDFLAGS = -L. -L../spec/src -lm -lcalo -lspec
MAKE = gmake
RM = rm -f
MV = mv -v
SED = sed

### GENERAL MACROS

INCLUDE = -I../../include -I../spec/include -I./include
FILES = trigtowr.c common.c portable.c ring.c uniform.c energy.c normal.c
SRCS = $(FILES:%.c=./src/%.c)
OBJS = $(SRCS:.c=.o)
DOC_DIR = ./doc

### DATA SPECIFICATION FILES
DATA_SPEC_OBJ = ../../src/data.o

### DEPENDENCY MACROS

MKDEP = makedepend
MKDEPFILE = .depend
MKDEPFLAGS = -f $(MKDEPFILE)

### The rules

.PHONY: clean depend clean_test

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDE) -c $< -o $@

all: libcalo.a

test: libcalo.a test_portable test_ring

test_%: src/test_%.o complements libcalo.a
	$(CC) $(CFLAGS) $< ../../src/data.o ../../src/util.o $(LDFLAGS) -o $@

complements:
	$(MAKE) -C ../spec/src libspec.a 
	$(MAKE) -C ../.. ./src/data.o
	$(MAKE) -C ../.. ./src/util.o

libcalo.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $?
	$(RANLIB) $@

dep:
	@if [ ! -e $(MKDEPFILE) ]; then touch $(MKDEPFILE); fi
	$(MKDEP) $(MKDEPFLAGS) $(INCLUDE) $(SRCS) src/test_*.c

clean: clean_test clean_emacs
	$(RM) $(OBJS) libcalo.a
	$(RM) $(MKDEPFILE) $(MKDEPFILE:%=%.bak)

clean_test:
	$(RM) src/test_*.o 
	$(RM) test_*

clean_emacs:
	$(RM) src/*~ include/*~ src/*~ mk/*~ *~

# The dependencies (not obligatory)
# =================================

sinclude $(MKDEPFILE)
