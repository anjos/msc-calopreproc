### Calorimeter Library compilation

### GENERAL MACROS

INCLUDE = -I../../include -I../spec/include -I./include
SRCS = trigtowr.c flat.c common.c portable.c zstt.c calostr.c fexdef.c calo.c
ADD_DIR = $(SRCS:%.c=./src/%.c)
OBJS = $(ADD_DIR:.c=.o)
DOC_DIR = ./doc

### COMPILATION MACROS

include ./mk/config.mk
LDFLAGS = -lm

### DEPENDENCY MACROS

DEPS_MAKE = all.dep clean.dep
DEPS_DIR = ../format

### MAKE LIBRARY

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

all: libcalo.a

libcalo.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $?
	$(RANLIB) $@

clean: 
	$(RM) $(OBJS) libcalo.a src/*~ include/*~

### DEPENDENCY RULES

$(DEPS_MAKE):
	$(MAKE) $(DEPS_DIR) TARGET=$(@:.dep=)

$(DEPS_DIR): FORCE
	cd $@; $(MAKE) $(TARGET)

FORCE:

