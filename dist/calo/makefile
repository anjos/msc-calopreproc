## hello emacs, this is a -*- makefile -*- 
## Andre Rabello dos Anjos <Andre.dos.Anjos@cern.ch>

## $Id: makefile,v 1.5 2000/06/28 16:06:30 rabello Exp $

### Calorimeter Library compilation

### GENERAL MACROS

INCLUDE = -I../../include -I../spec/include -I./include
FILES = trigtowr.c flat.c common.c \
	portable.c zstt.c calostr.c fexdef.c calo.c ring.c
SRCS = $(FILES:%.c=./src/%.c)
OBJS = $(SRCS:.c=.o)
DOC_DIR = ./doc

### DATA SPECIFICATION FILES
DATA_SPEC_OBJ = ../../src/data.o

### COMPILATION MACROS

include ./mk/config.mk
LDFLAGS = -L. -L../spec/src -lm -lcalo -lspec

### DEPENDENCY MACROS

MKDEP = makedepend
MKDEPFILE = .depend
MKDEPFLAGS = -f $(MKDEPFILE)

### The rules

.PHONY: clean depend

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

all: libcalo.a

test: libcalo.a test_portable

test_%: src/test_%.o complements libcalo.a
	$(CC) $(CFLAGS) $< ../../src/data.o ../../src/util.o $(LDFLAGS) -o $@

complements:
	$(MAKE) -C ../spec/src libspec.a 
	$(MAKE) -C ../.. ./src/data.o
	$(MAKE) -C ../.. ./src/util.o

libcalo.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $?
	$(RANLIB) $@

depend:
	@if [ ! -e $(MKDEPFILE) ]; then touch $(MKDEPFILE); fi
	$(MKDEP) $(MKDEPFLAGS) $(INCLUDE) $(SRCS) src/test_*.c

clean: 
	$(RM) $(OBJS) libcalo.a src/*~ include/*~ *~  test_*
	$(RM) $(MKDEPFILE) $(MKDEPFILE:%=%.bak)
	$(RM) src/test_portable.o

# The dependencies (not obligatory)
# =================================

sinclude $(MKDEPFILE)



