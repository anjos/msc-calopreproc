#
# $Id: Linux.mk,v 1.3 2000/10/23 02:26:33 andre Exp $
#
# variables for OS dependent source files
# Linux 2.0.x
#
# using gcc and c++
#

# compiler and linker flags
#
CXX      = c++
CC       = gcc

# depending on user preference
###CXXFLAGS	= -O2 -I$(INCDIR)
CXXFLAGS    = -O -g -I$(INCDIR) # -Wall 
CFLAGS      = -O -g -I$(INCDIR)
### CFLAGS      = -O2 -I$(INCDIR)

# generic linker flags
LDFLAGS  = 

# make depend command
MAKEDEPEND = $(CXX) -MM $(CXX_INCLUDE_FLAGS) 

