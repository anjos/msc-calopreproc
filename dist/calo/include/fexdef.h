#ifndef FEXDEF_H
#define FEXDEF_H

#include "error.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct CaloFeatures {
  int NoOfFeatures;
  double* feature;
}CaloFeatures;

extern ErrorCode FreeCaloFeatures(CaloFeatures* feat);
extern void PrintCaloFeatures(FILE*, const CaloFeatures*);

#endif
