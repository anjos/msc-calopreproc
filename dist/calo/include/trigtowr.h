#ifndef TRIGTOWR_H
#define TRIGTOWR_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "data.h"
#include "error.h"
#include "common.h"
#include "portable.h"
#include "ttdef.h"
#include "zstt.h"

/* static constants */
const static double EMRoIGran = EMROIGRAN;
const static int MaxNumberOfLayers = 8;
const static double EtaTTSize = 0.1;
const static double PhiTTSize = 0.1;

/* const static double PhiTTSize = 2 * PI / 64; */

ErrorCode BuildCaloTTS(const ROI*, const Flag, CaloTTEMRoI*);
void FreeCaloEMRoI(CaloTTEMRoI*);

extern double rint(double x);

#endif
