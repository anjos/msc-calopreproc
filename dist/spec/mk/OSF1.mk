#
# $Id: OSF1.mk,v 1.1 2000/03/13 21:03:45 rabello Exp $
#
# variables for OS dependent source files
# Digital Unix 4.0b
#
# using gcc and c++
#

# compiler and linker flags
#
CXX      = c++
CC       = gcc

# depending on user preference
CXXFLAGS    = -O -g -I$(INCDIR) # -Wall 
CFLAGS      = -O -g -I$(INCDIR)

# generic linker flags
LDFLAGS  = 

# make depend command
MAKEDEPEND = $(CXX) -MM $(CXX_INCLUDE_FLAGS) 

