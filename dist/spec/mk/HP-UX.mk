#
# $Id: HP-UX.mk,v 1.2 2000/07/07 18:24:02 rabello Exp $
#
# variables for OS dependent source files
# HP/UX
#
# using vendor compilers
#

# compiler and linker flags
#
CXX      = aCC
CC       = cc 

# depending on user preference
CXXFLAGS    = -z +p -Wc,-ansi_for_scope,on -Wc,-nrv_optimization,on +O2 +Onoinitcheck -I$(INCDIR)
CFLAGS      = -Aa -D_HPUX_SOURCE -Wp,-H256000 -z +w1 -O -Wp,-Mt2scfexlib.u -I$(INCDIR)

# generic linker flags
LDFLAGS  = 

# don't know if hp has something similar
MAKEDEPEND = g++ -MM
