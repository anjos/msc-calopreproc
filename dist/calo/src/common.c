/* Hello emacs, this is -*- c -*- */

/* $Id: common.c,v 1.8 2000/08/16 11:21:21 andre Exp $ */

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

/* Concatenate one string to the other, returning the first. The memory needed
   is allocated as with malloc(). If *to is NULL, newly allocated memory is
   returned. The returned value is the number of chars passed to (*to) */
int ascat (char** to, const char* from)
{
  char* temp; /* a temporary stack for placing changes */
  int count; /* the number of elements put onto the new string */

  if (*to == NULL) return asprintf(to, "%s", from);

  /* else, meaning that *to is pointing somewhere, I have to concat */

  /* backup original string */
  asprintf(&temp, "%s", *to);

  /* Concat */
  count = asprintf(to, "%s %s", temp, from);

  /* free the old used string */
  free(temp);

  /* return what we promissed */
  return count;
}

/* Concatenate one string to a double, returning the first. The memory needed
   is allocated as with malloc(). If *to is NULL, newly allocated memory is
   returned. The returned value is the number of chars passed to (*to) */
int ascat_double (char** to, const double* dp)
{
  char* temp; /* a temporary stack for placing changes */
  int count; /* the number of elements put onto the new string */

  if (*to == NULL) return asprintf(to, "%e", *dp);

  /* else, meaning that *to is pointing somewhere, I have to concat */

  /* backup original string */
  asprintf(&temp, "%s", *to);

  /* Concat */
  count = asprintf(to, "%s %e", temp, *dp);

  /* free the old used string */
  free(temp);

  /* return what we promissed */
  return count;
}

/* Concatenate one string to an int, returning the first. The memory needed
   is allocated as with malloc(). If *to is NULL, newly allocated memory is
   returned. The returned value is the number of chars passed to (*to) */
int ascat_int (char** to, const int* ip)
{
  char* temp; /* a temporary stack for placing changes */
  int count; /* the number of elements put onto the new string */

  if (*to == NULL) return asprintf(to, "%d", *ip);

  /* else, meaning that *to is pointing somewhere, I have to concat */

  /* backup original string */
  asprintf(&temp, "%s", *to);

  /* Concat */
  count = asprintf(to, "%s %d", temp, *ip);

  /* free the old used string */
  free(temp);

  /* return what we promissed */
  return count;
}


