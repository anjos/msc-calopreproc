#include "common.h"

/* Look for PhiMax and PhiMin, adjust and return if are in wrap around region
 */ 

const static double MaxPhiWindow = 2.0;

Flag PhiWrap(double* PhiMax, double* PhiMin)
{
  if ( fabs(*PhiMax - *PhiMin) > MaxPhiWindow) {
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
