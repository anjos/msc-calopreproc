/* Hello emacs, this is -*- c -*- */
/* André Rabello dos Anjos <Andre.Rabello@ufrj.br> */

/* $Id: normal.c,v 1.4 2000/09/19 00:32:58 andre Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "normal.h"
#include "common.h"
#include "uniform.h"
#include "energy.h"
#include "ring.h"

/* For normalization specification */
typedef enum normal_t {NORMAL_NONE=0x0, NORMAL_ALL=0x1, NORMAL_SECTION=0x2, 
		       NORMAL_LAYER=0x4, NORMAL_UNITY=0x10,
                       NORMAL_UNITYX=0x20} normal_t;

/***************/
/* Prototyping */
/***************/
/* Just divides all energy values on the pointer to the uniform RoI by the
   energy value contained in nfactor. OBS: the uniform RoI is changed during
   this call. The other function (uniform_layer_scale()) is just a helper
   function for this one. */
void uniform_roi_scale (uniform_roi_t*, const Energy*);
void uniform_layer_scale (CaloLayer*, const Energy*);

/* This function calculates the modulus of a vector composed of all elements of
   a ringroi_t. This is, the square root of the sum of squares of the ring
   roi. The function will return on the modulus pointer (that should be
   pre-allocated) the value calculated. Such pointer is also returned to the
   caller. */
Energy* ring_modulus (const ringroi_t*, Energy*);

/* This function evaluates the sum of squares of all RoI ring features. This
   value is returned on the space pointed by the second argument, that should
   be pre-allocated (we suggest static allocation on the caller). The first
   argument is the ringroi_t* pointer, as expected. The returned pointer is the
   same as the second argument. */
Energy* ring_square_sum (const ringroi_t*, Energy*);

/* This function divides each value of the ringroi_t pointed by the first
   argument by the second argument, which should be greater than zero, of
   course. */
void ring_scale (ringroi_t*, const Energy*);

/* This function will input the ring roi pointer (1st. argument), will evaluate
   the norm using all ring elements and them divide each element of the
   ringroi_t so that the sqrt of sum of squares evaluates 1.*/
void ring_unity_normalize (ringroi_t*);

/* This function will input the ring roi pointer (1st. argument) and the second
   argument (the sphere radius), and will create a
   "number-of-ringroi-features-plus-one" variable that will do as the extra
   normalization variable, for not loosing separation information. This
   variable is calculated targeting the sphere radius passed as the second
   argument to this function. Therefore, the sum squares of all features plus
   the square of this extra feature, will have to be equal to the square of
   the second argument. After evaluating this variable, we can proceed with
   normal modulus normalization using ring_unity_normalize(). */
void ring_unityx_normalize (ringroi_t*, const Energy*);


/******************/
/* Implementation */
/******************/

/* This function is a front end to the uniform_roi_t type. It will apply the
   correct type of normalization to the variable or dunno, if no normalization
   is demanded. */
void uniform_roi_normalize (uniform_roi_t* rp, const unsigned short* flags)
{
  /* If necessary, I normalize using ETOT */
  if ( (*flags) & NORMAL_ALL ) {
    Energy etot;
    uniform_roi_energy(rp, &etot);
    uniform_roi_scale(rp, &etot);
  }

  return;
}

/* Just divides all energy values on the pointer to the uniform RoI by the
   energy value contained in nfactor. OBS: the uniform RoI is changed during
   this call. */
void uniform_roi_scale (uniform_roi_t* rp, const Energy* factor)
{
  int i; /* iterator */

  /* Now, if necessary, I normalize */
  for (i=0; i< rp->nlayer; ++i) 
    uniform_layer_scale(&rp->layer[i], factor);

  return;
}

/* Do the same as uniform_roi_normalize(), but on CaloLayers */
void uniform_layer_scale (CaloLayer* lp, const Energy* factor)
{
  int eta,phi;
  int cell_index;

  for(phi=0; phi < lp->PhiGran; ++phi)
    for(eta=0; eta < lp->EtaGran; ++eta) {
      cell_index = eta + phi* lp->EtaGran;
      lp->cell[cell_index].energy /= (*factor);
  }

  return;
}

/* Although simple, its hacked for future upgrades, that's why the complexity
   of it. */
unsigned short* string2normalization(unsigned short* to, const char* from)
{
  char* token;
  const char delimiters [] = " ,";

  /* copies the initial string, not to alter it with a strtok() call */
  char* temp = strdup (from);

  /* Well, if you had something coded on to, say goodbye */
  (*to) = 0;

  if ( temp == NULL ) {
    fprintf(stderr, "(uniform)ERROR: Can't copy string on");
    fprintf(stderr, "string2normalization()\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(temp,delimiters);

  if ( strcasecmp(token,"all") == 0 ) (*to) = NORMAL_ALL;

  else if ( strcasecmp(token,"layer") == 0 ) { /* (*to)=NORMAL_LAYER; */
    fprintf(stderr, "(uniform)WARN: layer normalization not implemented\n");
    fprintf(stderr, "(uniform)WARN: switching to all\n");
    (*to) = NORMAL_ALL;
  }

  else if ( strcasecmp(token,"section") == 0 )  { /* (*to)=NORMAL_SECTION; */
    fprintf(stderr, "(uniform)WARN: section normalization not ");
    fprintf(stderr, "implemented\n");
    fprintf(stderr, "(uniform)WARN: switching to all\n");
    (*to) = NORMAL_ALL;
  }

  /* When normalizing for unity modulus, we have to preprocess using etot
     normalization and after that, proceed to find the modulus and divide all
     the rings using it. */
  else if ( strcasecmp(token,"unity") == 0 ) {
    (*to) = NORMAL_ALL | NORMAL_UNITY;
  }

  /* When normalizing for unity modulus on this category, we include an extra
     variable, so the clusters formed won't loose separation information, as
     expected in some 2-D cases. For instance, if 2 clusters have the same
     direction for their mean vector, then, when projecting to the unity circle
     they will superimpose. Using an extra variable and projecting both
     clusters into the envolving sphere, the clusters, if not superimposing on
     the plane won't superimpose on the projected sphere. This is a trick not
     to loose separation information, that may be hidden into N-dimensional
     events, where we cannot visualize a probable natural cluster
     separation. We remind the code reader, that for unity+ normalization, the
     events (actually, RoIs) must be pre-normalized in energy. */
  else if ( strcasecmp(token,"unity+") == 0 ) {
    (*to) = NORMAL_ALL | NORMAL_UNITYX;
  }

  else if ( strcasecmp(token,"none") == 0 ) (*to) = NORMAL_NONE;
  else {
    fprintf(stderr, "(uniform)WARN: valid token? -> %s\n", token);
    fprintf(stderr, "(uniform)WARN: switching to no normalization\n");
    (*to) = NORMAL_NONE;
  }
  
  /* Can't forget to free the space I've allocated... */
  free(temp);

  return (to);
}

char* normalization2string(const unsigned short* from, char* to)
{
  char* retval; /* the place where we're going to put the output description */
  retval = NULL;

  /* Check if I have to return nothing */
  if (*from == 0) {
    strcpy(to, "None");
    return to;
  }

  if (*from & NORMAL_ALL) ascat(&retval,"Total RoI Energy ");
  if (*from & NORMAL_SECTION) ascat(&retval,"Section Energy (EM/HAD) ");
  if (*from & NORMAL_LAYER) ascat(&retval,"Layer Energy ");
  if (*from & NORMAL_UNITY) ascat(&retval,"Unity modulus ");
  if (*from & NORMAL_UNITYX) 
    ascat(&retval,"Unity modulus (w/ extra variable) ");

  strncpy(to,retval,59);
  free(retval);
  return to;
}

/* This function returns true if the unity normalization flag is active,
   meaning that ring-unity-normalization will be applied. */
bool_t normal_is_unity(const unsigned short* flags)
{
  if ( (*flags) & NORMAL_UNITY ) return TRUE;
  return FALSE;
}

/* This function returns true if the unity+ normalization flag is active,
   meaning that ring-unity-normalization will be applied. */
bool_t normal_is_unityx(const unsigned short* flags)
{
  if ( (*flags) & NORMAL_UNITYX ) return TRUE;
  return FALSE;
}

/* This function is the frontend to ringroi_t normalization. It will select the
   correct function call to apply in each occasion or dunno if asked so. */
void ring_normalize(ringroi_t* rrp, const unsigned short* flags,
		    const Energy* radiusp)
{
  if (normal_is_unity(flags)) ring_unity_normalize(rrp);
  else if (normal_is_unityx(flags)) ring_unityx_normalize(rrp, radiusp);

  return;
}

/* This function will input the ring roi pointer (1st. argument), will evaluate
   the norm using all ring elements and them divide each element of the
   ringroi_t so that the sqrt of sum of squares evaluates 1.*/
void ring_unity_normalize (ringroi_t* rrp)
{
  Energy modulus;

  /* get the modulus of the ringroi_t */
  ring_modulus(rrp, &modulus);

  /* divide each cell by this predefined value */
  ring_scale(rrp, &modulus);

  return;
}

/* This function will input the ring roi pointer (1st. argument) and the second
   argument (the sphere radius), and will create a
   "number-of-ringroi-features-plus-one" variable that will do as the extra
   normalization variable, for not loosing separation information. This
   variable is calculated targeting the sphere radius passed as the second
   argument to this function. Therefore, the sum squares of all features plus
   the square of this extra feature, will have to be equal to the square of
   the second argument. After evaluating this variable, we can proceed with
   normal modulus normalization using ring_unity_normalize().
   ATTENTION: If the value given as radius is less then sum of squares, an
   error message should be reported to output since the extra variable will
   evaluate to a complex! */
void ring_unityx_normalize (ringroi_t* rrp, const Energy* radiusp)
{
  Energy sq_sum; /* The sum of squares of the current rrp */
  Energy extra; /* The extra variable to be created */

  /* Evaluate the sum of squares of the ringroi */
  ring_square_sum(rrp, &sq_sum);

  /* This will cover the "ATTENTION" section documented above. */
  if ( (pow((*radiusp),2) < sq_sum) ) {
    fprintf(stderr, "(normal) ERROR: Normalization using unity+ technique\n");
    fprintf(stderr, "(normal)   has detected a sum of squares greater than\n");
    fprintf(stderr, "(normal)   the square radius given (%e): %e\n", 
	    pow((*radiusp),2), sq_sum);
    exit(EXIT_FAILURE);
  }

  /* The extra variable is the complement to square-sum r² */
  extra = sqrt( pow((*radiusp),2) - sq_sum );
  
  /* Now, we add the extra variable to the ringroi, creating a new ring */
  rrp->ring = (ring_t*) realloc(rrp->ring, (rrp->nring+1)*sizeof(ring_t));
  ++rrp->nring;

  rrp->ring[rrp->nring-1].feat = (Energy*) calloc(1, sizeof(Energy));
  rrp->ring[rrp->nring-1].nfeat = 1;
  
  /* And finally, we copy the calculated value into that newly allocated space
   */ 
  rrp->ring[rrp->nring-1].feat[0] = extra;

  /* Now, we do normal ring normalization with that changed ringroi structure
   */ 
  ring_unity_normalize(rrp);

  /* And return gracefully */
  return;
}


/* This function calculates the modulus of a vector composed of all elements of
   a ringroi_t. This is, the square root of the sum of squares of the ring
   roi. The function will return on the modulus pointer (that should be
   pre-allocated) the value calculated. Such pointer is also returned to the
   caller. */
Energy* ring_modulus (const ringroi_t* rrp, Energy* modulus)
{
  /* evaluate the square sum, putting results on (*modulus) */
  ring_square_sum(rrp, modulus);

  /* take the sqrt of that and we're ready! */
  (*modulus) = sqrt(*modulus);

  return modulus;
}

/* This function evaluates the sum of squares of all RoI ring features. This
   value is returned on the space pointed by the second argument, that should
   be pre-allocated (we suggest static allocation on the caller). The first
   argument is the ringroi_t* pointer, as expected. The returned pointer is the
   same as the second argument. */
Energy* ring_square_sum (const ringroi_t* rrp, Energy* squaresum)
{
  int layer; /* iterator */  
  int feature; /* iterator */

  /* if you had anything there, say goodbye */
  (*squaresum) = 0.;

  for (layer = 0; layer < rrp->nring; ++layer)
    for (feature = 0; feature < rrp->ring[layer].nfeat; ++feature)
      (*squaresum) += pow(rrp->ring[layer].feat[feature],2);
  
  return squaresum;
}


/* This function divides each value of the ringroi_t pointed by the first
   argument by the second argument, which should be greater than zero, of
   course. */
void ring_scale (ringroi_t* rrp, const Energy* value)
{
  int layer, feature; /* iterators */
  
  if ((*value) <= 0) {
    fprintf(stderr, "(normal)ERROR: Normalization value should be ");
    fprintf(stderr, "greater than zero\n");
    exit;
  }

  for (layer = 0; layer < rrp->nring; ++layer)
    for (feature = 0; feature < rrp->ring[layer].nfeat; ++feature)
      rrp->ring[layer].feat[feature] /= (*value);
  
  return;
}
