/* Hello emacs, this is -*- c -*- */

/* $Id: ring.h,v 1.2 2000/07/07 18:26:23 rabello Exp $ */

#ifndef _RING_H
#define _RING_H

#include <stdio.h>
#include "ttdef.h"

typedef struct ring_t 
{
  Energy* feat;
  int nfeat;
}ring_t;

/* Given an array of cells in tt_roi_t format, this function can output the
   squared rings formed by summing the rings outside the energy peak of the
   layer being analysed. For now, this function can only work with the second
   EM layer (endcap or barrel).

   In order to do its job, this function will search for the highest energy
   value among all cells, and will continously sum the cells around the energy
   peack till there are no more cells to sum.

   The function should return a vector of energy sums based on the above
   algorithm and the number of rings, encapsulated in a ring_t structure. The
   space should no be preallocated, but one must free it after usage. */
ring_t* ring_sum (const tt_roi_t*, ring_t*);

/* This function just frees a ring_t */
bool_t free_ring (ring_t*);

/* Prints the features pointed by rp in fp in line format */
void print_ring (FILE*, const ring_t*);


#endif /*_RING_H*/
