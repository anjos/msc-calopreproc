/* Hello emacs, this is -*- c -*- */
/* André Rabello dos Anjos <Andre.Rabello@ufrj.br> */

/* $Id: normal.c,v 1.2 2000/09/06 21:13:07 rabello Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "normal.h"
#include "common.h"
#include "uniform.h"
#include "energy.h"

/* For normalization specification */
typedef enum normal_t {NORMAL_NONE=0x0, NORMAL_ALL=0x1, NORMAL_SECTION=0x2, 
		       NORMAL_LAYER=0x4, NORMAL_UNITY=0x10} normal_t;

/***************/
/* Prototyping */
/***************/
/* Just divides all energy values on the pointer to the uniform RoI by the
   energy value contained in nfactor. OBS: the uniform RoI is changed during
   this call. The other function (uniform_layer_scale()) is just a helper
   function for this one. */
void uniform_roi_scale (uniform_roi_t*, const Energy*);
void uniform_layer_scale (CaloLayer*, const Energy*);

/* This function will input the ring roi pointer (1st. argument), will evaluate
   the norm using all ring elements and them divide each element of the
   ringroi_t so that the sqrt of sum of squares evaluates 1.*/
void ring_unity_normalize (ringroi_t*);

/* This function calculates the modulus of a vector composed of all elements of
   a ringroi_t. This is, the square root of the sum of squares of the ring
   roi. The function will return on the modulus pointer (that should be
   pre-allocated) the value calculated. Such pointer is also returned to the
   caller. */
Energy* ring_modulus (const ringroi_t*, Energy*);

/* This function divides each value of the ringroi_t pointed by the first
   argument by the second argument, which should be greater than zero, of
   course. */
void ring_scale (ringroi_t*, const Energy*);

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

  strncpy(to,retval,59);
  free(retval);
  return to;
}

/* This function is the frontend to ringroi_t normalization. It will select the
   correct function call to apply in each occasion or dunno if asked so. */
void ring_normalize(ringroi_t* rrp, const unsigned short* flags)
{
  if ( (*flags) & NORMAL_UNITY ) {
    ring_unity_normalize(rrp);
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

/* This function calculates the modulus of a vector composed of all elements of
   a ringroi_t. This is, the square root of the sum of squares of the ring
   roi. The function will return on the modulus pointer (that should be
   pre-allocated) the value calculated. Such pointer is also returned to the
   caller. */
Energy* ring_modulus (const ringroi_t* rrp, Energy* modulus)
{
  int layer, feature; /* iterators */
  
  (*modulus) = 0.; /* if had anything there, say goodbye:) */

  for (layer = 0; layer < rrp->nring; ++layer)
    for (feature = 0; feature < rrp->ring[layer].nfeat; ++feature)
      (*modulus) += pow(rrp->ring[layer].feat[feature],2);

  (*modulus) = sqrt(*modulus);

  return modulus;
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
