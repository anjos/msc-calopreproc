/* Hello emacs, this is -*- c -*- */

/* $Id: ttdef.h,v 1.3 2000/07/07 18:27:40 rabello Exp $ */

#ifndef _TTDEF_H
#define _TTDEF_H

#include "common.h"

#define EMROIGRAN 4 /* The granularity of trigger towers in EM */
#define HADROIGRAN 2 /* The granularity of trigger towers in HAD */

typedef struct CaloCell {
  Energy energy;
  index_t index;
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

typedef struct tt_roi_t {
  CaloTriggerTower em_tt[EMROIGRAN][EMROIGRAN];
  CaloTriggerTower had_tt[HADROIGRAN][HADROIGRAN];
  Area Region;
  bool_t PhiWrap;
  bool_t fixed;
}tt_roi_t;

#endif /* _TTDEF_H */


