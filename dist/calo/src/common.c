/* $Id: common.c,v 1.5 2000/04/07 18:58:43 rabello Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common.h"

double fabs(double x);

const static double MaxPhiWindow = 2.0;

/* The L1 RoI selector, may mark the value of PhiMax as being the max value of
   phi among the corners of the RoI or the one in the end following the
   counter-clockwise direction of the RoI along phi. This functions examins the
   2 cases and correct the RoI phi values: 1st. case) If the RoI falls into the
   phi wrap region, but PhiMin is not greater than PhiMax. 2nd. case) If the
   RoI falls into the phi wrap region and PhiMin is greater than PhiMax. In
   both cases, the function adjusts PhiMin to be the greatest of the two values
   and add 2*PI to PhiMax. This way, PhiMax goes to a value, greater than
   PhiMin, which would not happen if this verification was not done here. */
Flag PhiWrap(double* PhiMax, double* PhiMin)
{
  if ( fabs(*PhiMax - *PhiMin) > MaxPhiWindow) { /* Yes is a PhiWrap case */
    if(*PhiMax > *PhiMin) {
      /* now we have to exchage phimax and phimin because the expected order is
       inverted. If this is not correct, wrong results may arrive. */
      double temp = *PhiMax;
      *PhiMax = *PhiMin;
      *PhiMin = temp;
    }
    /* increase the value of PhiMax by 2*PI adjusting it */
    *PhiMax = *PhiMax + 2 * PI;
    return(ON);
  }

  return(OFF);
}

void* SmartAlloc(void* ptr, const int size)
{
  if( ptr == NULL ) {/* first time */
    if ( (ptr = malloc(size)) == NULL ) 
      {
	fprintf(stderr, "ERROR(common.c): No space for allocation\n");
	return(NULL);
      }
  }
  
  else { /* can realloc */
    if( ( ptr = realloc (ptr, size) ) == NULL ) {
      fprintf(stderr, "ERROR(common.c): No space for reallocation\n");
      return(NULL);
    }
  }
  
  return(ptr);

}
