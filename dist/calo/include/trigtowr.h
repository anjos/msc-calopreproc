/* Hello emacs, this is -*- c -*- */

/* $Id: trigtowr.h,v 1.3 2000/06/28 16:00:00 rabello Exp $ */

#ifndef _TRIGTOWR_H
#define _TRIGTOWR_H

#include "data.h"
#include "error.h"
#include "common.h"
#include "ttdef.h"

const static double EMRoIGran = EMROIGRAN;
const static int MaxNumberOfLayers = 8;
const static double EtaTTSize = 0.1;
const static double PhiTTSize = 0.1;

/* Using the ROI, define by CERN specification files, build, optinally fixing
   the window size, a CaloTTEMRoI */
ErrorCode build_roi(const ROI*, const bool_t, CaloTTEMRoI*);

/* Free an *used* CaloTTEMRoI */
void free_roi(CaloTTEMRoI*);

/* Free an *used* CaloLayer */
void free_layer(CaloLayer*);

/* Dumps layer(s) from a CaloTTEMRoI. For the time being, it can only dump the
   second EM layer, from the endcap or barrel. */
bool_t print_roi(FILE*, const CaloTTEMRoI*);

/* This function just checks the second EM layer on a particular trigger tower
   and return TRUE if it finds one and FALSE otherwise. */
bool_t verify_tt(const CaloTriggerTower*);

/* This function only verifies if this is an EM middle layer */
bool_t is_middle(const Calorimeter, const int);

#endif /* _TRIGTOWR_H */





