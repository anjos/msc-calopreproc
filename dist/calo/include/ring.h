/* Hello emacs, this is -*- c -*- */

/* $Id: ring.h,v 1.9 2000/10/23 02:24:30 andre Exp $ */

#ifndef _RING_H
#define _RING_H

#include <stdio.h>
#include "ttdef.h"
#include "uniform.h"

/* defines how rings are going to be arranged */
typedef struct ring_t 
{
  Energy* feat;
  int nfeat;
}ring_t;

/* defines how the rings are going to be arranged globally */
typedef struct ringroi_t
{
  ring_t* ring;
  int nring;
}ringroi_t;

/* The next type configures the limit for each segment in terms of weighted
   normalization. After the configured limit, the remaining features are
   normalized using a constant factor. As stated by Igor's work. This has to be
   public, so the user can use it for configuration purposes. I had to put this
   thing over here, so both ring.c and normal.h can use it... No other way */
typedef struct config_weighted_t
{
  unsigned short nlayers;
  unsigned short* last2norm;
}config_weighted_t;

/* Given an array of cells in uniform_roi_t format, this function can output
   the squared rings formed by summing the rings outside the energy peak of the
   layer being analysed. The RoI on it's uniformized state (by uniform.c)
   should be passed along with a place to put the ringroi_t malloc'ed inside
   and options for printing and normalization on that order.

   In order to do its job, this function will search for the highest energy
   value among all cells, and will continously sum the cells around the energy
   peak till there are no more cells to sum or the maximum number of rings is
   reached as described above ring.c:ring_sum_around(). The layers to be
   included in processing and normalization type are specified by the unsigned
   shorts in the end (last argument), as describe in module uniform.[ch].

   The function should return the number of layers were the ringing algorithm
   were applied. The ringroi_t.ring pointer will be directed to some space
   containing the extracted rings. The space should not be preallocated, but
   one must free it after usage. The actual space for ringroi_t SHOULD be
   preallocated. I suggest using static local allocation.

   This function will also apply the normalization (if needed) to the newly
   formed ring structure of values. For that, the fifth argument should be
   passed with the value of the maximum radius value for 'unity+'
   normalization, as describe by the 'normal' module. The last argument will be
   used in the case of weighted normalization, as exposed in Igor's work. It's
   a structured described in normal.h that contains the last rings that will
   have varying normalization factors. */
int ring_sum (const uniform_roi_t*, ringroi_t*, const unsigned short*,
	      const unsigned short*, const Energy*, const config_weighted_t*);

/* This function just frees a ring_t */
bool_t free_ring (ring_t*);

/* Prints the features pointed by rp in fp in line format. The integer returned
   is the total number of features printed to file. */
int fprintf_ring (FILE*, const ring_t*); 

/* This functions prints the ring vector pointed by rp and containing nring
   elements (3rd. argument). The output goes to the file pointed by arg#1. The
   integer returned represents the number of fields dumped to the pointed
   file. */
int fprintf_ring_vector (FILE*, const ring_t*, const int);

/* This function will print the contents of the ring vector pointed by *rp and
   with nring rings in a dinamically allocated string instead of a file. It
   will return such string as the first argument. */
int asprintf_ring_vector (char**, const ring_t*, const int);

/* This function will print the contents of *rp in a dinamically allocated
   string instead of a file. The string is pointed by the first argument. It
   will return such string. */
int asprintf_ring (char**, const ring_t*);

/* This function frees a vector of rings pointed by r and having nring 
   elements. */ 
bool_t free_ring_vector (ring_t*, const int);

/* Calculates the energy in a ring layer, returning the result on *e */
void ringlayer_energy(const ring_t*, Energy*);

/* Calculates the total RoI energy, based on rings only */
void ringroi_energy(const ringroi_t*, Energy*);

#endif /*_RING_H*/  





