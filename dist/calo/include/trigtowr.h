/* Hello emacs, this is -*- c -*- */

/* $Id: trigtowr.h,v 1.2 2000/05/31 13:57:12 rabello Exp $ */

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


ErrorCode BuildCaloTTS(const ROI*, const bool_t, CaloTTEMRoI*);
void FreeCaloEMRoI(CaloTTEMRoI*);

#endif /* _TRIGTOWR_H */
