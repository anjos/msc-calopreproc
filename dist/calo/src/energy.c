/* Hello emacs, this is -*- c -*- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "energy.h"
#include "data.h"
#include "uniform.h"
#include "common.h"

/* $Id: energy.c,v 1.11 2000/12/20 08:17:00 andre Exp $ */

/* Some prototypes used here on */
Energy* energy_from_all_digis(const ROI*, Energy*);
Energy* uniform_layer_energy (const CaloLayer*, Energy*);
void uniform_roi_classics(const ROI*, const uniform_roi_t*, char**);

/* Functions for classical feature extraction */
void build_etem (const uniform_roi_t*, const int*, double*);
void build_ethad(const uniform_roi_t*, const int*, double*);
void build_rshape (const CaloLayer*, const int*, double*);
void build_rstrip (const CaloLayer*, const int*, double*);

/* These are the shorts that define what we'll dump. Each of those is explained
   above. */
typedef enum edump_flag_t {EDUMP_DB_ET=0x1, EDUMP_DB_ETHAD=0x2,
			   EDUMP_DB_T1ET=0x4, EDUMP_ROI_ET=0x8,
			   EDUMP_ROI_ETEM=0x10, EDUMP_ROI_ETHAD=0x20, 
			   EDUMP_ROI_DIGIS=0x40, EDUMP_ALL=0xFF,
			   EDUMP_NONE=0x0, EDUMP_CLASSICS=0x80} edump_flag_t;

/* 
   This simple procedure just catches a string given by the user and transforms
   it into a edump_flag_t (actually a short). This is to make it easy the life
   of your main.c.
   Pay attention: "none" has precedence over "all". 
*/
unsigned short* string2edump(unsigned short* to, const char* from)
{
  char* token;
  const char delimiters [] = " ,";

  /* copies the initial string, not to alter it with a strtok() call */
  char* temp2 = strdup (from);
  char* temp = temp2;

  /* Well, if you had something coded on to, say goodbye */
  (*to) = 0;

  if ( temp == NULL ) {
    fprintf(stderr, "(uniform)ERROR: Can't copy string on string2layer()\n");
    exit(EXIT_FAILURE);
  }

  /* Now I can use temp normally */
  while( (token = strtok(temp,delimiters)) != NULL ) {
    temp = NULL; /* next calls will continue to process temp */

    if ( strcasecmp(token,"db_et") == 0 ) (*to) |= EDUMP_DB_ET;
    else if ( strcasecmp(token,"db_ethad") == 0 ) (*to) |= EDUMP_DB_ETHAD;
    else if ( strcasecmp(token,"db_t1et") == 0 ) (*to) |= EDUMP_DB_T1ET;
    else if ( strcasecmp(token,"roi_et") == 0 ) (*to) |= EDUMP_ROI_ET;
    else if ( strcasecmp(token,"roi_etem") == 0 ) (*to) |= EDUMP_ROI_ETEM;
    else if ( strcasecmp(token,"roi_ethad") == 0 ) (*to) |= EDUMP_ROI_ETHAD;
    else if ( strcasecmp(token,"roi_digis") == 0 ) (*to) |= EDUMP_ROI_DIGIS;
    else if ( strcasecmp(token,"classics") == 0 ) (*to) |= (EDUMP_CLASSICS);
    else if ( strcasecmp(token,"all") == 0 ) {
      (*to) = EDUMP_ALL;
      break;
    }
    else if ( strcasecmp(token,"none") == 0 ) (*to) = EDUMP_NONE;
    else fprintf(stderr, "(energy)WARN: valid token? -> %s\n", token);
  }
  
  /* Can't forget to free the space I've allocated... */
  free(temp2);

  return (to);
}

/* Now I can prepare AND return a string that is going to be allocated here and
   will contain all the information one required. This function and the
   string2dump should be the only interface between us the the final user. You
   should not be concerned with the implementation from here down if you only
   want to use this code. 
   The order of printing is:
   1) DB_ET - The transverse energy forseen to be found by L2
   2) DB_ETHAD - The Et to be found on the HAD section by L2
   3) DB_T1ET - The L1 Threshold
   4) ROI_ET - The Et found by uniform.[ch] processing, with layer elimination
   5) ROI_ETEM - The EM part of ROI_ET
   6) ROI_ETHAD. - The HAD part of ROI_ET
   7) ROI_DIGIS - The total amount of energy summing all digis with no
                  preprocessing 
   8) CLASSICS - Will dump the classical features for e/jet discrimination, as
                 defined by the ATLAS Trigger TPR. Those are basically cell
		 sums in different configurations. There 4 features on the
		 grand total.
*/ 
char* get_energy(const ROI* r, const uniform_roi_t* ur,
		 const unsigned short* flags, const char* initstring)
{
  char* retval = NULL;
  char* tempval = NULL;
  char nl[] = "\n";
  Energy temp; /* a temporary space for holding some values */

  /* First of all see if we have to print something */
  if ((*flags) == EDUMP_NONE) return 0;

  /* In such case I have to allocate the space for the initstring */
  ascat(&retval, initstring);

  if ((*flags) & EDUMP_DB_ET) {
    temp = r->l2CalEm.Et;
    ascat_double(&retval, &temp);
  }

  if ((*flags) & EDUMP_DB_ETHAD) { 
    temp = r->l2CalEm.EtHad;
    ascat_double(&retval, &temp);
  }

  if ((*flags) & EDUMP_DB_T1ET) 
    ascat_int(&retval, &r->header.t1Et);

  if ((*flags) & EDUMP_ROI_ET) {
    uniform_roi_energy(ur,&temp);
    ascat_double(&retval, &temp);
  }

  if ((*flags) & EDUMP_ROI_ETEM) {
    uniform_roi_EM_energy(ur,&temp);
    ascat_double(&retval,&temp);
  }

  if ((*flags) & EDUMP_ROI_ETHAD) {
    uniform_roi_HAD_energy(ur,&temp);
    ascat_double(&retval, &temp);
  }

  if ((*flags) & EDUMP_ROI_DIGIS) {
    energy_from_all_digis(r,&temp);
    ascat_double(&retval, &temp);
  }

  /* This will print on tempval, the classical features. The RoI contains, at
     least, the layers necessary for performing this feature extraction. */
  if ((*flags) & EDUMP_CLASSICS) {
    uniform_roi_classics(r, ur, &tempval);
    ascat(&retval, tempval);
  }

  /* Add a new-line to the end of each row */
  ascat(&retval, nl);

  /* Now we can return the final stuff */
  return retval;
}

/* Given a flag description of energy configuration, this function can return a
   string containing the energy parameter descriptions. The string must have
   been allocated previously (with at least 60 bytes). */
char* edump2string (const unsigned short* from, char* to)
{
  char* retval; /* the place where we're going to put the output description */
  retval = NULL;

  /* Check if I have to return nothing */
  if (*from == EDUMP_NONE) {
    strcpy(to, "none");
    return to;
  }

  /* In such case I have to allocate the space for the initstring */
  ascat(&retval, "(");

  if (*from & EDUMP_DB_ET) ascat(&retval,"DB_ET");
  if (*from & EDUMP_DB_ETHAD) ascat(&retval,"DB_ETHAD");
  if (*from & EDUMP_DB_T1ET) ascat(&retval,"DB_T1ET");
  if (*from & EDUMP_ROI_ET) ascat(&retval,"ROI_ET");
  if (*from & EDUMP_ROI_ETEM) ascat(&retval,"ROI_ETEM");
  if (*from & EDUMP_ROI_ETHAD) ascat(&retval,"ROI_ETHAD");
  if (*from & EDUMP_ROI_DIGIS) ascat(&retval,"ROI_DIGIS");
  if (*from & EDUMP_CLASSICS) ascat(&retval,"CLAS_ET37 CLAS_ETHAD CLAS_RCORE CLAS_RSTRIP");

  /* final delimiter */
  ascat(&retval,")");

  strncpy(to,retval,99);
  free(retval);
  return to;
}

/* Validates energy selection based on layer selection criteria. For instance,
   there's no sense to print HAD energy IF no hadronic layer has been selected
   for dumping. If this function finds anything unusual, it should warn the
   used, correct energy parameters and return TRUE. Only stuppid cases are
   rejected with FALSE. */
bool_t validate_energy_selection(const unsigned short* layer_flags, 
				 unsigned short* energy_flags)
{
  unsigned short result;

  if (*energy_flags & EDUMP_ROI_ETEM) {
    result = (*layer_flags) & (FLAG_PS | FLAG_EM1 | FLAG_EM2 | FLAG_EM3);
    if (! flag_contains_nlayers ( &result ) ) {
      fprintf(stderr, "(energy) You can't demand ET energy if you did not");
      fprintf(stderr, " selected it\n");
      *energy_flags &= ~(EDUMP_ROI_ETEM);
      fprintf(stderr, "(energy) I won't print EM URoI energies\n");
    }
  }

  if (*energy_flags & EDUMP_ROI_ETHAD) {
    result = (*layer_flags) & (FLAG_HAD1 | FLAG_HAD2 | FLAG_HAD3);
    if (! flag_contains_nlayers ( &result ) ) {
      fprintf(stderr, "(energy) You can't demand HAD energy if you did not");
      fprintf(stderr, " selected it\n");
      *energy_flags &= ~(EDUMP_ROI_ETHAD);
      fprintf(stderr, "(energy) I won't print HAD URoI energies\n");
    }
  }

  /* This is quite difficult to happen, but let's try it */
  if (*energy_flags & EDUMP_ROI_ET)
    if (! flag_contains_nlayers (layer_flags)) {
      fprintf(stderr, "(energy) You can't demand Et energy if you did not");
      fprintf(stderr, " selected anything\n");
      *energy_flags = EDUMP_NONE;
      fprintf(stderr, "(energy) I won't print URoI energies\n");
    }

  if (*energy_flags & EDUMP_CLASSICS)
    if ( ~(*layer_flags) & FLAG_ALL) {
      fprintf(stderr, "(energy) You can't demand classical features ");
      fprintf(stderr, "extraction if you did not selected PS, EM1-3, ");
      fprintf(stderr, "HAD1-3\n");
      *energy_flags &= ~EDUMP_CLASSICS;
      fprintf(stderr, "(energy) I won't print CLASSICAL features\n");
    }

  /* always return TRUE here, no harm in doing that */
  return TRUE;

}

/* This function will sum the energy values of all digis found on an ROI and
   return a pointer to the place where it stored the sum of all those values
   found on digis. This is different from uniform_roi_energy() since it does
   not preprocess the digis or eliminate not used layers from the RoI. The
   energy memory space should be pre-allocated and should contain room for at
   least 1 Energy (double nowadays). I suggest using static allocation for that
   and passing a reference, but that is up to you. */
Energy* energy_from_all_digis(const ROI* r, Energy* ep)
{
  int i; /* iterator */

  /* start over */
  *ep = 0;

  for(i=0; i<r->calDigi.nEmDigi; ++i)
    *ep += r->calDigi.emDigi[i].Et;

  for(i=0; i<r->calDigi.nhadDigi; ++i)
    *ep += r->calDigi.hadDigi[i].Et;

  return ep;
}


/* This function shall add all energies on a uniform RoI. It returns the result
   of that summation on a pointer. This pointer has the same address of the
   second argument, which should be passed pre-allocated with the space needed
   by 1 (one) Energy variable. */
Energy* uniform_roi_energy (const uniform_roi_t* rp, Energy* counter)
{
  int i;
  Energy temp;
  
  (*counter) = 0.;

  for (i=0; i< rp->nlayer; ++i) {
      uniform_layer_energy(&rp->layer[i], &temp);
      (*counter) += temp;
    }
    
  return counter;
}

/* This function shall add all energies on a uniform RoI:EM section. It returns
   the result of that summation. The space required to store the Energy has to
   be pre-allocated by the caller */
Energy* uniform_roi_EM_energy (const uniform_roi_t* rp, Energy* counter)
{
  int i;
  Energy temp;
  
  (*counter) = 0.;

  for (i=0; i< rp->nlayer; ++i)
    if (rp->layer[i].calo == EM || rp->layer[i].calo == PS) {
      uniform_layer_energy(&rp->layer[i], &temp);
      (*counter) += temp;
    }
    
  return counter;
}

/* This function shall add all energies on a uniform RoI:HAD section. It
   returns the result of that summation on the space pointed by the second
   argument and to the caller (pointer). The space required to store the Energy
   has to be pre-allocated by the caller */
Energy* uniform_roi_HAD_energy (const uniform_roi_t* rp, Energy* counter)
{
  int i;
  Energy temp;
  
  (*counter) = 0.;

  for (i=0; i< rp->nlayer; ++i)
    if (rp->layer[i].calo == HAD) {
      uniform_layer_energy(&rp->layer[i], &temp);
      (*counter) += temp;
    }

  return counter;
}

/* This function shall add all energies over a layer. It returns the result of
   that summation on the value pointed by the second argument and to the caller
   (pointer). The space required to store the Energy has to be pre-allocated by
   the caller */
Energy* uniform_layer_energy (const CaloLayer* lp, Energy* counter)
{
  int eta,phi;
  int cell_index;

  /* If you had anything there, say goodbye:) */
  (*counter) = 0.;

  for(phi=0; phi < lp->PhiGran; ++phi)
    for(eta=0; eta < lp->EtaGran; ++eta) {
      cell_index = eta + phi* lp->EtaGran;
      (*counter) += lp->cell[cell_index].energy;
  }

  return counter;
}

/* This function is actually, just a front end to the functions that will
   perform classical feature extraction. The first parameter is the ROI, as
   seen by level 2. This routine will use only the first argument to calculate
   the quantities. Following works may compare this results with the ones
   extracted from the 'uniform_roi_t'. Anyway, results are print on (*retval),
   the last argument. .  */
void uniform_roi_classics(const ROI* r, const uniform_roi_t* ur, char** retval)
{
  Energy temp;
  int max=0;
  double eta;
  double phi;
  int i; /* iterator */

  /* I */
  /* Find the peak of energy on EM2 and put it into retval */
  for (i=0; i<ur->nlayer; ++i) {
    if (ur->layer[i].calo == EM && ur->layer[i].level == 2) 
      peak_find(&ur->layer[i], &max, &eta, &phi);
  }

  /* II */
  /* This energy is assumed to be PS+EM cells into a 3x7 (etaxphi) region
     around the peak of energy. The values should be corrected according to a
     formula shown on TPR, but since for discriminating purposes this is *not*
     needed (correction is linear) we won't apply it.
  */
  build_etem(ur, &max, &temp);
  ascat_double(retval, &temp);

  /* III */
  /* This is actually the same as above, but taken for the Hadronic
     Calorimeter. The quantity is used with predefined cuts, that are used
     according to the object Et (calculated on the first step). For Et till
     25.5GeV, the cut is 2.2, till 60GeV, 4.0 and abover 120Gev the cut used is
     1000 (no cut). */
  build_ethad(ur, &max, &temp);
  ascat_double(retval, &temp);

  /* IV */
  /* This measures the lateral shape of the current object. This quantity is
     not influenced directly by the objects' Et, but there should be caution
     using it with different objects. */
  for (i=0; i<ur->nlayer; ++i) {
    if (ur->layer[i].calo == EM && ur->layer[i].level == 2) 
      build_rshape(&ur->layer[i], &max, &temp);
  }
  ascat_double(retval, &temp);

  /* V */
  /* The last quantity measures the lateral shape in the first sampling. This
     cut is applied by last and therefore is more "fine" than the others. You
     have to tune it correctly in order to get the most out of it. */
  for (i=0; i<ur->nlayer; ++i) {
    if (ur->layer[i].calo == EM && ur->layer[i].level == 1 ) 
      build_rstrip(&ur->layer[i], &max, &temp);
  }
  ascat_double(retval, &temp);

  return;
}

/* This function will build the EtEM quantity on a 3x7 region around the peak
   of energy (found on EM layer 2). The 3x7 regions correspond to the cell
   size of EM layer 2. For the other EM layers and the PreSampler, a comparison
   for cell position inside the 3x7 (0.075x0.175, eta x phi) is done. If the
   cell center falls inside this region, its added, otherwise not.
*/
void build_etem (const uniform_roi_t* ur, const int* max, double* val)
{
  int i; /* iterator */
  int j; /* iterator */
  double eta_max;
  double phi_max;
  double eta;
  double phi;

  const int stdgran_eta = 16;
  const int stdgran_phi = 16;

  const double cluster_eta_range = 3 * (0.4/stdgran_eta);
  const double cluster_phi_range = 7 * (0.4/stdgran_phi);

  /* Initializes (*val) */
  (*val) = 0;

  /* Get coordinates of the peak */
  vector2point(&stdgran_eta, &stdgran_phi, max, &eta_max, &phi_max);

  /* Sums all cells from within the prescribed range */
  for (i=0; i<ur->nlayer; ++i)
    if (ur->layer[i].calo == PS || ur->layer[i].calo == EM) {
      for (j=0; j<ur->layer[i].NoOfCells; ++j) {
	vector2point(&ur->layer[i].EtaGran, &ur->layer[i].PhiGran,
		     &j, &eta, &phi);
	if (fabs(eta-eta_max) <= (0.5*cluster_eta_range) &&
	    fabs(phi-phi_max) <= (0.5*cluster_phi_range) ) {
	  (*val) += ur->layer[i].cell[j].energy;
	}
      }
    }

  return;
}

/* This function will calculate the Hadronic energy on a 0.2 by 0.2 window
   around the peak of energy. It basically does the same as the previous
   function (build_etem()), but for the hadronic section. The first argument
   points to the RoI, already uniformized, the second to the maximum point
   found on EM layer 2 and the third is there for returning the result only. */
void build_ethad(const uniform_roi_t* ur, const int* max, double* val)
{
  int i; /* iterator */
  int j; /* iterator */
  double eta_max;
  double phi_max;
  double eta;
  double phi;

  const int stdgran_on_em2_eta = 16;
  const int stdgran_on_em2_phi = 16;

  const double cluster_eta_range = 0.2;
  const double cluster_phi_range = 0.2;

  /* Initializes (*val) */
  (*val) = 0;

  /* Get coordinates of the peak */
  vector2point(&stdgran_on_em2_eta, &stdgran_on_em2_phi, max, 
	       &eta_max, &phi_max);

  /* Sums all cells from within the prescribed range */
  for (i=0; i<ur->nlayer; ++i)
    if (ur->layer[i].calo == HAD) {
      for (j=0; j<ur->layer[i].NoOfCells; ++j) {
	vector2point(&ur->layer[i].EtaGran, &ur->layer[i].PhiGran,
		     &j, &eta, &phi);
	if (fabs(eta-eta_max) <= 0.5*cluster_eta_range &&
	    fabs(phi-phi_max) <= 0.5*cluster_phi_range) {
	  (*val) += ur->layer[i].cell[j].energy;
	}
      }
    }

  return;
}

/* This function will build the quantity known as Rshape. It is the division of
   the 3x7 energy found on EM layer 2 by the 7x7 energy on the same layer. Both
   ranges are considered for the peak. Here I won't have to convert the cell
   indexes into real numbers since will operate only on layer 2. The input
   argument #1 will be expected to be _that_ layer. Output, as always, will be
   on the second argument. Note that this quantity may be greater than one
   since the pedestal of events may be removed. */  
void build_rshape (const CaloLayer* layer, const int* peak, double* val)
{
  int j; /* iterator */
  int eta_max;
  int phi_max;
  int eta;
  int phi;
  div_t R; /* just to get eta's and phi's */

  double e37 = 0;
  double e77 = 0;

  const int cluster_eta_range_numerator = 3;
  const int cluster_eta_range_denominator = 7;
  const int cluster_phi_range = 7;

  /* Get coordinates of the peak */
  R = div((*peak),layer->EtaGran);
  eta_max = R.rem;
  phi_max = R.quot;

  /* Sums all cells from within the prescribed range */
  for (j=0; j < layer->NoOfCells; ++j) {
    R = div(j,layer->EtaGran);
    eta = R.rem;
    phi = R.quot;
    /* First constraint is that it has to be inside the phi range */
    if (abs(phi-phi_max) <= 0.5*cluster_phi_range) {
      /* Then, if it is inside the numerator range I add to both 
	 subquantities*/
      if (abs(eta-eta_max) <= 0.5*cluster_eta_range_numerator) {
	e37 += layer->cell[j].energy;
	e77 += layer->cell[j].energy;
      }
      /* Else, if it is still under the denominator range, I add only to the
	 e77 quantity */
      else if (abs(eta-eta_max) <= 0.5*cluster_eta_range_denominator) {
	e77 += layer->cell[j].energy;
      }
    }
  }

  if (e77 > 0) (*val) = e37/e77;
  else (*val) = 99; /* Don't know why this, I'm following RefSoft
		       implementation */ 

  return;
}

/* This function will build the quantity known as Rstrip. This quantity is used
   for fine jet separation (as I understood from Saul...). In order to find
   this quantity, one searchs (in a window 0.2x0.2 around the peak of energy at
   EM layer 2) on EM layer 1 for two strips with the maximum energy. That's an
   easy job! */
void build_rstrip (const CaloLayer* layer, const int* max, double* val)
{
  double e1 = 0;
  double e2 = 0;

  double eta_max;
  double phi_max;

  double eta;
  double phi;

  const int stdgran_on_em2_eta = 16;
  const int stdgran_on_em2_phi = 16;

  const double cluster_eta_range = 0.2;
  const double cluster_phi_range = 0.2;

  /* This is a dummy algorithm */
  int i; /* iterator */

  /* Find the peak maximum point */
  vector2point(&stdgran_on_em2_eta, &stdgran_on_em2_phi, max,
	       &eta_max, &phi_max);

  /* Search in a reduced region is implemented here. Search will happen on a
     0.2x0.2 region. This algorithm is _not_ exaustive!!!! */

  /* for all the cells on this layer */
  for (i=0; i<layer->NoOfCells; ++i) { 
    vector2point(&layer->EtaGran, &layer->PhiGran, &i, &eta, &phi);

    /* if they are on a prescribed interval */
    if (fabs(eta-eta_max) <= 0.5*cluster_eta_range &&
	fabs(phi-phi_max) <= 0.5*cluster_phi_range) {

      /* Test if this is a peak */
      if (layer->cell[i].energy > layer->cell[i+1].energy &&
	  layer->cell[i].energy > layer->cell[i-1].energy)

	/* if the energy is greater than the previous peak */
	if (layer->cell[i].energy > e1) {
	  /* Swap values */
	  e2 = e1;
	  e1 = layer->cell[i].energy;

	}

    }

  }

  /* Finally, we get the quantity */
  if (e1+e2 > 0) (*val) = (e1-e2)/(e1+e2);
  else (*val) = 99; /* ?? */

  return;
}

