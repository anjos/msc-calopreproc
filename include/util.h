/* Hello emacs, this is -*- c -*- */

/* The utility header file. This library helps the user, providing facilities
   to dump on screen or file, tags of some ascii data files. This can be done
   according to some internal format. 

   This library also includes functions to deal with the conversion from
   strings into doubles or longs. 
*/

/* $Id: util.h,v 1.3 2000/07/12 04:49:38 rabello Exp $ */

#ifndef __UTIL_H
#define __UTIL_H

#include <stdio.h>
#include "data.h"
#include "error.h" /* This is from calo library... */

/* This function simply reads de data file in parameter 0 and tries to locate a
   VERSION tag. In case of success, it writes it to the file in parameter
   1. Else it aborts with a exit(EXIT_FAILURE). The routine will only print the
   calorimeter version number. */
int fprint_filespec(FILE*, FILE*);

/* This function counts the number of events that are present in a file and
   have at least 1 RoI. The RoI's must also have to have calorimeter digis (em
   or had) to be counted as valid RoI's. It presumes the file is not used so
   far. Meaning that it will waste all tbe initial tags of geometry (nowadays
   it's calgeom, trtgeom and sctgeom) and will count the number of events by
   loading them up. */
long count_events(FILE*);

/* This function will search (and return) the event number designated by the
   second argument. It will also waste the initial tags of this file. In the
   case the searched event has no RoI info, a warning to stderr will be
   issued. This will happen also in the case there are RoIs, but no cell
   information is present in none of them. Both warnings are distingueshable.*/
EVENT search_event(FILE*, const long);

/* This funtion just returns a valid long based on a string. It's an
   implemenatation of atol(), but with the verification of strtol(). */
long to_valid_long(const char*);

/* This funtion just returns a valid long based on a string. It's an
   implemenatation of atol(), but with the verification of strtol(). */
double to_valid_double(const char*);

/* Waste all initial crap */
void waste_initial_info(FILE*);

/* Dumps all the digis in an RoI in human readable format */
void dump_DIGIS(FILE* fp,const ROI* roi);

#endif /* __UTIL_H */








