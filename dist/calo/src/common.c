/* Hello emacs, this is -*- c -*- */

/* $Id: common.c,v 1.6 2000/05/31 12:09:36 rabello Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common.h"

double fabs(double x);

bool_t PhiWrap(double* max, double* min)  
{
  const double max_phi_window = 2.0;

  if ( fabs(*max - *min) > max_phi_window) { /* Yes, is a PhiWrap case */

    /* exchange maximum and minimum values */
    double temp = *max;
    *max = *min;
    *min = temp;

    /* increase the value of PhiMax by 2*PI adjusting it */
    *max += 2 * PI;

    return(TRUE);
  }

  return(FALSE);
}

void* SmartAlloc(void* ptr, const int size)
{
  if( ptr == NULL ) {/* first time */
    if ( (ptr = malloc(size)) == NULL ) 
      {
	fprintf(stderr, "(common.c): [ERROR] No space for allocation.\n");
	return(NULL);
      }
  }
  
  else { /* can realloc */
    if( ( ptr = realloc (ptr, size) ) == NULL ) {
      fprintf(stderr, "(common.c): [ERROR] No space for reallocation.\n");
      return(NULL);
    }
  }
  
  return(ptr);

}
