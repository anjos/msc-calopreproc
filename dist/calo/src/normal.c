/* Hello emacs, this is -*- c -*- */
/* André Rabello dos Anjos <Andre.Rabello@ufrj.br> */

/* $Id: normal.c,v 1.1 2000/09/06 14:58:33 andre Exp $ */

#include <stdlib.h>
#include <stdio.h>

#include "normal.h"
#include "common.h"
#include "uniform.h"
#include "energy.h"

/* For normalization specification */
typedef enum normal_t {NORMAL_NONE=0x0, NORMAL_ALL=0x1, NORMAL_SECTION=0x2, 
		       NORMAL_LAYER=0x4} normal_t;

/***************/
/* Prototyping */
/***************/
void uniform_layer_normalize (CaloLayer*, const Energy);

/******************/
/* Implementation */
/******************/

/* Just divides all energy values on the pointer to the uniform RoI by the
   energy value contained in nfactor. OBS: the uniform RoI is changed during
   this call. */
void uniform_roi_normalize (uniform_roi_t* rp, const short flags)
{
  int i; /* iterator */
  Energy etot;

  /* Now, if necessary, I normalize */
  if ( (flags & NORMAL_ALL) != 0 ) {
    etot = uniform_roi_energy(rp);
    for (i=0; i< rp->nlayer; ++i) 
      uniform_layer_normalize(&rp->layer[i], etot);
  }

  return;
}

/* Do the same as uniform_roi_normalize(), but on CaloLayers */
void uniform_layer_normalize (CaloLayer* lp, const Energy nfactor)
{
  int eta,phi;
  int cell_index;

  for(phi=0; phi < lp->PhiGran; ++phi)
    for(eta=0; eta < lp->EtaGran; ++eta) {
      cell_index = eta + phi* lp->EtaGran;
      lp->cell[cell_index].energy /= nfactor;
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

  if (*from & NORMAL_ALL) ascat(&retval,"Total RoI Energy");
  if (*from & NORMAL_SECTION) ascat(&retval,"Section Energy (EM/HAD)");
  if (*from & NORMAL_LAYER) ascat(&retval,"Layer Energy");

  strncpy(to,retval,59);
  free(retval);
  return to;
}
