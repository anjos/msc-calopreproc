/* Hello emacs, this is -*- c -*- */

/* $Id: common.c,v 1.7 2000/06/16 21:32:38 rabello Exp $ */

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

void* mxalloc(void* ptr, const int n, const int size)
{

  if( ptr == NULL ) {
    /* Ok, this is to be newly allocated memory */

    register void* value = calloc (n,size);

    if (value == 0)
      fprintf(stderr, "(common.c): [ERROR] No space for allocation.\n");

    return value;

  }
  
  /* Here, I just reallocate memory. */
  ptr = realloc (ptr,n*size);

  if( ptr == 0 )
    fprintf(stderr, "(common.c): [ERROR] No space for reallocation.\n");
  
  return(ptr);

}
