/* Hello emacs, this is -*- c -*- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "energy.h"
#include "data.h"
#include "uniform.h"
#include "common.h"

/* $Id: energy.c,v 1.6 2000/11/25 19:38:18 andre Exp $ */

/* Some prototypes used here on */
Energy* energy_from_all_digis(const ROI*, Energy*);
Energy* uniform_layer_energy (const CaloLayer*, Energy*);
void uniform_roi_classics(const ROI*, const uniform_roi_t*, char**);

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
    else if ( strcasecmp(token,"classics") == 0 ) 
      (*to) |= (EDUMP_CLASSICS | EDUMP_DB_ETHAD);
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
		 grand total. The first 2 are 1) and 2) from this listing the
		 others are described on the Trigger Performance TPR.
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
  if (*from & EDUMP_CLASSICS) ascat(&retval,"CLAS_ET37 CLAS_RCORE CLAS_RSTRIP");

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
   seen by level 2, depending on implementationg, this function may take the
   values from the RoI header directly, process the 'raw' RoI to get them or
   process the uniformized RoI. Anyway, results are print on (*retval), the
   last argument. The user should not note differences between direct RoI
   header access or calculations, with matter to the interface usage.
*/
void uniform_roi_classics(const ROI* r, const uniform_roi_t* ur, char** retval)
{
  Energy temp;

  /* I */
  /* This energy is assumed to be PS+EM cells into a 3x7 (etaxphi) region
     around the peak of energy. The values are corrected according to a formula
     shown on TPR. Now, I'll take for granted that this is the same as the
     quantity Et_EM, since I can't see why to have other value for the same
     stuff. */
  temp = r->l2CalEm.Et37;
  ascat_double(retval, &temp);

  /* II */
  /* ------ ETHAD IS DUMPED BY THE DEFAULT PROCEDURE UPSTAIRS ------ */
  /* This is actually the same as above, but taken for the Hadronic
     Calorimeter. The quantity is used with predefined cuts, that are used
     according to the object Et (calculated on the first step). For Et till
     25.5GeV, the cut is 2.2, till 60GeV, 4.0 and abover 120Gev the cut used is
     1000 (no cut). */

  /* III */
  /* This measures the lateral shape of the current object. This quantity is
     not influenced directly by the objects' Et, but there should be caution
     using it with different objects. */
  temp = r->l2CalEm.Etlay[2] / r->l2CalEm.Et77lay[2];
  ascat_double(retval, &temp);

  /* IV */
  /* The last quantity measures the lateral shape in the first sampling. This
     cut is applied by last and therefore is more "fine" than the others. You
     have to tune it correctly in order to get the most out of it. */
  temp = (r->l2CalEm.E1st - r->l2CalEm.E2nd) /
    (r->l2CalEm.E1st + r->l2CalEm.E2nd);
  ascat_double(retval, &temp);

  return;
}


