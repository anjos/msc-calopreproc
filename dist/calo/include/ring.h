/* Hello emacs, this is -*- c -*- */

/* $Id: ring.h,v 1.3 2000/07/12 04:32:21 rabello Exp $ */

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
   layer being analysed. 

   In order to do its job, this function will search for the highest energy
   value among all cells, and will continously sum the cells around the energy
   peak till there are no more cells to sum or the maximum number of rings is
   reached as described above ring.c:ring_sum_around()

   The function should the number of layers were the ringing algorithm were
   applied. The space should no be preallocated, but one must free it after
   usage. */ 
int ring_sum (const tt_roi_t*, ring_t**);

/* This function just frees a ring_t */
bool_t free_ring (ring_t*);

/* Prints the features pointed by rp in fp in line format */
void print_ring (FILE*, const ring_t*);

/* Dumps to the given file, the roi in form of rings */
void dump_ring (FILE*, const tt_roi_t*);

#endif /*_RING_H*/
