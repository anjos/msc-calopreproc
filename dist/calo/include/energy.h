/* Hello emacs, this is -*- c -*- */
/* André Rabello dos Anjos <Andre.dos.Anjos@cern.ch> */

/* $Id: energy.h,v 1.6 2001/01/30 16:34:34 andre Exp $ */

#ifndef _ENERGY_H
#define _ENERGY_H

#include "data.h"
#include "common.h"
#include "uniform.h"
#include "energy.h"

/* 
   This component includes functions for processing event energy. It introduces
   facilities for dumping:
   ASCII DATABASE L2CALEM parameters such: 
   - Et (the transverse energy that L2 should get, 
   - EtHad (the transverse energy of the hadronic part of the event and from 
   ROIHEAD:
   - t1Et (the energy threshold of Level 1)
   
   It can also calculate the energies contained on the current RoI by summing
   the digi energies of those RoIs. These options are only valid for situations
   where one wants to dump rings or uniformized RoIs. The quantities that can
   be returned are:
   - Energy of the whole RoI;
   - Energy of the EM section;
   - Energy of the Hadronic section;
*/
  
/* 
   This simple procedure just catches a string given by the user and transforms
   it into a edump_flag_t (actually a short). This is to make it easy the life
   of your main.c.
   Pay attention: "none" has precedence over "all". 
*/
unsigned short* string2edump(unsigned short*, const char*);

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
   6) ROI_ETHAD.

   The input parameters are the ROI as defined by the 'data' module (spec
   library), the uniformized RoI, as defined by 'uniform'. A short containing
   the 'energy flags' and finally a pointer to a string  containing the comment
   characters to be inserted on the beginning of the string. The string
   returned is allocated with malloc() and should be freed after usage. */
char* get_energy(const ROI*, const uniform_roi_t*, 
		 const unsigned short*, const char*);

/* Validates energy selection based on layer selection criteria. For instance,
   there's no sense to print HAD energy IF no hadronic layer has been selected
   for dumping. If this function finds anything unusual, it should warn the
   used, correct energy parameters and return TRUE. Only stuppid cases are
   rejected with FALSE. The first argument is a pointer to the layer_flags and
   the second to the energy_flags as forseen on parameter_t@main.c */
bool_t validate_energy_selection(const unsigned short*, unsigned short*);

/* Given a flag description of energy configuration, this function can return a
   string containing the energy parameter descriptions. The string must have
   been allocated previously (with at least 60 bytes). */
char* edump2string (const unsigned short*, char*);

/* The next functions extract the energy sum of all cells of the collected
   RoI. The RoI cells available are selected at run time by the appropriate
   layer_flags on main.c. The EM and HAD versions of it return only the EM and
   hadronic sections energies. Note: The first function (uniform_roi_energy())
   is *NOT* implemented using the following EM/HAD versions, it just sums over
   all cells on the RoI with no EM/HAD distinction. Hipothetically, the sum of
   the last two, should be the return value of the first, but you can think the
   implementation of uniform_roi_energy() as a debugging parameter for the sum
   of the other two. The last function will evaluate only the layer
   energy. This one is actually used by uniform_roi_energy() to compute the
   total uniform_roi_t energy. */
Energy* uniform_roi_energy (const uniform_roi_t*, Energy*);
Energy* uniform_roi_EM_energy (const uniform_roi_t*, Energy*);
Energy* uniform_roi_HAD_energy (const uniform_roi_t*, Energy*);
Energy* uniform_layer_energy (const CaloLayer*, Energy*);

#endif /* _ENERGY_H */





