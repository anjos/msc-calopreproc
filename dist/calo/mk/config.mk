# COMPILATION MACROS
# include this file in your makefiles. Doing this you'll guarantee
# the use of the same compilation flags for each object created.
#
# $Id: config.mk,v 1.2 2000/05/26 17:27:00 rabello Exp $

AR = ar
CC = gcc
RANLIB = ranlib
ARFLAGS = -rvu
CFLAGS = -g -ansi -pedantic -Wall
MAKE = gmake
RM = rm -f
MV = mv -v
SED = sed
