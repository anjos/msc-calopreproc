#include "common.h"

/* Look for PhiMax and PhiMin, adjust and return if are in wrap around region
 */ 

const static double MaxPhiWindow = 2.0;

/* This functions examins 2 cases. 1) The RoI falls into the phi wrap region,
   but PhiMin is not greater than PhiMax. 2) The RoI falls into the phi wrap
   region and PhiMin is greater than PhiMax. */
Flag PhiWrap(double* PhiMax, double* PhiMin)
{
  if ( fabs(*PhiMax - *PhiMin) > MaxPhiWindow) { /* Yes is a PhiWrap case */
    if(*PhiMax > *PhiMin) {
      /* now we have to exchage phimax and phimin because the expected order is
       inverted. If this is not correct, wrong results may arrive. */
      double temp = *PhiMax;
      *PhiMax = PhiMin;
      *PhiMin = temp;
    }
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
