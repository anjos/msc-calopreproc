# COMPILATION MACROS
# include this file in your makefiles. Doing this you'll guarantee
# the use of the same compilation flags for each object created.
#
# $Id: config.mk,v 1.3 2000/06/28 15:57:56 rabello Exp $

AR = ar
CC = gcc
RANLIB = ranlib
ARFLAGS = -rvu
CDEFS = -D_BSD_SOURCE
CFLAGS = -g -ansi -pedantic -Wall $(CDEFS)
MAKE = gmake
RM = rm -f
MV = mv -v
SED = sed
