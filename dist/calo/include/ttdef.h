#ifndef TTDEF_H
#define TTDEF_H

#include "common.h"

#define EMROIGRAN 4

typedef struct CaloCell {
  Energy energy;
  Index index;
}CaloCell;

/* always a 0.1 x 0.1 TT */
typedef struct CaloLayer {
  int EtaGran;
  int PhiGran;
  int NoOfCells;
  CaloCell* cell;
  Calorimeter calo;
  LayerLevel level;
}CaloLayer;

typedef struct CaloTriggerTower {
  int NoOfLayers;
  CaloLayer* layer;
}CaloTriggerTower;

typedef struct CaloTTEMRoI {
  CaloTriggerTower tt[EMROIGRAN][EMROIGRAN];
  Area Region;
  Flag PhiWrap;
  Flag fixed;
}CaloTTEMRoI;

#endif


