#
# $Id: Makefile,v 1.2 2000/07/07 18:24:03 rabello Exp $
#

#
# figure out architecture and OS
#
OS   = $(shell uname -s)

# the include directory
INCDIR        = ../include

#
# include arch/os specific definitions
# this fragment defines
# CC  - the C compiler
# CXX - the C++ compiler
# CFLAGS   - flags for compiling C programs
# CXXFLAGS - flags for compiling C++ programs
# LDFLAGS  - flags for linking programs
#
include ../mk/$(OS).mk

#
# we require bison and flex in any case
#
BISON      = bison
BISONFLAGS = -d
FLEX       = flex

PROGS = parser
LIBS  = libspec.a

SRCS  = grammar.tab.c lex.yy.c spec.cc codegen.cc \
        cppgen.cc cgen.cc main.cc
OBJS  = grammar.tab.o lex.yy.o spec.o codegen.o \
        cppgen.o cgen.o main.o

all:	$(PROGS) $(LIBS)

# libaray
libspec.a:	atrdmplib.o dbmalloc.o
	ar cru libspec.a $?
	-ranlib libspec.a

# parser
parser:	$(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

# grammar 
grammar.tab.h	grammar.tab.c:	grammar.y
	$(BISON) $(BISONFLAGS) $<

grammar.tab.o:	grammar.tab.c grammar.tab.h
	$(CXX) -c $(CXXFLAGS) $<

# scanner 
lex.yy.c:	scanner.l
	$(FLEX) scanner.l

lex.yy.o:	lex.yy.c
	$(CXX) -c $(CXXFLAGS) $<

scanner.l:	$(INCDIR)/spec.h grammar.tab.h
grammar.y:	$(INCDIR)/spec.h

depend:
	$(MAKEDEPEND) -I$(INCDIR) *.c *.cc > .depend

clean:
	rm -f *.o $(LIBS) $(PROGS) *~ core

real-clean:	clean
	rm -f grammar.tab.c grammar.tab.h lex.yy.c

include .depend
