# This is a -*-sed-*- script
#
# $Id: cleandump.sed,v 1.1 2000/05/26 17:23:20 rabello Exp $
# 
# It's useful in eliminating fields from the ASCII datafiles.
# Currently, it's set to eliminate 'L2CAL' fields, but as you can
# see, this can be easily changed. The string 'L2CAL' is matched
# with no respect to the case and could be, optionally, separated
# by the begining of line or apart from its brace by any number of
# white spaces, but NO tabs.
# This script would eliminate ANY lines between and includding those
# with {L2CAL.
#
# In order to run sed (which you must have previously installed,
# just do this:
# 
# >> sed -f [this script] [old filename] > [new filename]
#
# André Rabello dos Anjos <Andre.dos.Anjos@cern.ch>

# Replace the name of L2CALEM, this is not to elim
\%[{}][ ]*L2CAL%Is/L2CALEM/TEMP/I

# Erase all fields with l2cal in it
\%{[ ]*L2CAL%I,\%}[ ]*L2CAL%ID

# Replace the TEMP entry with the original one
\%[{}][ ]*TEMP%Is/TEMP/L2CALEM/I
