/* Hello emacs, this is -*- c -*- */
/* André Rabello dos Anjos <Andre.Rabello@ufrj.br> */

/* $Id: normal.c,v 1.6 2001/01/30 16:34:34 andre Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "normal.h"
#include "common.h"
#include "uniform.h"
#include "energy.h"
#include "ring.h"

/* For normalization specification. Attention: when adding a new value,
   consider analysing the whole program, since normalization flags _are_ set
   inside a short, which, generally, is an 8-bit word. */
typedef enum normal_t {NORMAL_NONE=0x0, NORMAL_ALL=0x1, NORMAL_SECTION=0x2, 
		       NORMAL_LAYER=0x4, NORMAL_UNITY=0x8,
                       NORMAL_UNITYX=0x10, NORMAL_WEIGHTED_SEG=0x20,
                       NORMAL_WEIGHTED_ALL=0x40} normal_t;

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

/* This function will take each segment with no other normalization applied
   previously and will divide the first feature by all energy on the current
   segment, the second by all energy minus the first feature and will continue
   till it reaches the last segment _or_ till reach lastring th. segment. In
   the case it reaches lastring first, the remaining features will be
   normalized by the same value of the lastring feature. Note that maximum
   number for each layer (or segment) is the number of rings on that layer.*/
void ring_weighted_seg_normalize(ringroi_t*, const config_weighted_t*);

/* Do the same as the function above, but use the total RoI energy as its
   starting point for normalization instead of the layer energy */
void ring_weighted_all_normalize(ringroi_t*, const config_weighted_t*);

/* This function actually normalizes the ringlayer information. It takes
   exactly three arguments describing, the ring to normalize (1), the energy
   that will be used as the basis of normalization (2) and in which ring to
   stop using the variable normalization scheme, starting to use a constant to
   normalize each feature. */
void ringlayer_weighted_normalize(ring_t*, const Energy*, 
				  const unsigned short*);

/******************/
/* Implementation */
/******************/

/* This function is a front end to the uniform_roi_t type. It will apply the
   correct type of normalization to the variable or dunno, if no normalization
   is demanded. */
void uniform_roi_normalize (uniform_roi_t* rp, const unsigned short* flags)
{
  register int i; /* iterator */

  /* If necessary, I normalize using ETOT */
  if ( (*flags) & NORMAL_ALL ) {
    Energy etot;
    uniform_roi_energy(rp, &etot);
    uniform_roi_scale(rp, &etot);
  }

  if ( (*flags) & NORMAL_LAYER) {
    Energy elayer;
    for(i=0; i<rp->nlayer; ++i) {
      uniform_layer_energy(&rp->layer[i], &elayer);
      uniform_layer_scale(&rp->layer[i], &elayer);
    }
  }

  if ( (*flags) & NORMAL_SECTION) {
    Energy em, had;
    uniform_roi_EM_energy(rp, &em);
    uniform_roi_HAD_energy(rp, &had);
    for(i=0; i<rp->nlayer; ++i) {
      if (rp->layer[i].calo == EM || rp->layer[i].calo == PS)
	uniform_layer_scale(&rp->layer[i], &em);
      else
	uniform_layer_scale(&rp->layer[i], &had);
    }
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
  else if ( strcasecmp(token,"layer") == 0 ) (*to)=NORMAL_LAYER;
  else if ( strcasecmp(token,"section") == 0 )  (*to)=NORMAL_SECTION;

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

  /* The next two types of are for weighted normalization, where the central
     ring (or cell) is divided by the segment or all RoI energy (depending on
     your choice) and the following rings are divided by the segment (or total
     RoI) energy minus the preceeding ring energies. This assures the the
     neighbor cells would be boosted compared to the central cell. */
  else if ( strcasecmp(token,"weighted_segment") == 0 ) {
    (*to) = NORMAL_WEIGHTED_SEG;
  }

  else if ( strcasecmp(token,"weighted_all") == 0 ) {
    (*to) = NORMAL_WEIGHTED_ALL;
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
  if (*from & NORMAL_WEIGHTED_SEG) 
    ascat(&retval,"Weigthed normalization considering segment energy");
  if (*from & NORMAL_WEIGHTED_ALL)
    ascat(&retval,"Weigthed normalization considering all RoI energy");

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

/* These next 2 functions test whether we have one of the normalizations
   described. */
bool_t normal_is_weighted_seg(const unsigned short* flags)
{
  if ( (*flags) & NORMAL_WEIGHTED_SEG ) return TRUE;
  return FALSE;
}

bool_t normal_is_weighted_all(const unsigned short* flags)
{
  if ( (*flags) & NORMAL_WEIGHTED_ALL ) return TRUE;
  return FALSE;
}

/* This function is the frontend to ringroi_t normalization. It will select the
   correct function call to apply in each occasion or dunno if asked so. Note
   that the first argument is mandatory in all situations since the process
   requires the rings to be there, also the normalization type. The third and
   fourth parameters are there for some specific methods: 
   3rd parameter -> UNITY+ normalization, indicating the normalization radius
   4th parameter -> WEIGTHED normalization, indicating the maximum ring to
   normalize with changing parameters, meaning the next rings will be
   normalized according a constant principle. */
void ring_normalize(ringroi_t* rrp, const unsigned short* flags,
		    const Energy* radiusp, const config_weighted_t* lastfeats)
{
  if (normal_is_unity(flags)) ring_unity_normalize(rrp);
  else if (normal_is_unityx(flags)) ring_unityx_normalize(rrp, radiusp);
  else if (normal_is_weighted_seg(flags)) 
    ring_weighted_seg_normalize(rrp, lastfeats);
  else if (normal_is_weighted_all(flags))
    ring_weighted_all_normalize(rrp, lastfeats);

  return;
}

/* This function will take each segment with no other normalization applied
   previously and will divide the first feature by all energy on the current
   segment, the second by all energy minus the first feature and will continue
   till it reaches the last segment _or_ till reach lastring th. segment. In
   the case it reaches lastring first, the remaining features will be
   normalized by the same value of the lastring feature. Note that maximum
   number for each layer (or segment) is the number of rings on that layer.*/
void ring_weighted_seg_normalize(ringroi_t* rrp, const config_weighted_t* cw) 
{
  int layer; /* iterators */
  Energy e; /* energy accumulator */

  /* No need to verify cw, since it was verified on startup */

  for (layer=0; layer<rrp->nring; ++layer) {
    ringlayer_energy(&rrp->ring[layer], &e);
    ringlayer_weighted_normalize(&rrp->ring[layer], &e, &cw->last2norm[layer]);
  }

  return;
}

/* Do the same as the function above, but use the total RoI energy as its
   starting point for normalization instead of the layer energy */
void ring_weighted_all_normalize(ringroi_t* rrp, const config_weighted_t* cw) 
{
  int layer; /* iterators */
  Energy e; /* energy accumulator */

  /* No need to verify cw->nlayers , since it was verified on startup */
  ringroi_energy(rrp, &e);

  for (layer=0; layer<rrp->nring; ++layer) {
    ringlayer_weighted_normalize(&rrp->ring[layer], &e, &cw->last2norm[layer]);
  }

  return;
}

/* This function actually normalizes the ringlayer information. It takes
   exactly three arguments describing, the ring to normalize (1), the energy
   that will be used as the basis of normalization (2) and in which ring to
   stop using the variable normalization scheme, starting to use a constant to
   normalize each feature. */
void ringlayer_weighted_normalize(ring_t* rp, const Energy* e, 
				  const unsigned short* last)
{
  int feature; /* iterator */
  Energy current_norm_factor = *e; /* The value that will be used for the
				      current feature normalization */
  Energy new_norm_factor; /* The value to be used to the next feature
			     normalization */
  unsigned short last_to_use = *last;

  /* 1. Verify the consistency of *last */
  if ( last_to_use > rp->nfeat) {
    /* truncate to the last */
    last_to_use = rp->nfeat;
  }

  if ( last_to_use < 1) {
    fprintf(stderr, "[ringlayer_weighted_normalize] Detected last ring to be less than 1, this I can't do\n");
    fprintf(stderr,"[ringlayer_weighted_normalize] Review your entries\n");
    exit(EXIT_FAILURE);
  }

  /* 2. Now do the calculations */
  for (feature = 0; feature < rp->nfeat; ++feature) {
    if (feature <= last_to_use) /* In this case we continue to subtract */
      new_norm_factor = current_norm_factor - rp->feat[feature];
    rp->feat[feature] /= current_norm_factor;
    current_norm_factor = new_norm_factor;
  }

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
