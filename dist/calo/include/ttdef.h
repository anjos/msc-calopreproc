/* Hello emacs, this is -*- c -*- */

/* $Id: ttdef.h,v 1.2 2000/05/31 12:01:13 rabello Exp $ */

#ifndef _TTDEF_H
#define _TTDEF_H

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
  bool_t PhiWrap;
  bool_t fixed;
}CaloTTEMRoI;

#endif /* _TTDEF_H */


