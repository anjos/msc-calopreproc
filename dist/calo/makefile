### Calorimeter Library compilation

### GENERAL MACROS

INCLUDE = -I../../include -I../spec/include -I./include
FILES = trigtowr.c flat.c common.c portable.c zstt.c calostr.c fexdef.c calo.c
SRCS = $(FILES:%.c=./src/%.c)
OBJS = $(SRCS:.c=.o)
DOC_DIR = ./doc

### COMPILATION MACROS

include ./mk/config.mk
LDFLAGS = -lm

### DEPENDENCY MACROS

MKDEP = makedepend
MKDEPFILE = .depend
MKDEPFLAGS = -f $(MKDEPFILE)

### The rules

.PHONY: clean depend

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

all: libcalo.a

libcalo.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $?
	$(RANLIB) $@

depend:
	@if [ ! -e $(MKDEPFILE) ]; then touch $(MKDEPFILE); fi
	$(MKDEP) $(MKDEPFLAGS) $(INCLUDE) $(SRCS)

clean: 
	$(RM) $(OBJS) libcalo.a src/*~ include/*~ *~ 
	$(RM) $(MKDEPFILE) $(MKDEPFILE:%=%.bak)

