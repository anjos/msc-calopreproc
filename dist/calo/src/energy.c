/* Hello emacs, this is -*- c -*- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "uniform.h"
#include "common.h"

/* $Id: energy.c,v 1.1 2000/08/11 20:29:07 rabello Exp $ */

/* These are the shorts that define what we'll dump. Each of those is explained
   above. */
typedef enum edump_flag_t {EDUMP_DB_ET=0x1, EDUMP_DB_ETHAD=0x2,
			   EDUMP_DB_T1ET=0x4, EDUMP_ROI_ET=0x8,
			   EDUMP_ROI_ETEM=0x10, EDUMP_ROI_ETHAD=0x20, 
                           EDUMP_ALL=0x3F, EDUMP_NONE=0x0} edump_flag_t;

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
   1) DB_ET
   2) DB_ETHAD
   3) DB_T1ET
   4) ROI_ET
   5) ROI_ETEM
   6) ROI_ETHAD. */
char* get_energy(const ROI* r, const uniform_roi_t* ur, 
		 const unsigned short flags, const char* initstring) 
{
  char* retval;
  char* temp; /* temporary resource for holding strings */

  /* First of all see if we have to print something */
  if (flags == EDUMP_NONE) return 0;

  /* In such case I have to allocate the space for the initstring */
  asprintf(&temp, "%s", initstring);

  if (flags & EDUMP_DB_ET) /* have to print it */ {
    /* copy the preceeding string into a safe place and append data */
    asprintf(&retval, "%s %e", temp, r->l2CalEm.Et);
    /* free the old used string */
    free(temp);
    /* duplicate data pointer into the temporary variable */
    temp = retval;
  }

  if (flags & EDUMP_DB_ETHAD) /* have to print it */ {
    /* copy the preceeding string into a safe place and append data */
    asprintf(&retval, "%s %e", temp, r->l2CalEm.EtHad);
    /* free the old used string */
    free(temp);
    /* duplicate data pointer into the temporary variable */
    temp = retval;
  }

  if (flags & EDUMP_DB_T1ET) /* have to print it */ {
    /* copy the preceeding string into a safe place and append data */
    asprintf(&retval, "%s %d", temp, r->header.t1Et);
    /* free the old used string */
    free(temp);
    /* duplicate data pointer into the temporary variable */
    temp = retval;
  }

  if (flags & EDUMP_ROI_ET) /* have to print it */ {
    /* copy the preceeding string into a safe place and append data */
    asprintf(&retval, "%s %e", temp, uniform_roi_energy(ur));
    /* free the old used string */
    free(temp);
    /* duplicate data pointer into the temporary variable */
    temp = retval;
  }

  if (flags & EDUMP_ROI_ETEM) /* have to print it */ {
    /* copy the preceeding string into a safe place and append data */
    asprintf(&retval, "%s %e", temp, uniform_roi_EM_energy(ur));
    /* free the old used string */
    free(temp);
    /* duplicate data pointer into the temporary variable */
    temp = retval;
  }

  if (flags & EDUMP_ROI_ETHAD) /* have to print it */ {
    /* copy the preceeding string into a safe place and append data */
    asprintf(&retval, "%s %e", temp, uniform_roi_HAD_energy(ur));
    /* free the old used string */
    free(temp);
    /* duplicate data pointer into the temporary variable */
    temp = retval;
  }

  /* Now we can return the final stuff */
  return retval;
}

/* Validates energy selection based on layer selection criteria. For instance,
   there's no sense to print HAD energy IF no hadronic layer has been selected
   for dumping. If this function finds anything unusual, it should warn the
   used, correct energy parameters and return TRUE. Only stuppid cases are
   rejected with FALSE. */
bool_t validate_energy_selection(const unsigned short* layer_flags, 
				 unsigned short* energy_flags)
{
  if (*energy_flags & EDUMP_ROI_ETEM)
    if (! flag_contains_nlayers (*layer_flags & 
				 (FLAG_PS | FLAG_EM1 | FLAG_EM2 | FLAG_EM3))) {
      fprintf(stderr, "(energy) You can't demand ET energy if you did not");
      fprintf(stderr, " selected it\n");
      *energy_flags &= ~(EDUMP_ROI_ETEM);
      fprintf(stderr, "(energy) I won't print EM URoI energies\n");
    }

  if (*energy_flags & EDUMP_ROI_ETHAD)
    if (! flag_contains_nlayers (*layer_flags & 
				 (FLAG_HAD1 | FLAG_HAD2 | FLAG_HAD3))) {
      fprintf(stderr, "(energy) You can't demand HAD energy if you did not");
      fprintf(stderr, " selected it\n");
      *energy_flags &= ~(EDUMP_ROI_ETHAD);
      fprintf(stderr, "(energy) I won't print HAD URoI energies\n");
    }

  /* This is quite difficult to happen, but let's try it */
  if (*energy_flags & EDUMP_ROI_ET)
    if (! flag_contains_nlayers (*layer_flags)) {
      fprintf(stderr, "(energy) You can't demand Et energy if you did not");
      fprintf(stderr, " selected anything\n");
      *energy_flags = EDUMP_NONE;
      fprintf(stderr, "(energy) I won't print URoI energies\n");
    }

  /* always return TRUE here, no harm in doing that */
  return TRUE;

}
	
